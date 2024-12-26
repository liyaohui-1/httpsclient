#include <memory>
#include "https_client.h"
#include "compress.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main()
{
    // HttpsClient& client = HttpsClient::getInstance("/path/to/ca_certificate.pem");  //需要设定实际的CA证书
    HttpsClient& client = HttpsClient::getInstance();  

    HttpHeader header;
    header.vin = "LNAAAAAAAP5012345";
    header.domain = 1;
    header.compressType = "1";
    header.version = "2.001";
    header.standardVersion = "2.0";

    client.SetUrl(std::string{"http://8.135.10.183:11476"});    //需要设定实际的url串
    client.SetHeader(header);

    client.RegisterCallback([](void* ptr, size_t size, size_t nmemb, void* userdata){ return size * nmemb;});
        
    if(!client.Init())
    {
        std::cout << "client.Init() Failed!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "client.Init() Successed!" << std::endl;
    }
    
    json j; // 首先创建一个空的json对象
    j["timestamp"] = 1647253105799;
    j["hardware_version"] = "A.1";
    j["sottware_version"] = "S.181020.V5.22.R";
    j["account_id"] = "0M300OCAVR183";
    j["device_id"] = "GAC5846515";
    j["device_brand"] = "德赛";
    j["device_os_name"] = "A06AVNT";
    j["platform"] = "android5";

    json j_body;
    j_body["tag"] = 3;
    j_body["service_id"] = "0x10C0";
    j_body["service_interface_id"] = "0x0001";
    j_body["fault_time_stamp"] = 1647253105799;
    j_body["fault_code"] = 51380242;
    j_body["fault_string"] = "大数据预处理文件压缩故障";
    j_body["fault_reason"] = "unable to locate the component";
    j_body["fault_detail"] = "";

    j["datas"].push_back(j_body);

    std::string data = j.dump();
    std::string destChar;

    if(!compress_string(data, destChar))
    {
        std::cout << "compress string error occur." << std::endl;
    }
    else
    {
        std::cout << "srcChar:  " << data << std::endl;
        std::cout << "destChar: " << destChar << std::endl;
    }

    if (client.SendData(destChar.c_str())) 
    {
        std::cout << "Send data Successed!" << std::endl;
    } 
    else 
    {
        // 重发多次失败之后保存为补发文件
        std::cerr << "Failed to send data" << std::endl;
    }

    return 0;
}