#!/bin/bash
cc tmdparse.c -o tmdparse
cc tikdecrypt.c crypto/aes.c crypto/sha1.c -o tikdecrypt
cc bindecrypt.c crypto/aes.c crypto/sha1.c -o bindecrypt
