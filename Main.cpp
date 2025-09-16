#include "Capturer.h"
#include "Network.h"
#include "nlohmann/json.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <curl/curl.h>

using json = nlohmann::json;

int main() {
	const std::string ConfigPath = "./config.json";
	std::ifstream ConfigFile(ConfigPath);
	if (!ConfigFile.is_open()) {
		std::cout << "Open file failed: " << "./config.json" << std::endl;
		return 1;
	}
	json Config;

	ConfigFile >> Config;
	ConfigFile.close();

	const std::string ImagePath = Config["ImagePath"];
	const std::string Apikey = Config["ApiKey"];
	const std::string SystemPrompt = Config["SystemPrompt"];
	const std::string UserPrompt = Config["UserPrompt"];
	const std::string Model = Config["Model"];
	const std::string OutputPath = Config["OutputPath"];

	std::cout << "======== Start capture screen ========" << std::endl;
	Capturer::Capture(ImagePath);

	curl_global_init(CURL_GLOBAL_ALL);

	std::string UploadImageRes;
	Network::UploadImage(ImagePath, Apikey, UploadImageRes);
	std::cout << "UploadImageRes: " << UploadImageRes << std::endl;

	json ImageRes = json::parse(UploadImageRes);
	std::string ImageId = ImageRes["id"];
	std::string Body = Network::BuildBody(Model, ImageId, SystemPrompt, UserPrompt);

	std::cout << Body << std::endl;

	std::string OutputText;
	Network::Request(Apikey, Body, OutputText);

	curl_global_cleanup();

	std::cout << "Response Text: " << OutputText << std::endl;
	
	std::ofstream OutputFile(OutputPath);
	if (!OutputFile.is_open()) {
		std::cout << "Open file failed: " << OutputPath << std::endl;
		return 1;
	}

	OutputFile << OutputText;

	return 0;
}
