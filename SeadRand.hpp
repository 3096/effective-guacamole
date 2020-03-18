#pragma once

#include <stdint.h>
#include <array>
#include <cstring>

class SeadRand {
   private:
    std::array<uint32_t, 4> mState;

   public:
    SeadRand(uint32_t seed);
    SeadRand(std::array<uint32_t, 4> state);

    uint32_t getU32();
    uint64_t getU64();
    std::array<uint32_t, 4> getContext();
};
