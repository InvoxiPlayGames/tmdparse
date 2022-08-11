#include <stdio.h>
#include <stdlib.h>
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

int GetCertTypeSize(int type) {
    type &= TMD_CERT_TYPEMASK;
    if (type == TMD_CERT_RSA_4096)
        return TMD_RSA_4096_SIZE;
    if (type == TMD_CERT_RSA_2048)
        return TMD_RSA_2048_SIZE;
    if (type == TMD_CERT_ECC_B233)
        return TMD_ECC_B233_SIZE;
    return -1;
}

int GetKeyTypeSize(int type) {
    type &= TMD_CERT_TYPEMASK;
    if (type == TMD_CERT_RSA_4096)
        return sizeof(TMDPublicKeyRSA4096);
    if (type == TMD_CERT_RSA_2048)
        return sizeof(TMDPublicKeyRSA2048);
    if (type == TMD_CERT_ECC_B233)
        return sizeof(TMDPublicKeyECCB233);
    return -1;
}

char *GetCertTypeString(int type) {
    type &= TMD_CERT_TYPEMASK;
    if (type == TMD_CERT_RSA_4096)
        return "RSA-4096";
    if (type == TMD_CERT_RSA_2048)
        return "RSA-2048";
    if (type == TMD_CERT_ECC_B233)
        return "ECC-B233";
    return "Unknown";
}

char *GetRegionString(short region) {
    if (region == 0)
        return "Japan";
    if (region == 1)
        return "America";
    if (region == 2)
        return "Europe";
    if (region == 3)
        return "Region Free";
    if (region == 4)
        return "Korea";
    return "Unknown";
}

void PrintTMDInfo(int sig_type, TMDHeader header) {
    printf("Title: %016llx (%.4s)\n", BE64(header.title_id.id), header.title_id.name.code);
    printf("IOS: %llu\n", BE64(header.ios_version) & 0xFF);
    printf("Version: %i\n", BE16(header.title_version));
    printf("Region: %s\n", GetRegionString(BE16(header.region)));
    if (BE(header.access_rights) & 1)
        printf("Full AHBPROT Access\n");
    if (BE(header.access_rights) & 2)
        printf("Full DVD Access\n");
    printf("Content Count: %i\n", BE16(header.num_contents));
    printf("Boot Content: %i\n", BE16(header.boot_index));
    printf("Signature Issuer: %s (%s)\n", header.issuer, GetCertTypeString(sig_type));
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

void PrintCertificateInfo(int sig_type, TMDCertificateData cert) {
    printf("   Issuer: %s\n", cert.issuer);
    printf("   Name: %s\n", cert.name);
    printf("   Signature Type: %s\n", GetCertTypeString(sig_type));
    printf("   Key Type: %s\n", GetCertTypeString(BE(cert.key_type)));
}

int main(int argc, char **argv) {
    // print usage info if we don't have the right number of args
    if (argc < 2) {
        printf("usage: %s /path/to/title.tmd\n", argv[0]);
        return -1;
    }
    // open the tmd file
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("failed to open tmd\n");
        return -1;
    }
    // read the signature type from the file
    int signature_type;
    int signature_size;
    char signature[TMD_RSA_4096_SIZE];
    fread(&signature_type, sizeof(signature_type), 1, fp);
    signature_size = GetCertTypeSize(BE(signature_type));
    if (signature_size < 0) {
        printf("invalid TMD signature type: %08x - can't continue\n", BE(signature_type));
        return -1;
    }
    // read the signature based on the signature type
    fread(signature, signature_size, 1, fp);
    // read the TMD header and print the information from it
    TMDHeader tmd;
    fread(&tmd, sizeof(TMDHeader), 1, fp);
    PrintTMDInfo(BE(signature_type), tmd);
    // read each individual content from the file and print the information
    for (int i = 0; i < BE16(tmd.num_contents); i++) {
        TMDContent cnt;
        fread(&cnt, sizeof(TMDContent), 1, fp);
        printf("Content %i:\n", i);
        PrintContentInfo(cnt);
    }
    // read each certificate in the chain and print information
    int cert_type = 0;
    int cert_count = 1;
    while (fread(&cert_type, 1, sizeof(int), fp) == sizeof(int)) {
        printf("Certificate %i:\n", cert_count);
        // read the signature
        signature_size = GetCertTypeSize(BE(cert_type));
        if (signature_size < 0) {
            printf("Unknown cert type: %08x - can't continue\n", BE(cert_type));
            break;
        }
        fread(signature, signature_size, 1, fp);
        // read the certificate information
        TMDCertificateData info;
        fread(&info, sizeof(TMDCertificateData), 1, fp);
        PrintCertificateInfo(BE(cert_type), info);
        // read the public key
        int key_size = GetKeyTypeSize(BE(info.key_type));
        if (key_size < 0) {
            printf("Unknown key type: %08x - can't continue\n", BE(info.key_type));
            break;
        }
        // we don't do anything with the key yet, use regular buffer
        void *blah = malloc(key_size);
        fread(blah, key_size, 1, fp);
        free(blah);
        cert_count++;
    }
    return 0;
}