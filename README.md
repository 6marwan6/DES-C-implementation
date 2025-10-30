# DES-C-implementation

A complete implementation of the DES (Data Encryption Standard) algorithm in C, supporting both encryption and decryption of binary files.

## Features

- Full DES encryption and decryption
- Proper endianness handling for cross-platform compatibility
- Command-line interface for easy file processing
- Support for any file size (multiples of 8 bytes)

## Compilation

Compile the program using GCC with C99 standard:

```powershell
gcc -o main.exe main.c -std=c99 
```

On Linux/Mac:
```bash
gcc -o main main.c -std=c99 
```

## Usage

```
main.exe <e|d> <keyfile> <inputfile> <outputfile>
```

### Arguments

- `<e|d>`: Mode - `e` for encryption, `d` for decryption
- `<keyfile>`: Path to the 8-byte binary key file
- `<inputfile>`: Path to the input file to encrypt/decrypt
- `<outputfile>`: Path where the output will be written

### Examples

**Encrypt a file:**
```powershell
.\main.exe e files\key.bin files\input.bin encrypted.bin
```

**Decrypt a file:**
```powershell
.\main.exe d files\key.bin encrypted.bin decrypted.bin
```

## Input/Output Format

### Key File
- **Format**: Binary file
- **Size**: Exactly 8 bytes (64 bits)
- **Content**: Raw binary key data
- **Example**: A file containing 8 bytes like `DA CD A8 92 3C A7 32 61`

### Input File
- **Format**: Binary file
- **Size**: Must be a multiple of 8 bytes (DES block size)
- **Content**: Any binary data 

### Output File
- **Format**: Binary file
- **Size**: Same as input file
- **Content**: Encrypted or decrypted binary data
- **Endianness**: Big-endian format 


## Technical Details

### Algorithm
- **Block size**: 64 bits (8 bytes)
- **Key size**: 64 bits (56 bits + 8 parity bits)
- **Rounds**: 16 Feistel rounds

### Implementation
- Written in C99
- Uses standard DES permutation tables (IP, FP, E, P, PC1, PC2)
- Implements all 8 S-boxes
- Automatic endianness handling for little-endian systems (Windows, Linux x86/x64)
- Key generation with proper left rotations per round

### Files
- `main.c` - Main implementation with DES core, key generation, and file I/O
- `files/` - Test data directory


