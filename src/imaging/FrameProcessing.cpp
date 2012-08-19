//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: FrameProcessing.cpp
//
//  Implementation of Cartoonizer Algorithms
//
//--------------------------------------------------------------------------

#include "pch.h"
#include "Utils.h"
#include "FrameData.h"
#include "ppltasks.h"

using namespace concurrency;

const double Util::Wr = 0.299;
const double Util::Wb = 0.114;
const double Util::wg = 1 - Util::Wr - Util::Wb;

FrameProcessing::FrameProcessing()
{
    m_Size = 0;
    m_BPP = 0;
    m_Pitch = 0;
    m_NeighborWindow = 3;
    m_pCurrentImage = NULL;
    m_pBufferImage = NULL;
}


FrameProcessing::~FrameProcessing(void)
{
    if(NULL != m_pCurrentImage)
    {
        Concurrency::Free(m_pCurrentImage);
    }

    if(NULL != m_pBufferImage)
    {
        Concurrency::Free(m_pBufferImage);
    }
}

void FrameProcessing::ApplyFilters(int nPhases, Cartoonifier::ReportProgressCallback progressCallback)
{
    // ColorSimplifier work:
    m_totalCompletion = nPhases * (m_Height - m_NeighborWindow);

    // EdgeDetection work:
    m_totalCompletion += m_Height - 2;

    m_lastReportedCompletion = 0;
    m_workDone = 0;

    ApplyColorSimplifier(nPhases, progressCallback);
    ApplyEdgeDetection(progressCallback);
}

void FrameProcessing::SetCurrentFrame(BYTE* pFrame, int size, int width, int height, int pitch, int bpp, int clrPlanes)
{
    m_Width = width;
    m_Height = height;
    m_Pitch = pitch;
    m_BPP = bpp;
    m_ColorPlanes = clrPlanes;
    if(size != m_Size)
    {
        if(NULL != m_pCurrentImage)
        {
            Concurrency::Free(m_pCurrentImage);
            m_pCurrentImage = NULL;
        }

        if(NULL != m_pBufferImage)
        {
            Concurrency::Free(m_pBufferImage);
            m_pBufferImage = NULL;
        }
    }

    if(NULL == m_pCurrentImage)
    {
        m_pCurrentImage = (BYTE*)Concurrency::Alloc(size);
    }

    if(NULL == m_pBufferImage)
    {
        m_pBufferImage = (BYTE*)Concurrency::Alloc(size);
    }

    memcpy_s(m_pCurrentImage, size, pFrame, size);
    memcpy_s(m_pBufferImage, size, pFrame, size);
    m_Size = size;
}

void FrameProcessing::ApplyColorSimplifier(int nPhases, Cartoonifier::ReportProgressCallback progressCallback)
{
    for(int i = 0; i < nPhases; ++i)
    {
        ApplyColorSimplifier(progressCallback);
    }
}

void FrameProcessing::ApplyColorSimplifier(Cartoonifier::ReportProgressCallback progressCallback)
{
    unsigned int shift = m_NeighborWindow / 2;

    ApplyColorSimplifier(shift, (m_Height - shift), shift, ( m_Width - shift), progressCallback);
}

void FrameProcessing::ApplyColorSimplifier(unsigned int startHeight, unsigned int endHeight, unsigned int startWidth, unsigned int endWidth, Cartoonifier::ReportProgressCallback progressCallback)
{
    critical_section progressCritSec;

    if(NULL != m_pBufferImage)
    {
        parallel_for(startHeight, endHeight, [&](unsigned int j)
        {
            for(unsigned int i = startWidth; i < endWidth; ++i)
            {
                SimplifyIndexOptimized(m_pBufferImage, i, j);
            }

            {
                critical_section::scoped_lock lockHolder(progressCritSec);
                UpdateProgress(progressCallback);
            }
        });
    }
}

void FrameProcessing::SimplifyIndexOptimized(BYTE* pFrame, int x, int y)
{
    COLORREF orgClr =  GetPixel(pFrame, x, y, m_Pitch, m_BPP);

    int shift = m_NeighborWindow / 2;
    double sSum = 0;
    double partialSumR = 0, partialSumG = 0, partialSumB = 0;
    double standardDeviation = 0.025;

    for(int j = y - shift; j <= (y + shift); ++j)
    {
        for(int i = x - shift; i <= (x + shift); ++i)
        {
            if(i != x ||  j != y) // don't apply filter to the requested index, only to the neighbors
            {
                COLORREF clr = GetPixel(pFrame, i, j, m_Pitch, m_BPP);
                int index = (j - (y - shift)) * m_NeighborWindow + i - (x - shift);
                double distance = Util::GetDistance(orgClr, clr);
                double sValue = pow(M_E, -0.5 * pow(distance / standardDeviation, 2));
                sSum += sValue;
                partialSumR += GetRValue(clr) * sValue;
                partialSumG += GetGValue(clr) * sValue;
                partialSumB += GetBValue(clr) * sValue;
            }
        }
    }

    COLORREF simplifiedClr;
    int simpleRed, simpleGreen, simpleBlue;

    simpleRed     = (int)min(max(partialSumR / sSum, 0), 255);
    simpleGreen = (int)min(max(partialSumG / sSum, 0), 255);
    simpleBlue     = (int)min(max(partialSumB / sSum, 0), 255);
    simplifiedClr = RGB(simpleRed, simpleGreen, simpleBlue);
    SetPixel(m_pBufferImage, x, y, m_Pitch, m_BPP, simplifiedClr);
}

