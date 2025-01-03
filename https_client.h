#ifndef HTTPS_CLIENT_H_
#define HTTPS_CLIENT_H_

#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <atomic>
#include "curl/curl.h"

typedef struct HttpHeader{
    std::string vin;             // 车辆vin码
    uint8_t domain {0};          // 业务域id
    std::string compressType;    // 压缩算法
    std::string version;         // 采集配置文件的版本号
    std::string standardVersion; // 使用的埋点数据采集规范版本
}HttpHeader;

class HttpsClient
{
public:
    HttpsClient(const std::string& ca_certificate_path);
    ~HttpsClient();

public:
    void SetUrl(const std::string& url);
    void SetHeader(const HttpHeader& header);

    bool AddRequest(const std::string& postData);
    void StartPerformRequests();

private:
    bool InitCURLHandle(CURL* curl_handle);
    void PerformRequests();

    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) 
    {
        return size * nmemb;
    }

    std::vector<CURL*> curlHandles_;
    std::atomic<CURLM*> multiHandle_;

    curl_slist* headers_ {nullptr};
    std::string ca_certificate_path_;
    std::string url_;
    
    std::thread workerThread_;
};

#endif // HTTPS_CLIENT_H_