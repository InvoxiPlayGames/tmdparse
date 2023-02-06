# tmdparse

## + tikdecrypt + nus_download.sh + bindecrypt

This is a collection of command line utilities to parse Wii TMD files and decrypt title contents.

tmdparse is a program that will parse a TMD file and print out information (title ID, contents).

tikdecrypt is a program that will decrypt a Wii content file using a ticket file and the associated content index.

bindecrypt is a program that will decrypt a Wii SD card DLC backup file, provided the console-specific keys.

nus_download.sh is a bash script that will download and decrypt a system title from NUS using the above 2 tools.

## Building

```
cc tmdparse.c -o tmdparse
cc tikdecrypt.c crypto/aes.c -o tikdecrypt
```

(This is required before using nus_download.sh)

## License

These tools are licensed under the MIT license.

This project uses [tiny-AES-c](https://github.com/kokke/tiny-AES-c), under public domain.