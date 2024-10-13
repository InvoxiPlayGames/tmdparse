# tmdparse

## + tikdecrypt + nus_download.sh + bindecrypt

This is a collection of command line utilities to parse Wii TMD files and decrypt title contents.

tmdparse is a program that will parse a TMD file and print out information (title ID, contents).

tikdecrypt is a program that will decrypt a Wii content file using a ticket file and the associated content index.

nus_download.sh is a bash script that will download and decrypt a title from NUS using the above 2 tools.

bindecrypt is a program that will decrypt a Wii SD card DLC backup file, provided the console-specific keys.

## Building

Building requires a GCC or Clang compiler.

* On Windows, use MSYS2 MINGW64. **Visual Studio / MSVC is not supported.**
* On macOS, install the Xcode Command Line Tools.
* On Linux / WSL, install the development package for your distro.
    * Debian / Ubuntu: `build-essential`

```
cc tmdparse.c -o tmdparse
cc tikdecrypt.c crypto/aes.c crypto/sha1.c -o tikdecrypt
cc bindecrypt.c crypto/aes.c crypto/sha1.c -o bindecrypt
```

...or `build_all.sh`.

(This is required before using nus_download.sh)

## License

These tools are licensed under the MIT license.

This project uses [tiny-AES-c](https://github.com/kokke/tiny-AES-c), under public domain.

This project uses Steve Reid's SHA-1 in C implementation, under public domain.
