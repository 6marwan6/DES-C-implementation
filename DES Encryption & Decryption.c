#include <stdint.h>
#include <stddef.h>

/*
 * DES core (block encrypt/decrypt).
 *
 * Provides:
 *   - DES(): encrypt/decrypt a 64-bit block using 16 round subkeys
 *   - processData(): build subkeys and run DES over all blocks
 *
 * Expects :
 *   - F_function() (ahmed emad), keyGeneration() (hani)
 *   - permutation tables: initial_permutation[], final_permutation[]
 *   - globals: key, data_blocks, num_blocks, is_encrypt_mode
 */

uint64_t DES(uint64_t block, const uint64_t *keys) {
    /* Initial permutation */
    uint64_t block_permuted = permute(block, initial_permutation, 64, 64);

    uint32_t L = (uint32_t)(block_permuted >> 32);
    uint32_t R = (uint32_t)(block_permuted & 0xFFFFFFFF);

    /* 16 Feistel rounds */
    for (int i = 0; i < 16; ++i) {
        uint32_t oldR = R;
        R = L ^ F_function(R, keys[i]);
        L = oldR;
    }

    /* Swap halves */
    block_permuted = ((uint64_t)R << 32) | (uint64_t)L;

    /* Final permutation */
    uint64_t cipher = permute(block_permuted, final_permutation, 64, 64);

    return cipher;
}

/* processData: build subkeys and apply DES to every 64-bit block.
 * If decrypting, use subkeys in reverse order .
 */
void processData(void) {
    uint64_t keys[16];

    /* Generate round subkeys */
    keyGeneration(keys);

    /* Reverse subkeys for decryption */
    if (!is_encrypt_mode) {
        for (int i = 0; i < 8; i++) {
            uint64_t temp = keys[i];
            keys[i] = keys[15 - i];
            keys[15 - i] = temp;
        }
    }

    /* Process blocks in-place */
    for (size_t i = 0; i < num_blocks; ++i) {
        data_blocks[i] = DES(data_blocks[i], keys);
    }
}
