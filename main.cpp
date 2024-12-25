#include <memory>
#include "https_client.h"
#include "zlib.h"

int main()
{
    // HttpsClient& client = HttpsClient::getInstance("/path/to/ca_certificate.pem");  //需要设定实际的CA证书
    HttpsClient& client = HttpsClient::getInstance();  

    HttpHeader header;
    header.vin = "LNAAAAAAAP5012345";
    header.domain = 1;
    header.compressType = "0";
    header.version = "2.001";
    header.standardVersion = "2.0";

    client.SetUrlAndHeader(std::string{"http://8.135.10.183:11476"}, header);    //需要设定实际的url串
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
 
    std::string data= {"{ { \"timestamp\":1647253105799, \"hardware_version\":\"A.1\", \"sottware_version\":\"S.181020.V5.22.R\", \"account_id\":\"0M300OCAVR183\", \"device_id\":\"GAC5846515\", \"device_brand\":\"德赛\", \"device_os_name\":\"A06AVNT\", \"platform\":\"android5\", \"body\":[ { \"tag\":3, \"service_id\":\"0x10C0\", \"service_interface_id\":\"0x0001\", \"fault_time_stamp\":1647253105799, \"fault_code\":51380242, \"fault_string\":\"大数据预处理文件压缩故障\", \"fault_reason\":\"unable to locate the component\", \"fault_detail\":\"\" } ] } }" };
    std::string destChar;
    unsigned long nread,nwrite;

    nread = data.length();
    destChar.resize(nread);
    nwrite = compressBound(nread);

    if (compress((Bytef*)destChar.c_str(), &nwrite, (const Bytef*)data.c_str(), nread) != Z_OK)
    {
        printf("compress error occur.\n");
        return -2;
    }
    else
    {
        printf("srcChar:{%s}.\n",data.c_str());
        printf("destChar:{%s}.\n",destChar.c_str());
    }

    if (client.SendData(destChar.c_str())) 
    {
        std::cout << "Send data Successed!" << std::endl;
    } 
    else 
    {
        std::cerr << "Failed to send data" << std::endl;
    }

    return 0;
}