#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BLOCK_SIZE 8

// === DES TABLES ===
//

bool is_encrypt_mode = false;
uint64_t *data_blocks = NULL;
size_t num_blocks = 0;

// Initial Permutation Table
static const int initial_permutation[64] = {
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

// Final Permutation Table
static const int final_permutation[64] = {
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9,  49, 17, 57, 25};

// Expansion Table
static const int expansion[48] = {
    32, 1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,  8,  9,  10, 11,
    12, 13, 12, 13, 14, 15, 16, 17, 16, 17, 18, 19, 20, 21, 20, 21,
    22, 23, 24, 25, 24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1};

// Permutation Function (P)
static const int transposition_permutation[32] = {
    16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
    2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25};

// Permuted Choice 1 (PC1)
static const int permuted_choice_1[56] = {
    57, 49, 41, 33, 25, 17, 9,  1,  58, 50, 42, 34, 26, 18, 10, 2,  59, 51, 43,
    35, 27, 19, 11, 3,  60, 52, 44, 36, 63, 55, 47, 39, 31, 23, 15, 7,  62, 54,
    46, 38, 30, 22, 14, 6,  61, 53, 45, 37, 29, 21, 13, 5,  28, 20, 12, 4};

// Permuted Choice 2 (PC2)
static const int permuted_choice_2[48] = {
    14, 17, 11, 24, 1,  5,  3,  28, 15, 6,  21, 10, 23, 19, 12, 4,
    26, 8,  16, 7,  27, 20, 13, 2,  41, 52, 31, 37, 47, 55, 30, 40,
    51, 45, 33, 48, 44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32};

// Left Shifts per Round
static const int shift_left[16] = {1, 1, 2, 2, 2, 2, 2, 2,
                                   1, 2, 2, 2, 2, 2, 2, 1};

// S-boxes
static const int s_box[8][4][16] = {
    {{14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
     {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
     {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
     {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}},
    {{15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
     {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
     {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
     {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}},
    {{10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
     {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
     {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
     {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}},
    {{7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
     {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
     {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
     {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}},
    {{2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
     {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
     {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
     {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}},
    {{12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
     {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
     {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
     {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}},
    {{4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
     {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
     {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
     {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}},
    {{13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
     {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
     {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
     {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}}};

uint64_t permute(uint64_t input, const int *table, int size, int out_bits) {

  /// All these comments can be removed later, just for clarity while using
  /// 1) This Function performs permutation based on a given table
  /// 2) usage example :   uint64_t result = permute(input, permuted_choice_2,
  /// 56, 48); 3) input is the input bits to be permuted 4) table is the
  /// permutation table : ex : initial_permutation 5) size is the size of input
  /// in bits ( ex : 64 ) 6) out_bits is the size of output in bits ( ex : 64
  /// for initial_permutation ) 7) Returns the permuted output as uint64_t

  uint64_t output = 0;
  for (int i = 0; i < out_bits; i++) {

    /// src_pos is the position that the permutation table points to
    int src_pos = table[i] - 1;

    /// start filling the last (least significant) bit of output then shift left
    /// to make room for the next bit to be added in the least significant bit
    /// again
    output <<= 1;

    /// input >> (size - src_pos - 1)   to shift the bit we want to permute to
    /// the least significant bit position & 1 -> isolates only that bit ORing
    /// that bit with the output to put it in the least significant bit position
    /// of output
    output |= (input >> (size - src_pos - 1)) & 1;
  }
  return output;
}

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

void des_encrypt(const char *input, const char *key, char *output) {
  // DES encryption implementation
}

void des_decrypt(const char *input, const char *key, char *output) {
  // DES decryption implementation
}

int main(int argc, char *argv[]) {
  // DES algorithm implementation
  //
  if (argc != 5) {
    printf("Usage: %s <e|d> <key> <file1> <file2>\n", argv[0]);
    return 1;
  }

  const char *mode = argv[1];
  FILE *keyFile = fopen(argv[2], "rb");
  FILE *file1 = fopen(argv[3], "rb");
  FILE *file2 = fopen(argv[4], "wb");

  if (!keyFile || !file2 || !file1) {
    perror("Cannot open file");
    return 1;
  }

  char key[BLOCK_SIZE];
  if (fread(key, 1, BLOCK_SIZE, keyFile) != BLOCK_SIZE) {
    fprintf(stderr, "Error: key file must be %d bytes\n", BLOCK_SIZE);
    return 1;
  }
  struct stat st;

  stat(argv[3], &st);

  num_blocks = st.st_size / BLOCK_SIZE;

  data_blocks = (uint64_t *)malloc(num_blocks * BLOCK_SIZE);

  fread(data_blocks, BLOCK_SIZE, num_blocks, file1);
  fclose(file1);

  is_encrypt_mode = (mode[0] == 'e');

  processData();

  fwrite(data_blocks, BLOCK_SIZE, num_blocks, file2);

  fclose(keyFile);
  fclose(file1);
  fclose(file2);

  return 0;
}
