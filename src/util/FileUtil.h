
#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <iostream>
#include <vector>
#include <fstream>


class FileUtil {
public:
    static std::vector<char> readFile(const std::string& filename) {
        const std::string res = std::filesystem::absolute(filename);
        std::ifstream file(res, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: '" + res + "'");
        }

        const long fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
};



#endif //FILEUTIL_H
