#ifndef COMPRESS_H
#define COMPRESS_H

#include <string>
#include <iostream>
#include "zlib.h"

const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

bool compress_string(const std::string& src_string, std::string& dst_string);

bool compress_file(const std::string& src_file, const std::string& dst_file);

std::string base64_encode(const std::string &input);

#endif