#include "precompiled.hpp"
#include "utilities/bitMap.inline.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"
#include "unittest.hpp"
#include <stdlib.h>
#include <chrono>

// Tests that iteratively calling find_first_set_bit and getting
// an array of indices from find_first_n_set_bits are the same.
TEST(BitMap, SIMDIndex){
    size_t N = 10000; // bytes of bitMap
    size_t startOffset = 123;
    size_t nBits = N * 64;
    CHeapBitMap bm(nBits, mtTest); // extra room
    size_t iterative[nBits]; // results from iterative search
    uint32_t output[nBits]; // relative results from SIMD search

    EXPECT_TRUE(bm.is_empty());

    double ratio = 0.10;
    size_t expected = (size_t)(N * ratio * 64);
    size_t count = 0;
    while (count < expected) {
        int bit = rand() % (nBits);
        if (!bm.at(bit)) {
            bm.set_bit(bit);
            count++;
        }
    }
    size_t bitCount = bm.count_one_bits(startOffset, nBits);
    size_t found = startOffset;
    size_t iterativeIndex = 0;
    // time this
    auto start = std::chrono::high_resolution_clock::now();
    while((found = bm.find_first_set_bit(found, nBits)) != nBits){
        iterative[iterativeIndex++] = found;
        found++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Iterative time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    uint res = (uint) bm.find_first_n_set_bits(startOffset, nBits, output, nBits);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "SIMD time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us" << std::endl;
    EXPECT_EQ(res, bitCount);
    // compare arrays
    for (size_t i = 0; i < bitCount; i++) {
        EXPECT_EQ(iterative[i], startOffset + output[i]);
    }
}