void FrameProcessing::ApplyEdgeDetection(Cartoonifier::ReportProgressCallback progressCallback)
{
    ApplyEdgeDetectionParallel(1, (m_Height - 1), 1, (m_Width - 1), progressCallback);
}

void FrameProcessing::ApplyEdgeDetectionParallel(unsigned int startHeight, unsigned int endHeight, unsigned int startWidth, unsigned int endWidth, Cartoonifier::ReportProgressCallback progressCallback)
{
    const float alpha = 0.3f;
    const float beta = 0.8f;
    const float s0 = 0.054f;
    const float s1 = 0.064f;
    const float a0 = 0.3f;
    const float a1 = 0.7f;

    BYTE* pFrame = new BYTE[m_Size];
    memcpy_s(pFrame, m_Size, m_pBufferImage, m_Size);

    critical_section progressCritSec;

    parallel_for(startHeight , endHeight,  [&](int y)
    {
        for(unsigned int x = startWidth; x < endWidth; ++x)
        {
            float Sy, Su, Sv;
            float Ay, Au, Av;

            CalculateSobel(m_pBufferImage, x, y, Sy, Su, Sv);
            CalculateSobel(m_pCurrentImage, x, y, Ay, Au, Av);

            float edgeS = (1 - alpha) * Sy +
                alpha * (Su + Sv) / 2;

            float edgeA = (1 - alpha) * Ay +
                alpha * (Au + Av) / 2;

            float i = (1 - beta) * Util::SmoothStep(s0, s1, edgeS)
                + beta * Util::SmoothStep(a0, a1, edgeA);

            float oneMinusi = 1 - i;
            COLORREF clr = this->GetPixel(m_pBufferImage, x, y, m_Pitch, m_BPP);
            COLORREF newClr = RGB(GetRValue(clr) * oneMinusi, GetGValue(clr) * oneMinusi, GetBValue(clr) * oneMinusi);
            this->SetPixel(pFrame, x, y, m_Pitch, m_BPP, newClr);
        }

        {
            critical_section::scoped_lock lockHolder(progressCritSec);
            UpdateProgress(progressCallback);
        }

    });
    memcpy_s(m_pBufferImage, m_Size, pFrame, m_Size);
    delete[] pFrame;
}

void FrameProcessing::CalculateSobel(BYTE* pSource, int row, int column, float& dy, float& du, float& dv)
{
    int gx[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };   //  The matrix Gx
    int gy[3][3] = { { 1, 2, 1 }, { 0, 0, 0 }, { -1, -2, -1 } };  //  The matrix Gy

    double new_yX = 0, new_yY = 0;
    double new_uX = 0, new_uY = 0;
    double new_vX = 0, new_vY = 0;
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            double y, u, v;
            Util::RGBToYUV(GetPixel(pSource, row + i, column + j, m_Pitch, m_BPP), y, u, v);

            new_yX += gx[i + 1][j + 1] * y;
            new_yY += gy[i + 1][j + 1] * y;

            new_uX += gx[i + 1][j + 1] * u;
            new_uY += gy[i + 1][j + 1] * u;

            new_vX += gx[i + 1][j + 1] * v;
            new_vY += gy[i + 1][j + 1] * v;
        }
    }

    dy = (float)sqrt(pow(new_yX, 2) + pow(new_yY, 2));
    du = (float)sqrt(pow(new_uX, 2) + pow(new_uY, 2));
    dv = (float)sqrt(pow(new_vX, 2) + pow(new_vY, 2));
}

void FrameProcessing::UpdateProgress(Cartoonifier::ReportProgressCallback progressCallback)
{
    //static const int delay = 0; // delay so that progress can be observed
    m_workDone++;
    if( m_workDone - m_lastReportedCompletion > m_totalCompletion/100 )
    {
        m_lastReportedCompletion = m_workDone;
        progressCallback(100*m_workDone/m_totalCompletion);
        //Sleep(delay * 1000 / 100);
    }
}