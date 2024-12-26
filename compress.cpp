#include "compress.h"

bool compress_string(const std::string& src_string, std::string& dst_string)
{
    unsigned long nread,nwrite;

    nread = src_string.length();
    dst_string.resize(nread);
    nwrite = compressBound(nread);

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