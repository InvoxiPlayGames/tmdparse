#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "tmd.h"

#ifndef SHOULD_BE_BE
#define BE16(i) ((((i) & 0xFF) << 8 | ((i) >> 8) & 0xFF) & 0xFFFF)
#define BE(i)   (((i) & 0xff) << 24 | ((i) & 0xff00) << 8 | ((i) & 0xff0000) >> 8 | ((i) >> 24) & 0xff)
#define BE64(i) (BE((i) & 0xFFFFFFFF) << 32 | BE(((i) >> 32) & 0xFFFFFFFF))
#else
#define BE16(i) i
#define BE(i) i
#define BE64(i) i
#endif

void PrintTMDInfo(TMDHeader header) {
    printf("Title: %016llx\n", BE64(header.title_id));
    printf("IOS: %llu\n", BE64(header.ios_version) & 0xFF);
    printf("Version: %i\n", BE16(header.title_version));
    printf("Content Count: %i\n", BE16(header.num_contents));
    printf("Boot Content: %i\n", BE16(header.boot_index));
}

void PrintContentInfo(TMDContent content) {
    printf("   Index: %i\n", BE16(content.index));
    printf("   ID: %08x\n", BE(content.id));
    printf("   Type: %04x\n", BE16(content.type));
    printf("   Size: %llu\n", BE64(content.size));
    printf("   SHA1: ");
    for (int i = 0; i < 0x14; i++) {
        printf("%02x", content.SHA1[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s /path/to/title.tmd\n", argv[0]);
        return -1;
    }
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("failed to open tmd\n");
        return -1;
    }
    TMDHeader tmd;
    fread(&tmd, sizeof(TMDHeader), 1, fp);
    PrintTMDInfo(tmd);
    for (int i = 0; i < BE16(tmd.num_contents); i++) {
        TMDContent cnt;
        fread(&cnt, sizeof(TMDContent), 1, fp);
        printf("Content %i:\n", i);
        PrintContentInfo(cnt);
    }
    return 0;
}