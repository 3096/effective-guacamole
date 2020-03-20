#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>
#include <utility>

#include "MurmurHash3.h"

struct MurmurHashSection {
    long hashOffset;
    long sectionLen;
};
std::unordered_map<std::string, std::list<MurmurHashSection>> FILENAME_HASHSECTIONS_MAP;

void processPath(std::string basePath) {
    const std::string SAVE_EXT = ".dat";
    const std::string HEADER_SUFFIX = "Header";
    constexpr long HEADER_INFO_SECTION_SIZE = 0x100;

    // don't hash header files
    if (basePath.find(HEADER_SUFFIX) != std::string::npos) return;

    // read file
    std::ifstream fileIS(basePath, std::ios::binary | std::ios::ate);
    if (not fileIS.good()) {
        std::cout << "Could not open file " << basePath << std::endl;
        return;
    }
    long fileSize = fileIS.tellg();
    auto fileBuffer = std::make_unique<char[]>(fileSize);
    fileIS.seekg(0, std::ios::beg);
    fileIS.read(fileBuffer.get(), fileSize);

    // find section matches for every hash and add to FILENAME_HASHSECTIONS_MAP
    std::list<MurmurHashSection> hashSectionList;
    long curOffset = HEADER_INFO_SECTION_SIZE;  // this seems like a good position to start
    while (curOffset < fileSize) {
        // search for hash
        uint32_t curHash;
        long curHashOffset = curOffset;
        while (true) {
            if (not(curHashOffset < fileSize)) {  // so far all hashed sections ends at EOF, so this shouldn't happen
                throw std::range_error("hash search reached EOF");
            }

            curHash = *reinterpret_cast<uint32_t*>(&fileBuffer[curHashOffset]);
            if (((curHash & 0xFF) > 0) + ((curHash & 0xFF00) > 0) + ((curHash & 0xFF0000) > 0) +
                    ((curHash & 0xFF000000) > 0) >
                2) {  // assmuse if at least 2 bytes aren't 0, it's a hash? So far it worked ¯\_(ツ)_/¯
                std::cout << "    hash search skipped 0x" << std::hex << curHashOffset - curOffset << " bytes"
                          << std::endl;
                break;
            }
            curHashOffset += 4;
        }

        // find matching hash with the following data
        curOffset = curHashOffset + 4;
        int curSectionSize = MurmurHash3_32_FindMatch(&fileBuffer[curOffset], fileSize - curOffset, 0, curHash);
        hashSectionList.push_back({curHashOffset, curSectionSize});

        curOffset += curSectionSize;
    }

    // add current list to map
    auto filenameStartPos = basePath.find_last_of("/\\") + 1;
    auto filenameEndPos = basePath.find(SAVE_EXT);
    std::string filename = basePath.substr(filenameStartPos, filenameEndPos - filenameStartPos);
    FILENAME_HASHSECTIONS_MAP[filename] = hashSectionList;
    std::cout << "searched " << filename << std::endl << std::endl;
}

void printMap() {
    for (auto mapEntry : FILENAME_HASHSECTIONS_MAP) {
        std::cout << std::hex << "    {" << std::endl;
        std::cout << "        \"" << mapEntry.first << "\", {" << std::endl;
        for (auto listEntry : mapEntry.second) {
            std::cout << "            {0x" << listEntry.hashOffset << ", 0x" << listEntry.sectionLen << "},"
                      << std::endl;
        }
        std::cout << "        }" << std::endl;
        std::cout << "    }," << std::endl;
    }
}

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [path to *.dat.dec]" << std::endl;
        return -1;
    }

    const char** paths = &argv[1];
    int paths_count = argc - 1;
    try {
        for (int i = 0; i < paths_count; i++) {
            processPath(paths[i]);
        }
        printMap();
    } catch (std::range_error& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
