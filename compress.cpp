#include "compress.h"

bool compress_string(const std::string& src_string, std::string& dst_string)
{
    unsigned long nread,nwrite;

    nread = src_string.length();
    nwrite = compressBound(nread);
    dst_string.resize(nwrite);

    if (compress((Bytef*)dst_string.c_str(), &nwrite, (const Bytef*)src_string.c_str(), nread) != Z_OK)
    {
        return false;
    }
    return true;
}

bool compress_file(const std::string& src_file, const std::string& dst_file)
{
    return true;
}

std::string base64_encode(const std::string &input) 
{  
    std::string encoded;  
    size_t i = 0, j = 0;  
    uint8_t byte3[3] = {0};  
    uint8_t byte4[4] = {0};  

    for (char byte : input) 
    {  
        byte3[i++] = static_cast<uint8_t>(byte); 
        if (i == 3) 
        {  
            byte4[0] = (byte3[0] & 0xfc) >> 2;  
            byte4[1] = ((byte3[0] & 0x03) << 4) | ((byte3[1] & 0xf0) >> 4);  
            byte4[2] = ((byte3[1] & 0x0f) << 2) | ((byte3[2] & 0xc0) >> 6);  
            byte4[3] = byte3[2] & 0x3f;  

            for (int k = 0; k < 4; k++) 
            {  
                encoded += base64_chars[byte4[k]];  
            }  
            i = 0;  
        }  
    }  

    if (i != 0) 
    {  
        for (size_t k = i; k < 3; k++) 
        {  
            byte3[k] = 0;
        }

        byte4[0] = (byte3[0] & 0xfc) >> 2;  
        byte4[1] = ((byte3[0] & 0x03) << 4) | ((byte3[1] & 0xf0) >> 4);  
        byte4[2] = ((byte3[1] & 0x0f) << 2) | ((byte3[2] & 0xc0) >> 6);  

        for (size_t k = 0; k < i + 1; k++) 
        {  
            encoded += base64_chars[byte4[k]];  
        }  

        while (i++ < 3) 
        {  
            encoded += '=';  
        }  
    }  
    std::cout << "base64 size:" << encoded.size() << std::endl;

    return encoded;  
}