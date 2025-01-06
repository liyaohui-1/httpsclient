#include <fstream>
#include <memory>
#include "https_client.h"
#include "compress.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static constexpr const uint8_t MAX_SEND_TIMES = 3;

int main()
{
    HttpsClient client {"ca_certificate_path"};
    HttpHeader header;
    header.vin = "LNAAAAAAAP5012345";
    header.domain = 1;
    header.compressType = "1";
    header.version = "2.001";
    header.standardVersion = "2.0";

    client.SetUrl(std::string{"https://www.baidu.com"});  //测试用百度
    // client.SetUrl(std::string{"https://bc-v2c-eea2-servicedatasync.gacicv.com/"});
    client.SetHeader(header);

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

    std::string base64 = base64_encode(destChar);
    std::cout << "base64: " << base64 << std::endl;
    
    FileFormat tmp;
    tmp.domain_name = "example.com";
    tmp.node_name = "node123";
    tmp.business_type = "serviceA";
    tmp.function_module_id = "module001";
    tmp.function_trigger_id = "trigger005";
    tmp.trigger_timestamp = 1647253105799;
    tmp.package_separator = "001";
    tmp.end_separator = "010";
    tmp.data = base64;
    client.AddRequest(tmp);

    client.StartPerformRequests();

    std::cout << "Waiting for requests to complete..." << std::endl;

    return 0;
}