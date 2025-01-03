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
    static json j_resend;

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

    client.AddRequest(base64.c_str());

    client.StartPerformRequests();

    std::cout << "Waiting for requests to complete..." << std::endl;

    /*
    for(uint32_t send_times = 1; send_times <= MAX_SEND_TIMES; send_times++)
    {
        if (client.SendData(base64.c_str())) 
        {
            std::cout << "Send data Successed!" << std::endl;
            break;
        }
        else
        {
            std::cout << "Failed to PerformRequests send data times: " << send_times << std::endl;
        }

        if(send_times == MAX_SEND_TIMES)
        {
            // 重发多次失败之后保存为补发文件(暂定为重发MAX_SEND_TIMES次之后)
            std::cout << "Failed to send data, save to resend file." << std::endl;
            // 保存为补发文件格式：域名_域内节点名_业务类型_功能模块 ID_功能触发ID_时间_分包符_结束包符_0
            std::string resend_file_name = "example.com_node123_serviceA_module001_trigger005_20231005143000_001_010_0";
            j_resend["array"].push_back(base64);
            std::cout << "resend data: " << j_resend.dump(4) << std::endl;

            std::ofstream ofs(resend_file_name, std::ios::app | std::ios::binary);
            if(!ofs.is_open())
            {
                std::cout << "Failed to open file: " << resend_file_name << std::endl;
                return 0;
            }
            ofs << j_resend.dump(4);
            ofs.close();
        }
    }
    */

/*
    if(client.GetApi())
    {
        std::cout << "GetApi Successed!" << std::endl;
    }
    else
    {
        std::cout << "Failed to GetApi!" << std::endl;
    }

    compress_zipdir("test/","./test.zip",nullptr);
*/

    return 0;
}