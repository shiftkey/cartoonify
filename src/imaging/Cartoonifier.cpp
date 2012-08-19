// ImmersiveDll.cpp : Defines the exported functions for the DLL application.
//

#include "pch.h"
#include "Cartoonifier.h"
#include "FrameData.h"

using namespace Microsoft::WRL;

static unsigned int width, height;
static ComPtr<IWICBitmapSource> wicBitmap;
static ComPtr<IWICImagingFactory> wicFactory;
static ComPtr<IWICBitmapScaler> scaler;
static ComPtr<IWICBitmapFlipRotator> FlipRotator;

static HRESULT GetImageSize()
{
    HRESULT hr = wicBitmap->GetSize(&width, &height);
    return hr;
}

static HRESULT LoadFromImageFile(const wchar_t *wFileName)
{
    HRESULT hr = S_OK;

    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICBitmapFrameDecode> bitmapSource;

    hr = CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,&wicFactory);

    if (SUCCEEDED(hr))
        hr = wicFactory->CreateDecoderFromFilename(wFileName,nullptr,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&decoder);

    if (SUCCEEDED(hr))
        hr = decoder->GetFrame(0, &bitmapSource);

    if(SUCCEEDED(hr))
        hr = bitmapSource.As<IWICBitmapSource>(&wicBitmap);

    if(SUCCEEDED(hr))
        GetImageSize();

    return hr;
}

static HRESULT InternalTransformImage(Cartoonifier::ReportProgressCallback progressCallback)
{
    HRESULT hr = S_OK;

    ComPtr<IWICBitmap> bitmap = NULL;

    WICRect rcLock = { 0, 0, width, height };
    IWICBitmapLock *pLock = NULL;

    hr = wicFactory->CreateBitmapFromSource(wicBitmap.Get(), WICBitmapCacheOnDemand, &bitmap);
    if (SUCCEEDED(hr))
    {
        hr = bitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);

        if (SUCCEEDED(hr))
        {
            BYTE* pFrame = NULL;
            UINT cbBufferSize = 0;
            UINT cbStride = 0;

            hr = pLock->GetStride(&cbStride);

            if (SUCCEEDED(hr))
            {
                hr = pLock->GetDataPointer(&cbBufferSize, &pFrame);
            }

            WICPixelFormatGUID format;
            wicBitmap->GetPixelFormat(&format);

            int bpp = 8;
            if(format == GUID_WICPixelFormat24bppBGR || format == GUID_WICPixelFormat24bppBGR) 
            {
                bpp = 24;
            }
            int size = cbBufferSize;

            const int neighbourWindow = 4;
            const int phasesCount = 3;

            FrameData frameData;
            frameData.m_BBP = bpp;
            frameData.m_ColorPlanes = 1;
            frameData.m_EndHeight = height;
            frameData.m_EndWidth = width;
            frameData.m_neighbourArea = neighbourWindow;
            frameData.m_pFrame = pFrame;
            frameData.m_pFrameProcesser = NULL;
            frameData.m_PhaseCount = phasesCount;
            frameData.m_Pitch = cbStride;
            frameData.m_Size = size;
            frameData.m_StartHeight = 0;
            frameData.m_StartWidth = 0;

            FrameProcessing frameProcessing;
            frameData.m_pFrameProcesser = &frameProcessing;

            frameData.m_pFrameProcesser->SetNeighbourArea(frameData.m_neighbourArea);
            frameData.m_pFrameProcesser->SetCurrentFrame(frameData.m_pFrame, frameData.m_Size, frameData.m_EndWidth,
                                                            frameData.m_EndHeight, frameData.m_Pitch, frameData.m_BBP, frameData.m_ColorPlanes);

            frameData.m_pFrameProcesser->ApplyFilters(frameData.m_PhaseCount, progressCallback);
            frameData.m_pFrameProcesser->FrameDone(pFrame, size);

            // Release the bitmap lock.
            pLock->Release();
            wicBitmap = bitmap;
        }
    }
    return hr;
}

static HRESULT SaveToImageFile(const wchar_t *outFile)
{
    HRESULT hr = S_OK;

    FILE* fHandle = _wfopen(outFile, L"wb");
    if (!fHandle)
		return S_FALSE;

    fclose(fHandle);

    ComPtr<IWICBitmapEncoder> pEncoder = NULL;
    ComPtr<IWICBitmapFrameEncode> pFrameEncode = NULL;
    ComPtr<IWICStream> pStream = NULL;

    if (SUCCEEDED(hr))
        hr = wicFactory->CreateStream(&pStream);


    WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
    if (SUCCEEDED(hr))
        hr = pStream->InitializeFromFilename(outFile, GENERIC_WRITE);

    if (SUCCEEDED(hr))
        hr = wicFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);

    if (SUCCEEDED(hr))
        hr = pEncoder->Initialize(pStream.Get(), WICBitmapEncoderNoCache);

    if (SUCCEEDED(hr))
        hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);

    if (SUCCEEDED(hr))
        hr = pFrameEncode->Initialize(NULL);

    if (SUCCEEDED(hr))
        hr = pFrameEncode->SetSize(width, height);

    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->SetPixelFormat(&format);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->WriteSource(wicBitmap.Get(), NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->Commit();
    }
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->Commit();
    }

    return hr;
}

void Cartoonifier::TransformImage(const wchar_t *inFile, const wchar_t *outFile, ReportProgressCallback progressCallback)
{
    HRESULT hr;

    hr = LoadFromImageFile(inFile);
    if(SUCCEEDED(hr))
		hr = InternalTransformImage(progressCallback);
    if(SUCCEEDED(hr))
        hr = SaveToImageFile(outFile);
}