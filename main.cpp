#include <mbedtls/aes.h>
#include <fstream>
#include <iostream>
#include <memory>

#include "SeadRand.hpp"

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

void decrypt(std::string basePath) {
    const std::string FILE_FORMAT = ".dat";
    const std::string HEADER_SUFFIX = "Header";
    const std::string DECRYPTED_SUFFIX = ".dec";
    constexpr long HEADER_CRYPT_SECTION_OFFSET = 0x100;

    // don't decript header files
    if (basePath.find(HEADER_SUFFIX) != std::string::npos) return;

    // header should be "[base]Header.dat"
    std::string headerPath = basePath.substr(0, basePath.rfind(FILE_FORMAT)) + HEADER_SUFFIX + FILE_FORMAT;

    // get header crypt section size and create buffer
    std::ifstream headerFileIS(headerPath, std::ios::binary | std::ios::ate);
    long headerCryptSectionSize = headerFileIS.tellg() - HEADER_CRYPT_SECTION_OFFSET;
    auto headerCryptSectionBuffer = std::make_unique<uint32_t[]>(headerCryptSectionSize / sizeof(uint32_t));

    // read header crypt section
    headerFileIS.seekg(HEADER_CRYPT_SECTION_OFFSET, std::ios::beg);
    headerFileIS.read(reinterpret_cast<char*>(headerCryptSectionBuffer.get()), headerCryptSectionSize);

    // generate keys and iv
    std::array<uint8_t, 0x10> key = getKeyOrIV(headerCryptSectionBuffer, 0);
    std::array<uint8_t, 0x10> iv = getKeyOrIV(headerCryptSectionBuffer, 2);

    // read base file into buffer
    std::ifstream baseFileIS(basePath, std::ios::binary | std::ios::ate);
    long baseFileSize = baseFileIS.tellg();
    auto baseFileBuffer = std::make_unique<uint8_t[]>(baseFileSize);
    baseFileIS.seekg(0, std::ios::beg);
    baseFileIS.read(reinterpret_cast<char*>(baseFileBuffer.get()), baseFileSize);

    // decrypt
    size_t count = 0;
    std::array<uint8_t, 0x10> sb = {0};
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key.data(), 128);
    mbedtls_aes_crypt_ctr(&ctx, (size_t)baseFileSize, &count, iv.data(), sb.data(), baseFileBuffer.get(),
                          baseFileBuffer.get());
    mbedtls_aes_free(&ctx);

    // output result
    std::ofstream fileos(basePath + DECRYPTED_SUFFIX, std::fstream::binary);
    fileos.write(reinterpret_cast<char*>(baseFileBuffer.get()), baseFileSize);
    fileos.close();
}

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv << " [encrypted save file path]" << std::endl;
        return 0;
    }

    const char** paths = &argv[1];
    int paths_count = argc - 1;
    for (int i = 0; i < paths_count; i++) {
        decrypt(paths[i]);
    }

    return 0;
}
