#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include "Network.h"

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t totalSize = size * nmemb;
	std::string* str = (std::string*)userp;
	str->append((char*)contents, totalSize);
	return totalSize;
}

std::string Network::BuildBody(const std::string& Model, const std::string& ImageId, 
	const std::string& SystemPrompt, const std::string& UserPrompt)
{
	json j;
	j["model"] = Model;
	j["messages"] = json::array();

	json message;
	message["role"] = "system";
	message["content"] = SystemPrompt;
	j["messages"].push_back(std::move(message));

	json message1;
	message1["role"] = "system";
	message1["content"] = std::format("fileid://{}", ImageId);
	j["messages"].push_back(std::move(message1));

	json message2;
	message2["role"] = "user";
	message2["content"] = UserPrompt;
	j["messages"].push_back(std::move(message2));

	return j.dump();
}

void Network::Request(const std::string& ApiKey, const std::string& Body, std::string& OutBuffer)
{
	std::string BaseURL("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, BaseURL.c_str());

		// set write callback
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &OutBuffer);

		// set headers
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, std::format("Authorization: Bearer {}", ApiKey).c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// set data
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Body.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, Body.size());


		// send request
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
}

// upload image to server then get the image url
void Network::UploadImage(const std::string& ImagePath, const std::string& ApiKey, std::string& OutBuffer)
{
	std::string UploadImageURL = "https://dashscope.aliyuncs.com/compatible-mode/v1/files";

	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		// set base url
		curl_easy_setopt(curl, CURLOPT_URL, UploadImageURL.c_str());

		// set headers
		std::string ApiKeyHeader = std::format("Authorization: Bearer {}", ApiKey);
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, ApiKeyHeader.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// set form-data
		curl_mime* mime;
		curl_mimepart* part;
		mime = curl_mime_init(curl);

		// add file
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "file");
		curl_mime_filedata(part, ImagePath.c_str());

		// add purpose
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "purpose");
		curl_mime_data(part, "file-extract", CURL_ZERO_TERMINATED);

		// set form-data to curl
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		// set write callback
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &OutBuffer);

		// send request
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		// cleanup
		curl_mime_free(mime);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		std::cout << "Upload Image finished" << std::endl;
	}
}
