#include "https_client.h"

HttpsClient::HttpsClient(const std::string& ca_certificate_path)
        : ca_certificate_path_(ca_certificate_path), curl_handle_(nullptr) 
{

}

HttpsClient::~HttpsClient()
{
    if(curl_handle_)
    {
	    curl_easy_cleanup(curl_handle_);
    }
    curl_global_cleanup();
}

void HttpsClient::SetUrl(const std::string& url)
{
    url_ = url;
}

void HttpsClient::SetHeader(const HttpHeader& header)
{
    headers_ = curl_slist_append(headers_, "Content-Type:application/json");
    headers_ = curl_slist_append(headers_, "Accept:application/json");
    headers_ = curl_slist_append(headers_, (std::string{"vin:"}+header.vin).c_str());
    headers_ = curl_slist_append(headers_, (std::string{"domain:"}+std::to_string(header.domain)).c_str());
    headers_ = curl_slist_append(headers_, (std::string{"compressType:"}+header.compressType).c_str());
    headers_ = curl_slist_append(headers_, (std::string{"version:"}+header.version).c_str());
    headers_ = curl_slist_append(headers_, (std::string{"standardVersion:"}+header.standardVersion).c_str());
}


void HttpsClient::RegisterCallback(const WriteCallback& callback)
{
    cb = callback;
}

bool HttpsClient::Init()
{
    if (!curl_handle_) 
    { 
        curl_handle_ = curl_easy_init();
        if(!curl_handle_)
        {
            throw std::runtime_error("Failed to initialize libcurl");
            return false;
        }
    }
    
    if(access(ca_certificate_path_.c_str(), F_OK) != -1)
    {
        // 如果有服务端的CA证书，则启用SSL验证
        std::cout << "Using CA certificate file: " << ca_certificate_path_ << std::endl;
        curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYHOST, true);
        curl_easy_setopt(curl_handle_, CURLOPT_CAINFO, ca_certificate_path_.c_str());
    }
    else
    {
        // 如果没有服务端的CA证书，则跳过SSL验证
        std::cout << "Warning: CA certificate file not found, skipping SSL verification" << std::endl;
        curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYHOST, false);
    }

    if (!url_.empty()) 
    {
        curl_easy_setopt(curl_handle_, CURLOPT_HTTPHEADER,      headers_);
        curl_easy_setopt(curl_handle_, CURLOPT_URL,             url_.c_str());
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION,   cb);
        curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT,         1);
    } 
    else 
    {
        throw std::invalid_argument("URL is empty");
        return false;
    }

    return true;
}

bool HttpsClient::SendData(const char* data)
{
    std::cout << "Sending data to " << url_ <<" begin!" <<std::endl;
    curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, data);

    CURLcode res = curl_easy_perform(curl_handle_);
    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return false;
    }
    std::cout << std::endl << "Sending data to " << url_ <<" end!" << std::endl;
    return true;
}

bool HttpsClient::GetApi()
{
    CURLcode res = curl_easy_perform(curl_handle_);
    if(res)
    {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    return true;
}