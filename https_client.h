#ifndef HTTPS_CLIENT_H_
#define HTTPS_CLIENT_H_

#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include "curl/curl.h"

#define HTTPS 0

typedef struct{
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
#if HTTPS
    HttpsClient(const std::string& ca_certificate_path);
#else
    HttpsClient();
#endif
    HttpsClient(const HttpsClient&) = delete;
    HttpsClient& operator=(const HttpsClient&) = delete;
    ~HttpsClient();

    void RegisterCallback(const WriteCallback& cb);
 
#if HTTPS
    获取单例实例
    static HttpsClient& getInstance(const std::string& ca_certificate_path) {
        static std::unique_ptr<HttpsClient> instance;
        static std::once_flag onceFlag;
 
        std::call_once(onceFlag, [&]() {
            instance.reset(new HttpsClient(ca_certificate_path));
            curl_global_init(CURL_GLOBAL_ALL);
        });
 
        return *instance;
    }
#else
    static HttpsClient& getInstance() {
        static std::unique_ptr<HttpsClient> instance;
        static std::once_flag onceFlag;
 
        std::call_once(onceFlag, [&]() {
            instance.reset(new HttpsClient());
            curl_global_init(CURL_GLOBAL_ALL);
        });
 
        return *instance;
    }
#endif


public:
    bool Init();

    void SetUrl(const std::string& url);
    void SetHeader(const HttpHeader& header);

    bool SendData(const char* data);

private:
    CURL *curl_handle_;
    curl_slist* headers_ = NULL;
#if HTTPS
    std::string ca_certificate_path_;
#endif
    std::string url_;
    WriteCallback cb;
};

#endif