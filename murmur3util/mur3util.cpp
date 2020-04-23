#include <fstream>
#include <iostream>
#include <memory>

#include "../save_cryptor/smhasher/src/MurmurHash3.h"

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [path]" << std::endl;
        return -1;
    }

    const char** paths = &argv[1];
    int paths_count = argc - 1;
    for (int i = 0; i < paths_count; i++) {
        // read file into buffer
        std::string in_path = std::string(paths[i]);
        std::ifstream fileIS(in_path, std::ios::binary | std::ios::ate);
        if (not fileIS.good()) {
            std::cout << "Could not open file " << in_path << std::endl;
            return -2;
        }
        long fileSize = fileIS.tellg();
        auto fileBuffer = std::make_unique<uint8_t[]>(fileSize);
        fileIS.seekg(0, std::ios::beg);
        fileIS.read(reinterpret_cast<char*>(fileBuffer.get()), fileSize);

        uint32_t calcedHash;
        MurmurHash3_x86_32(fileBuffer.get(), fileSize, 0, &calcedHash);
        std::cout << std::hex << calcedHash << " " << in_path << std::endl;
    }

    return 0;
}
