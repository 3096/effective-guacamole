#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>

#ifdef __SWITCH__
#    include <switch.h>
#else
#    include <mbedtls/aes.h>
#endif

#include "SeadRand.hpp"
#include "smhasher/src/MurmurHash3.h"

std::array<uint8_t, 0x10> getKeyOrIV(std::unique_ptr<uint32_t[]>& headerCryptSectionBuffer, uint64_t sourceIdx) {
    // look up prng seed and advance it by some amount
    SeadRand rng(headerCryptSectionBuffer[headerCryptSectionBuffer[sourceIdx] & 0x7F]);
    uint32_t rngAdvanceCount = (headerCryptSectionBuffer[headerCryptSectionBuffer[sourceIdx + 1] & 0x7F] & 0xF) + 1;
    for (int i = 0; i < rngAdvanceCount; i++) {
        rng.getU64();
    }

    // generate key
    std::array<uint8_t, 0x10> result;
    for (int i = 0; i < 0x10; i++) {
        result[i] = (uint8_t)(rng.getU32() >> 24);
    }
    return result;
}

void aes_crypt_ctr(uint8_t* in_buf, std::array<uint8_t, 0x10> key, std::array<uint8_t, 0x10> iv, size_t len,
                   uint8_t* out_buf) {
#ifdef __SWITCH__
#    warning "I have not tested this, please test to make sure this works"
    Aes128CtrContext ctx;
    aes128CtrContextCreate(&ctx, key.data(), iv.data());
    aes128CtrCrypt(&ctx, out_buf, in_buf, len);
#else
    size_t count = 0;
    std::array<uint8_t, 0x10> sb = {0};
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key.data(), 128);
    mbedtls_aes_crypt_ctr(&ctx, len, &count, iv.data(), sb.data(), in_buf, out_buf);
    mbedtls_aes_free(&ctx);
#endif
}

struct MurmurHashSection {
    size_t hashOffset;
    size_t sectionLen;
};

// clang-format off
const std::unordered_map<std::string, std::list<MurmurHashSection>> FILENAME_HASHSECTIONS_MAP_V110 = {
    // map content automatically generated with: 
    // https://github.com/3096/effective-guacamole/blob/master/generate_hash_sections/main.cpp
    {
        "postbox", {
            {0x100, 0xb4448c},
        }
    },
    {
        "main", {
            {0x110, 0x1d6d5c},
            {0x1d6e70, 0x323c0c},
            {0x4fab90, 0x35afc},
            {0x530690, 0x362bc},
            {0x566a60, 0x35afc},
            {0x59c560, 0x362bc},
            {0x5d2930, 0x35afc},
            {0x608430, 0x362bc},
            {0x63e800, 0x35afc},
            {0x674300, 0x362bc},
            {0x6aa6d0, 0x35afc},
            {0x6e01d0, 0x362bc},
            {0x7165a0, 0x35afc},
            {0x74c0a0, 0x362bc},
            {0x782470, 0x35afc},
            {0x7b7f70, 0x362bc},
            {0x7ee340, 0x35afc},
            {0x823e40, 0x362bc},
            {0x85a100, 0x26899c},
        }
    },
    {
        "profile", {
            {0x100, 0x6945c},
        }
    },
    {
        "photo_studio_island", {
            {0x100, 0x262bc},
        }
    },
    {
        "personal", {
            {0x110, 0x35afc},
            {0x35c10, 0x362bc},
        }
    },
};
// clang-format on

