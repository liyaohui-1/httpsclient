#ifndef HTTPS_CLIENT_H_
#define HTTPS_CLIENT_H_

#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <unistd.h>
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
    using WriteCallback = size_t (*)(void*, size_t, size_t, void*);
public:
    HttpsClient(const std::string& ca_certificate_path);
    HttpsClient(const HttpsClient&) = default;
    HttpsClient& operator=(const HttpsClient&) = default;
    ~HttpsClient();

    void RegisterCallback(const WriteCallback& cb);

public:
    bool Init();

    void SetUrl(const std::string& url);
    void SetHeader(const HttpHeader& header);

    bool SendData(const char* data);
    bool GetApi();

private:
    CURL *curl_handle_;
    curl_slist* headers_ {nullptr};
    std::string ca_certificate_path_;
    std::string url_;
    WriteCallback cb;
};

#endif // HTTPS_CLIENT_H_