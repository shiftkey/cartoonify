#pragma once
#include <functional>

namespace Cartoonifier
{
	typedef std::function<void(int)> ReportProgressCallback;
    __declspec(dllexport) void TransformImage(const wchar_t *inFile, const wchar_t* outFile, ReportProgressCallback progressCallback);
}