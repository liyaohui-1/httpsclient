#include "https_client.h"
#include "nlohmann/json.hpp"
#include <fstream>

using json = nlohmann::json;

HttpsClient::HttpsClient(const std::string& ca_certificate_path)
        : ca_certificate_path_(ca_certificate_path) 
{
    curl_global_init(CURL_GLOBAL_ALL);
    multiHandle_ = curl_multi_init();
}

HttpsClient::~HttpsClient()
{
    if(workerThread_.joinable())
    {
        workerThread_.join();
    }

    for (auto& handle : curlHandles_) 
    {
        if (handle) 
        {
            curl_multi_remove_handle(multiHandle_, handle);
            curl_easy_cleanup(handle);
        }
    }
    curl_multi_cleanup(multiHandle_);
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

bool HttpsClient::InitCURLHandle(CURL* curl_handle)
{
    if(access(ca_certificate_path_.c_str(), F_OK) != -1)
    {
        // 如果有服务端的CA证书，则启用SSL验证
        std::cout << "Using CA certificate file: " << ca_certificate_path_ << std::endl;
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, true);
        curl_easy_setopt(curl_handle, CURLOPT_CAINFO, ca_certificate_path_.c_str());
    }
    else
    {
        // 如果没有服务端的CA证书，则跳过SSL验证
        std::cout << "Warning: CA certificate file not found, skipping SSL verification" << std::endl;
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, false);
    }

    if (!url_.empty()) 
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL,             url_.c_str());
    } 
    else 
    {
        throw std::invalid_argument("URL is empty");
        return false;
    }

    return true;
}

bool HttpsClient::AddRequest(FileFormat& fileFormat)
{
    CURL* handle = curl_easy_init();
    if (!handle) 
    {
        std::cout << "Failed to initialize CURL handle." << std::endl;
        return false;
    }
    if(!InitCURLHandle(handle))
    {
        std::cout << "InitCURLHandle error." << std::endl;
        return false;
    }

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER,      headers_);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION,   WriteCallback);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS,      fileFormat.data.c_str());

    curlHandles_.push_back(handle);
    curl_multi_add_handle(multiHandle_, handle);

    if(postData_.find(handle) == postData_.end())
    {
        postData_.emplace(handle, fileFormat);
    }

    return true;
}

void HttpsClient::SaveReissueData(const FileFormat& fileFormat)
{
    json j_resend;
    std::cout << "Failed to send data, save to resend file." << std::endl;
    // 保存为补发文件格式：域名_域内节点名_业务类型_功能模块_ID_功能触发ID_时间_分包符_结束包符_0
    std::string resend_file_name = fileFormat.domain_name + "_" + fileFormat.node_name + "_" + fileFormat.business_type + "_" +  \
                                   fileFormat.function_module_id + "_" + fileFormat.function_trigger_id + "_" +  \
                                   std::to_string(fileFormat.trigger_timestamp) + "_" + fileFormat.package_separator + "_" +  \
                                   fileFormat.end_separator + "_0";
    j_resend["array"].push_back(fileFormat.data.c_str());
    std::cout << "resend data: " << j_resend.dump(4) << std::endl;

    std::ofstream ofs(resend_file_name, std::ios::app | std::ios::binary);
    if(!ofs.is_open())
    {
        std::cout << "Failed to open file: " << resend_file_name << std::endl;
        return;
    }
    ofs << j_resend.dump(4);
    ofs.close();
}

void HttpsClient::PerformRequests()
{
    int stillRunning = 0;
    CURLMcode multiRes = curl_multi_perform(multiHandle_, &stillRunning);

    if (multiRes != CURLM_OK) 
    {
        std::cerr << "curl_multi_perform() failed, code " << multiRes << "." << std::endl;
        return;
    }

    while (stillRunning) 
    {
        int numfds;
        CURLMcode mc = curl_multi_wait(multiHandle_, nullptr, 0, 1000, &numfds);
        if (mc != CURLM_OK) 
        {
            std::cerr << "curl_multi_wait() failed, code " << mc << "." << std::endl;
            break;
        }

        curl_multi_perform(multiHandle_, &stillRunning);
    }

    for (size_t i = 0; i < curlHandles_.size(); ++i) 
    {
        long responseCode = 0;
        CURLcode res = curl_easy_getinfo(curlHandles_[i], CURLINFO_RESPONSE_CODE, &responseCode);
        if(res != CURLE_OK)
        {
            std::cerr << "curl_easy_getinfo() "<< "[" << i << "]" <<" failed, code " << res << "." << std::endl;
            continue;
        }

        std::cout << "Request " << i << " response code: " << responseCode << std::endl;
        if (responseCode >= 200 && responseCode < 300) 
        {
            std::cout <<"curlHandles_[" << i <<"]:" <<"Request succeeded." << std::endl;
            break;
        }
        else
        {
            SaveReissueData(postData_[curlHandles_[i]]);
            std::cout << "curlHandles_[" << i <<"]:"<<"Request failed with HTTP status code: " << responseCode << std::endl;
        }

        curl_multi_remove_handle(multiHandle_, curlHandles_[i]);
        curl_easy_cleanup(curlHandles_[i]);
    }

    postData_.clear();
    curlHandles_.clear();
}

void HttpsClient::StartPerformRequests()
{
    workerThread_ = std::thread(&HttpsClient::PerformRequests, this);
}