// WinRTComponent.cpp
#include "pch.h"
#include "WinRTComponent.h"

#include "Cartoonifier.h"

#include <ppltasks.h>
using namespace Concurrency;

using namespace imaging;
using namespace Platform;

WinRTComponent::WinRTComponent()
{
}

IAsyncActionWithProgress<int>^  WinRTComponent::TransformImageAsync(Platform::String^ inFile, Platform::String^ outFile)
{
	return create_async([=] (progress_reporter<int> progress) {
		Cartoonifier::TransformImage(inFile->Data(), outFile->Data(), [progress] (int percent) 
		{progress.report(percent);});
	});
}