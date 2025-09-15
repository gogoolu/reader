#include "Capturer.h"
#include "PostProcess.h"
#include <string>

int main() {
	std::string ImagePath = "./temp/capture.bmp";
	std::string TextPath = "./temp/output.txt";
	Capturer::Capture(ImagePath);
	OCR::GenerateTextFromImage(ImagePath, TextPath);
	return 0;
}