void processPath(std::string basePath) {
    const std::string SAVE_EXT = ".dat";
    const std::string DECRYPTED_EXT = ".dec";
    const std::string ENCRYPTED_EXT = ".enc";
    const std::string HEADER_SUFFIX = "Header";
    constexpr long HEADER_INFO_SECTION_SIZE = 0x100;

    // don't crypt header files
    if (basePath.find(HEADER_SUFFIX) != std::string::npos) return;

    // read base file into buffer
    std::ifstream baseFileIS(basePath, std::ios::binary | std::ios::ate);
    if (not baseFileIS.good()) {
        std::cout << "Could not open file " << basePath << std::endl;
        return;
    }
    long baseFileSize = baseFileIS.tellg();
    auto baseFileBuffer = std::make_unique<uint8_t[]>(baseFileSize);
    baseFileIS.seekg(0, std::ios::beg);
    baseFileIS.read(reinterpret_cast<char*>(baseFileBuffer.get()), baseFileSize);

    // header should be "[base]Header.dat"
    std::string headerPath = basePath.substr(0, basePath.rfind(SAVE_EXT)) + HEADER_SUFFIX + SAVE_EXT;

    // get header crypt section size and create buffer
    std::ifstream headerFileIS(headerPath, std::ios::binary | std::ios::ate);
    if (not headerFileIS.good()) {
        std::cout << "Could not open header " << headerPath << std::endl;
        return;
    }
    long headerCryptSectionSize = headerFileIS.tellg() - HEADER_INFO_SECTION_SIZE;
    auto headerCryptSectionBuffer = std::make_unique<uint32_t[]>(headerCryptSectionSize / sizeof(uint32_t));

    // read header crypt section
    headerFileIS.seekg(HEADER_INFO_SECTION_SIZE, std::ios::beg);  // crypt section is right after info section
    headerFileIS.read(reinterpret_cast<char*>(headerCryptSectionBuffer.get()), headerCryptSectionSize);

    // generate keys and iv
    std::array<uint8_t, 0x10> key = getKeyOrIV(headerCryptSectionBuffer, 0);
    std::array<uint8_t, 0x10> iv = getKeyOrIV(headerCryptSectionBuffer, 2);

    // check if base file is decrypted (simply using filename for this PoC)
    std::string outPath;
    size_t rfindDecExtPos = basePath.rfind(DECRYPTED_EXT);
    if (rfindDecExtPos != std::string::npos and basePath.substr(rfindDecExtPos) == DECRYPTED_EXT) {
        // base file is decrypted, need to fix hash before encrypting
        outPath = basePath + ENCRYPTED_EXT;

        // get filename for hash sections lookup
        auto filenameStartPos = basePath.find_last_of("/\\") + 1;
        auto filenameEndPos = basePath.find(SAVE_EXT);
        std::string filename = basePath.substr(filenameStartPos, filenameEndPos - filenameStartPos);

        // go through hash sections list and calculate each one
        auto filenameHashsectionsMap = FILENAME_HASHSECTIONS_MAP_V110;  // TODO: detect version using header
        for (MurmurHashSection hashSection : filenameHashsectionsMap[filename]) {
            uint32_t calcedHash;
            MurmurHash3_x86_32(&baseFileBuffer[hashSection.hashOffset + 4], hashSection.sectionLen, 0, &calcedHash);
            *reinterpret_cast<uint32_t*>(&baseFileBuffer[hashSection.hashOffset]) = calcedHash;
        }
    } else {
        // base file is encrypted, only decrypt is needed
        outPath = basePath + DECRYPTED_EXT;
    }

    // crypt
    aes_crypt_ctr(baseFileBuffer.get(), key, iv, (size_t)baseFileSize, baseFileBuffer.get());

    // output result
    std::ofstream fileos(outPath, std::fstream::binary);
    fileos.write(reinterpret_cast<char*>(baseFileBuffer.get()), baseFileSize);
    fileos.close();
    std::cout << outPath << " crypt successful" << std::endl;
}

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [path to *.dat]" << std::endl;
        std::cout << "   or  " << argv[0] << " [path to *.dec]" << std::endl;
        std::cout << "   or  " << argv[0] << " [path to *.enc]" << std::endl << std::endl;
        std::cout << "Example: " << argv[0] << " Villager0/*.dat" << std::endl << std::endl;
        return -1;
    }

    const char** paths = &argv[1];
    int paths_count = argc - 1;
    for (int i = 0; i < paths_count; i++) {
        processPath(paths[i]);
    }

    return 0;
}
