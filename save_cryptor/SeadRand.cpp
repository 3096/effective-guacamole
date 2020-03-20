#include "SeadRand.hpp"

SeadRand::SeadRand(uint32_t seed = 0) {
    for (int i = 0; i < 4; i++) {
        mState[i] = (uint32_t)(0x6C078965 * (seed ^ (seed >> 30)) + i + 1);
        seed = mState[i];
    }
}

SeadRand::SeadRand(std::array<uint32_t, 4> state) {
    mState = state;
}

uint32_t SeadRand::getU32() {
    uint32_t a = mState[0] ^ (mState[0] << 11);
    uint32_t b = mState[3];
    uint32_t c = a ^ (a >> 8) ^ b ^ (b >> 19);
    for (int i = 0; i < 3; i++)
        mState[i] = mState[i + 1];
    mState[3] = c;
    return c;
}

uint64_t SeadRand::getU64() {
    uint32_t a = mState[1];
    uint32_t b = mState[0] ^ (mState[0] << 11);
    uint32_t c = mState[3];
    mState[0] = mState[2];
    mState[1] = c;
    uint32_t d = b ^ (b >> 8) ^ c;
    uint32_t e = d ^ (c >> 19);
    uint32_t f = a ^ (a << 11) ^ ((a ^ (a << 11)) >> 8) ^ e ^ (d >> 19);
    mState[2] = e;
    mState[3] = f;
    return f | ((uint64_t)e << 32);
}

std::array<uint32_t, 4> SeadRand::getContext() {
    return mState;
}
