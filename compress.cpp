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
    dst_string.resize(nwrite);
    return true;
}

// 计算文件 CRC，加密压缩时必须使用
int getFileCrc(const char* filenameinzip, char* buf, unsigned long size_buf, unsigned long* result_crc)
{
	unsigned long calculate_crc = 0;
	int err = ZIP_OK;
	FILE * fin = fopen(filenameinzip, "rb");
	unsigned long size_read = 0;
	if (fin == nullptr)
	{
		err = ZIP_ERRNO;
	}

	if (err == ZIP_OK)
		do
		{
			err = ZIP_OK;
			size_read = fread(buf, 1, size_buf, fin);
			if (size_read < size_buf)
            {
				if (feof(fin) == 0)
				{
					printf("error in reading %s\n", filenameinzip);
					err = ZIP_ERRNO;
				}
            }

			if (size_read > 0)
            {
				calculate_crc = crc32_z(calculate_crc,(const Bytef *)buf, size_read);
            }
		} while ((err == ZIP_OK) && (size_read > 0));

		if (fin)
        {
			fclose(fin);
        }

		*result_crc = calculate_crc;
		printf("file %s crc %lx\n", filenameinzip, calculate_crc);
		return err;
}

int compress_zipAddFile(const std::string& file, zipFile& zf,const char *pw)
{
	int err=0;
	char buf[1024];
	int len;
	if(pw)
	{
		unsigned long crcFile;
		getFileCrc(file.c_str(), buf, sizeof(buf), &crcFile);
		err = zipOpenNewFileInZip3(zf, file.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, pw, crcFile);
	}
	else
	{
		err = zipOpenNewFileInZip3(zf, file.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0);
	}

	if (err != ZIP_OK)
	{
		return -1;
	}

	FILE* f = fopen(file.c_str(),"rb");
	if (f == nullptr)
	{
		return -2;
	}
	
	while ((len = fread(buf, 1, sizeof(buf), f)) > 0)
	{
		zipWriteInFileInZip(zf, buf, len);
	}
	fclose(f);
	zipCloseFileInZip(zf);
	return 0;
}

int compress_addDir(const std::string& sourcePath, zipFile& zf, const char* pw) {
    DIR* dir = opendir(sourcePath.c_str());
    if (!dir) {
        std::cout << "Error opening directory: " << sourcePath << std::endl;
        return -1;
    }
 
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 忽略 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }
 
        std::string fullPath = sourcePath + "/" + entry->d_name;
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) != 0) 
        {
            std::cerr << "Error stating file: " << fullPath << std::endl;
            closedir(dir);
            return -1;
        }
 
        if (S_ISDIR(statbuf.st_mode)) 
        {
            // 添加空目录到 zip 文件
            const char* zipEntryName = fullPath.c_str();
            int err = zipOpenNewFileInZip(zf, zipEntryName, nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION);
            if (err != ZIP_OK) 
            {
                std::cerr << "Error opening new file in zip: " << err << std::endl;
                closedir(dir);
                return -1;
            }
            zipCloseFileInZip(zf);
 
            // 递归调用以添加子目录内容
            if (compress_addDir(fullPath, zf, pw) != 0) 
            {
                closedir(dir);
                return -1;
            }
        } 
        else 
        {
            // 添加文件到 zip 文件
            if (compress_zipAddFile(fullPath, zf, pw) != 0) 
            {
                closedir(dir);
                return -1;
            }
        }
    }
 
    closedir(dir);
    return 0;
}

bool compress_zipdir(const std::string& sourcePath, const std::string& zipPath, const char *pw)
{
	zipFile zf = zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
	if (zf == nullptr)
	{
		return true;
	}
	zipOpenNewFileInZip3(zf, sourcePath.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0);
	zipCloseFileInZip(zf);
	int ret1 = compress_addDir(sourcePath, zf, pw);
	int ret2 = zipClose(zf, nullptr);
	return (ret1|| ret2);
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