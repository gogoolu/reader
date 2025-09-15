#include "PostProcess.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>


void OCR::GenerateTextFromImage(const std::string& ImagePath, const std::string& OutputPath) {
	cv::Mat img = cv::imread(ImagePath);
	if (img.empty()) {
		std::cerr << "read image failed\n";
		return;
	}
	cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

	tesseract::TessBaseAPI tess;
	tess.Init("D:/softwares/vcpkg/installed/x64-windows/share/tessdata", "chi_sim"); // 语言包：eng, chi_sim 等
	tess.SetImage((uchar*)img.data, img.size().width, img.size().height,
		img.channels(), img.step1());
	const char* textBytes = tess.GetUTF8Text();
	std::ofstream ofs(OutputPath, std::ios::binary);
	ofs.write(textBytes, strlen(textBytes));
	ofs.close();
	delete[] textBytes;

	std::cout << "======= Finish Image OCR ======\n";
}
