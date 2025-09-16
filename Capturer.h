#pragma once
#include <windows.h>
#include <string>

namespace Capturer {
	bool SaveBitmapToFile(BYTE* pBitmapBits, LONG width, LONG height, WORD bitsPerPixel, const std::string& filePath);
	int Capture(const std::string& ImagePath);
};