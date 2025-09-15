#pragma once
#include <windows.h>
#include <string>

class Capturer {
private:
	static bool SaveBitmapToFile(BYTE* pBitmapBits, LONG width, LONG height, WORD bitsPerPixel, const std::string& filePath);
public:
	static int Capture(const std::string& ImagePath);
};