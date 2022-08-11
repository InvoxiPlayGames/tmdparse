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
    printf("Signer: %s (%s)\n", header.issuer, BE(header.signature_type) == 0x10001 ? "RSA-2048" : "RSA-4096");
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

void PrintCertificateInfo2048(TMDCertificate2048 cert) {
    printf("   Issuer: %s\n", cert.issuer);
    printf("   Name: %s\n", cert.name);
    printf("   Type: RSA-2048\n");
    printf("   Tag: %08x\n", BE(cert.tag));
}

void PrintCertificateInfo4096(TMDCertificate4096 cert) {
    printf("   Issuer: %s\n", cert.issuer);
    printf("   Name: %s\n", cert.name);
    printf("   Type: RSA-4096\n");
    printf("   Tag: %08x\n", BE(cert.tag));
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
    int cert_type = 0;
    int cert_count = 1;
    while (fread(&cert_type, 1, sizeof(int), fp) == sizeof(int)) {
        printf("Certificate %i:\n", cert_count);
        if (BE(cert_type) == 0x10000) {
            TMDCertificate4096 cert4096;
            fread(&cert4096, sizeof(TMDCertificate4096), 1, fp);
            PrintCertificateInfo4096(cert4096);
        } else if (BE(cert_type) == 0x10001) {
            TMDCertificate2048 cert2048;
            fread(&cert2048, sizeof(TMDCertificate2048), 1, fp);
            PrintCertificateInfo2048(cert2048);
        } else {
            printf("Unknown cert type: %08x - can't continue\n", BE(cert_type));
            break;
        }
        cert_count++;
    }
    return 0;
}