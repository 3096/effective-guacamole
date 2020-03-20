# effective-guacamole
PoC save decryption and re-signing for Animal Crossing: New Horizon. Last updated for 1.1.0

This PoC includes 2 programs:
- save_cryptor
  - decrypts and re-encrypts+signs given save files with a single command
  - example usage (ensure "Header" files are in the same directory):
    -  `save_cryptor main.dat` - decrypts main.dat
    -  `save_cryptor main.dat.dec` - encrypts main.dat.dec
    -  `save_cryptor Villager0/*.dat` - decrypts all .dat in Villager0 folder
- generate_hash_sections
  - automatically generates the hash sections map for use with save_cryptor (prints to stdout)
  - example usage:
    - `save_cryptor all_saves_in_this_dir/*.dat && generate_hash_sections all_saves_in_this_dir/*.dat.dec`
  - sample output for 1.1.0:
```
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
```

## Build Instructions

Test executable for Windows can be found in [releases](https://github.com/3096/effective-guacamole/releases)

To build:

- generate_hash_sections
  - `g++ *.cpp -o generate_hash_sections`

- save_cryptor
  - `g++ *.cpp smhasher/src/MurmurHash3.cpp -lmbedcrypto -o save_cryptor`
  - make sure you have libmbedcrypto installed from [mbedtls](https://github.com/ARMmbed/mbedtls)

## Credits
- [@shadowninja108](https://github.com/shadowninja108/Blanket) for key generation
- [@Cuyler36](https://github.com/Cuyler36/HorizonSummer) for save hashing
