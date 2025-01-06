#ifndef COMPRESS_H
#define COMPRESS_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include "zlib.h"
#include "zip.h"

const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

// 压缩字符串接口,src_string为待压缩的字符串,dst_string为压缩后的字符串
bool compress_string(const std::string& src_string, std::string& dst_string);

// 计算文件 CRC，加密压缩时必须使用
int getFileCrc(const char* filenameinzip, char* buf, unsigned long size_buf, unsigned long* result_crc);

// 压缩一个文件,file为文件名,zf为压缩文件句柄,pw为压缩密码
int compress_zipAddFile(const std::string& file, zipFile& zf,const char *pw);

// 添加文件夹到压缩文件接口,sourcePath为文件夹路径,zf为压缩文件句柄,pw为压缩密码
int compress_addDir(const std::string& sourcePath, zipFile& zf,const char *pw);

// 压缩文件夹接口,sourcePath为待压缩的文件夹路径,zipPath为压缩后的文件路径(完整路径+文件名),pw为压缩密码
bool compress_zipdir(const std::string& sourcePath, const std::string& zipPath, const char *pw);

// base64编码接口
std::string base64_encode(const std::string &input);

// 获取文件总大小
uint32_t get_file_size(const std::string& file_path);

#endif