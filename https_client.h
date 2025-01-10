#ifndef HTTPS_CLIENT_H_
#define HTTPS_CLIENT_H_

#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <unordered_map>
#include <mutex>
#include "compress.h"
#include "curl/curl.h"
#include "thread_pool.h"

#define CHUNK_SIZE (1024*1024*50) // 50MB
#define MAX_SEND_FAIL_TIMES 3 // 最大发送失败次数

typedef struct{
    std::string vin;             // 车辆vin码
    uint8_t domain {0};          // 业务域id
    std::string compressType;    // 压缩算法
    std::string version;         // 采集配置文件的版本号
    std::string standardVersion; // 使用的埋点数据采集规范版本
}HttpHeader;

typedef struct{
    std::string domain_name;         // 域名
    std::string node_name;           // 域内节点名
    std::string business_type;       // 业务类型
    std::string function_module_id;  // 功能模块ID
    std::string function_trigger_id; // 功能触发ID
    uint64_t    trigger_timestamp;   // 触发时间戳
    std::string package_separator;   // 分包符
    std::string end_separator;       // 结束符
    std::string data;                // 数据
}FileFormat;

class HttpsClient
{
public:
    HttpsClient(const std::string& ca_certificate_path);
    ~HttpsClient();

public:
    void SetUrl(const std::string& url);
    void SetHeader(const HttpHeader& header);

    bool AddRequest(const FileFormat& fileFormat);
    void StartPerformRequests();
    void SaveReissueData(const FileFormat& fileFormat);
    void OnFileSizeOver(const std::string& file_path);

private:
    bool InitCURLHandle(CURL* curl_handle);
    void PerformRequests();
    void UploadChunkThread(int start, int end, int threadID, const std::string& file_path);

    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) 
    {
        return size * nmemb;
    }

    std::vector<CURL*> curlHandles_;
    CURLM* multiHandle_;
    std::unordered_map<CURL* ,FileFormat> postData_;

    curl_slist* headers_ {nullptr};
    HttpHeader httpheader_;
    std::string ca_certificate_path_;
    std::string url_;
    
    std::thread workerThread_;
    ThreadPool threadPool_;

    std::mutex mutex_;
    std::atomic_bool stop_ {false};
};

#endif // HTTPS_CLIENT_H_