#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

std::string BuildBody(const std::string& Model, const std::string& ImageURL, const std::string& Text)
{
	json j;
	j["model"] = Model;
	j["messages"] = json::array();

	json message;
	message["role"] = "user";
	message["content"] = json::array();

	json ImgStmt;
	ImgStmt["type"] = "image_url";
	ImgStmt["image_url"] = ImageURL;
	j["messages"].push_back(std::move(ImgStmt));

	json TextStmt;
	TextStmt["type"] = "text";
	TextStmt["text"] = Text;
	j["messages"].push_back(std::move(TextStmt));

	return j.dump();
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t totalSize = size * nmemb;
	std::string* str = (std::string*)userp;
	str->append((char*)contents, totalSize);
	return totalSize;
}

void Request(const std::string& BaseURL, const std::string& ApiKey, const std::string& Body, const char* Buffer)
{
	CURL* curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, BaseURL.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Body.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, Body.size());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, Buffer);

		// set headers
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, std::format("Authorization:Bearer {}", ApiKey).c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

// upload image to server then get the image url
void UploadImage(const std::string& ImagePath, const std::string& ApiKey)
{
	std::string UploadImageURL = "https://dashscope.aliyuncs.com/compatible-mode/v1/files";

	CURL* curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		// set base url
		curl_easy_setopt(curl, CURLOPT_URL, UploadImageURL.c_str());

		// set headers
		std::string ApiKeyHeader = std::format("Authorization:Bearer {}", ApiKey);
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
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

		// send request
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));


		// cleanup
		curl_mime_free(mime);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

#if TEST_CODE
int main() {
	std::string BaseURL("https://dashscope.aliyuncs.com/compatible-mode/v1");
	std::string Body = BuildBody("qwen-vl-plus", "https://dashscope.oss-cn-beijing.aliyuncs.com/images/dog_and_girl.jpeg", "回答图片中的问题");
	std::ofstream ofs("Body_test.txt", std::ios::out);
	if (!ofs) {
		std::cout << "Error opening file for writing" << std::endl;
		return 1;
	}
	std::cout << Body << std::endl;
	ofs << Body;
	ofs.close();
	return 0;
}
#endif
