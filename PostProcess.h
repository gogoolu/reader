#pragma once
#include <string>

class OCR {
public:
	static void GenerateTextFromImage(const std::string& ImagePath, const std::string& OutputPath);
};

class NetworkConnection {
private:
	std::string ApiKey;
	std::string URL;
public:

	NetworkConnection(const std::string& ApiKey, const std::string& URL) 
		:ApiKey(ApiKey) {
		
	}

	void SendMsg(const std::string& Msg);
};