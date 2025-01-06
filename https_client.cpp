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

    for (auto& t : threads)
    {
        if(t.joinable())
        {
            t.join(); 
        }
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
    curl_slist_free_all(headers_);
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
    if(curl_multi_perform(multiHandle_, &stillRunning) != CURLM_OK)
    {
        std::cout << "curl_multi_perform() failed !"<< std::endl;
        return;
    }

    while (stillRunning) 
    {
        int numfds;
        if(curl_multi_wait(multiHandle_, nullptr, 0, 1000, &numfds) != CURLM_OK)
        {
            std::cout << "curl_multi_wait() failed!" << std::endl;
            break;
        }

        curl_multi_perform(multiHandle_, &stillRunning);
    }

    for (size_t i = 0; i < curlHandles_.size(); ++i) 
    {
        long responseCode = 0;
        if(curl_easy_getinfo(curlHandles_[i], CURLINFO_RESPONSE_CODE, &responseCode) != CURLE_OK)
        {
            std::cout << "curl_easy_getinfo() "<< "[" << i << "]" <<" failed!" << std::endl;
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

// 获取已上传的文件大小（通过HTTP HEAD请求获取服务器上对应资源的Content-Length）
uint32_t HttpsClient::GetUploadSize(const std::string& url)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 50L); // 设置50毫秒（5秒）连接超时
        
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            double contentLength;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength); 
            curl_easy_cleanup(curl);
            return (uint32_t)contentLength;
        }
        
        curl_easy_cleanup(curl);
    }
    return 0;
}

// 每个线程执行的函数体，用于上传指定范围的数据块
void HttpsClient::UploadChunkThread(const std::string& url, int uploadedSize, int start, int end, int threadID, std::ifstream& file)
{
    curl_off_t rangeStart = start;
    curl_off_t rangeEnd = end;
    
    std::stringstream headerRange;
    headerRange << "Range: bytes=" << rangeStart << "-" << rangeEnd;
    
    CURL *curl = curl_easy_init();
    if (curl)
    {
        file.seekg(start);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L); // 设置为上传请求
        curl_easy_setopt(curl, CURLOPT_READDATA, &file);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, CHUNK_SIZE);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr); // 不需要返回body内容
        
        headers_ = curl_slist_append(headers_, headerRange.str().c_str());
        
        // 添加断点续传的相关选项
        if (threadID > 0)
        {
            curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, uploadedSize + start);
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_);
    
		// 执行上传请求
		if(curl_easy_perform(curl) == CURLE_OK)
        {
			std::cout << "Thread " << threadID << " upload success!" << std::endl;
        }
		else
        {
			std::cout << "Thread " << threadID << " upload failed! "  << std::endl;
        }

        curl_easy_cleanup(curl);
    }
}

void HttpsClient::OnFileSizeOver50MB(std::string& file_path)
{
    // 单个文件大小超过50MB时，需要分包发送
    if(compress_zipdir(file_path, file_path + ".zip", nullptr))
    {
        std::cout << "compress_zipdir success." << std::endl;
    }
    else
    {
        std::cout << "compress_zipdir failed." << std::endl;
        return;
    }

    uint32_t fileSize = get_file_size(file_path + "zip"); // 获取文件总大小
    uint32_t upLoadSize = GetUploadSize(url_);

    int chunkNum = (fileSize - upLoadSize + CHUNK_SIZE - 1) / CHUNK_SIZE; // 计算需要分块的数量

	// 创建多个线程，每个线程负责上传一个数据块，线程的创建数量与分块的数量一致
	for (int i = 0; i < chunkNum; ++i)
	{
		int start = i * CHUNK_SIZE;
		int end = (i == chunkNum - 1) ? fileSize : start + CHUNK_SIZE - 1;
		std::thread t(UploadChunkThread, url_, upLoadSize, start, end, i, std::ifstream(file_path + "zip", std::ios::binary));
		threads.push_back(std::move(t));
	}
}