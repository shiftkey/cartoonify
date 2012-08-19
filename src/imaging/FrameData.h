//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: CartoonAgentBase.h
//
//  base classes for Cartoon Agents
//
//--------------------------------------------------------------------------

#pragma once

#include <functional>
#include "Cartoonifier.h"

class FrameProcessing;

struct FrameData
{
    int m_Size;
    unsigned int m_StartWidth;
    unsigned int m_StartHeight;
    unsigned int m_EndWidth;
    unsigned int m_EndHeight;
    int m_Pitch;
    int m_BBP;
    int m_ColorPlanes;
    BYTE* m_pFrame;
    int m_PhaseCount;
    int m_neighbourArea;
    FrameProcessing* m_pFrameProcesser;

    FrameData()
    {
        m_neighbourArea = 3;
        m_PhaseCount = 0;
        m_Size = 0;
        m_StartWidth = 0;
        m_EndWidth = 0;
        m_StartHeight = 0;
        m_EndHeight = 0;
        m_Pitch = 0;
        m_BBP = 0;
        m_pFrame = NULL;
        m_pFrameProcesser = NULL;
        m_ColorPlanes = 0;
    }
};

//typedef std::function<void(int)> ReportProgressCallback;

class FrameProcessing
{
public:
    FrameProcessing();
    ~FrameProcessing(void);

    void ApplyFilters(int nPhases,Cartoonifier::ReportProgressCallback progressCallback);
    void StopFilters();
    void SetCurrentFrame(BYTE* pFrame,int size, int width, int height, int pitch, int bpp, int clrPlanes);

    void SetNeighbourArea(unsigned int area)
    {
        m_NeighborWindow = area;
    }
    unsigned int GetNeighbourArea()
    {
        return m_NeighborWindow;
    }

    void FrameDone(BYTE* pTarget, size_t size)
    {
        memcpy_s(pTarget, min(size,m_Size), m_pBufferImage, min(size,m_Size));
    }

public: //methods
    void ApplyColorSimplifier(Cartoonifier::ReportProgressCallback progressCallback);
    void ApplyColorSimplifier(int nPhases,Cartoonifier::ReportProgressCallback progressCallback);
    void ApplyColorSimplifier(unsigned int startHeight, unsigned int endHeight, unsigned int startWidth, unsigned int endWidth, Cartoonifier::ReportProgressCallback progressCallback);
    void SimplifyIndexOptimized(BYTE* pFrame, int x, int y);
    void ApplyEdgeDetection(Cartoonifier::ReportProgressCallback progressCallback);
    void ApplyEdgeDetectionParallel(unsigned int startHeight, unsigned int endHeight, unsigned int startWidth, unsigned int endWidth,Cartoonifier::ReportProgressCallback progressCallback);
    void CalculateSobel(BYTE* pSource, int row, int column, float& dy, float& du, float& dv);

public:
    static inline COLORREF GetPixel(BYTE* pFrame, int x, int y, int pitch, int bpp)
    {
        int width = abs((int)pitch);
        int bytesPerPixel = bpp / 8;
        int byteIndex = y * width + x * bytesPerPixel;
        return RGB(pFrame[byteIndex + 2], 
            pFrame[byteIndex + 1],
            pFrame[byteIndex]);
    }

    static inline void SetPixel(BYTE* pFrame, int x, int y, int pitch, int bpp, COLORREF color)
    {
        int width = abs((int)pitch);
        int bytesPerPixel = bpp / 8;
        int byteIndex = y * width + x * bytesPerPixel;
        pFrame[byteIndex + 2] = GetRValue(color);
        pFrame[byteIndex + 1] = GetGValue(color);
        pFrame[byteIndex] = GetBValue(color);
    }

private: //members

    BYTE* m_pCurrentImage;  // src for the current frame
    BYTE* m_pBufferImage;  // current image being processed
    BYTE* m_pOutputImage;  // frame after Processing
    unsigned int m_NeighborWindow;

    unsigned int  m_Width;
    unsigned int  m_Height;
    unsigned int m_Size;
    int m_Pitch;
    unsigned int m_BPP;
    unsigned int m_ColorPlanes;

    // Progress tracking data:
    UINT m_totalCompletion;
    UINT m_lastReportedCompletion;
    UINT m_workDone;

    void UpdateProgress(Cartoonifier::ReportProgressCallback progressCallback);
};