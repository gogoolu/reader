#pragma once
#include <string>

namespace Network {
	std::string BuildBody(const std::string& Model, const std::string& ImageId, 
		const std::string& SystemPrompt, const std::string& UserPrompt);

	void Request(const std::string& ApiKey, const std::string& Body, std::string& OutBuffer);
	void UploadImage(const std::string& ImagePath, const std::string& ApiKey, std::string& OutBuffer);
}
