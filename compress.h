#ifndef COMPRESS_H
#define COMPRESS_H

#include <string>
#include <iostream>
#include "zlib.h"

bool compress_string(const std::string& src_string, std::string& dst_string);

bool compress_file(const std::string& src_file, const std::string& dst_file);

#endif