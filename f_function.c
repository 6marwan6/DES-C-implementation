#include <stdint.h>

// Example tables (you must include real DES tables if needed)
extern const int expansion[48];
extern const int transposition_permutation[32];
extern const int s_box[8][4][16];

// Generic permute() function (should exist in your project)
extern uint64_t permute(uint64_t input, const int *table, int size, int out_bits);

// ------------------ F FUNCTION ------------------
uint32_t f(uint32_t R, uint64_t K) {
    // 1. Expand 32-bit R to 48 bits
    uint64_t expanded = permute((uint64_t)R << 32, expansion, 64, 48);

    // 2. XOR with 48-bit subkey
    uint64_t xored = expanded ^ K;

    // 3. Apply S-boxes
    uint32_t sbox_output = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t six_bits = (xored >> (42 - 6*i)) & 0x3F;
        int row = ((six_bits & 0x20) >> 4) | (six_bits & 1);
        int col = (six_bits >> 1) & 0xF;
        sbox_output = (sbox_output << 4) | s_box[i][row][col];
    }

    // 4. Apply permutation P
    uint32_t result = (uint32_t)permute((uint64_t)sbox_output << 32, transposition_permutation, 64, 32);
    return result;
}