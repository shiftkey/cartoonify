#pragma once

using namespace Windows::Foundation;

namespace imaging
{
    public ref class WinRTComponent sealed
    {
    public:
        WinRTComponent();
		IAsyncActionWithProgress<int>^ TransformImageAsync(Platform::String^ inFile, Platform::String^ outFile);
    };
}