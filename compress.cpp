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