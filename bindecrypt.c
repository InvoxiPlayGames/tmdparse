#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tmd.h"
#include "backupwad.h"
#include "bootmii.h"
#define CBC 1
#define ECB 0
#define CTR 0
#include "crypto/aes.h"

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

void PrintBackupInfo(BackupWadHeader header) {
    printf("Title: %016llx (%.4s)\n", BE64(header.title_id.id), header.title_id.name.code);
    printf("Console ID: %08x\n", BE(header.console_id));
}

void DecryptContent(unsigned char *title_key, unsigned short content_index, unsigned char *data, int length) {
    struct AES_ctx ctx;
    unsigned char iv[0x10] = { 0 };
    // set the IV to the content index in big endian
    unsigned short idx = BE16(content_index);
    memcpy(iv, &idx, sizeof(idx));
    // initialise our aes context using the title key we derived earlier
    AES_init_ctx(&ctx, title_key);
    AES_ctx_set_iv(&ctx, iv);
    // decrypt the buffer
    AES_CBC_decrypt_buffer(&ctx, data, length);
}

int RoundUp(int source_num, int multiple) {
    if (multiple == 0)
        return source_num;
    int remainder = source_num % multiple;
    if (remainder == 0)
        return source_num;
    return source_num + multiple - remainder;
}

int main(int argc, char **argv) {
    // print usage info if we don't have the right number of args
    if (argc < 3) {
        printf("usage: %s /path/to/dlc.bin /path/to/keys.bin\n", argv[0]);
        return -1;
    }
    // open the bin file
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("failed to open bin\n");
        return -1;
    }
    // read the backup header
    BackupWadHeader header;
    fread(&header, sizeof(BackupWadHeader), 1, fp);
    PrintBackupInfo(header);
    int index = 0;
    bool found_index = false;
    for (int i = 0; i < 0x40; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char mask = (1 << j);
            found_index = (header.included_contents[i] & mask) > 0;
            if (found_index) break;
            index++;
        }
        if (found_index) break;
    }
    if (!found_index) {
        printf("failed to get content index\n");
        return -1;
    }
    printf("Content Index: %i\n", index);

    // read the TMD and parse it
    void *tmd_buffer = malloc(BE(header.content_tmd_size));
    fread(tmd_buffer, BE(header.content_tmd_size), 1, fp);
    int signature_type = BE(*(int *)tmd_buffer);
    int signature_size = GetCertTypeSize(signature_type);
    TMDHeader *tmd = tmd_buffer + signature_size + sizeof(int);
    PrintTMDInfo(signature_type, *tmd);
    // go through each content
    bool found_content = false;
    TMDContent *content_ptr = (void *)tmd + sizeof(TMDHeader);
    for (int i = 0; i < BE16(tmd->num_contents); i++) {
        if (BE16(content_ptr->index) == index) {
            printf("Backed Up Content:\n");
            PrintContentInfo(*content_ptr);
            found_content = true;
            break;
        }
        content_ptr++;
    }
    if (!found_content) {
        printf("failed to get content from TMD\n");
        return -1;
    }

    // round to 0x40
    int current_pos = ftell(fp);
    int roundup = RoundUp(current_pos, 0x40);
    fseek(fp, roundup, SEEK_SET);

    // open the key file
    FILE *kfp = fopen(argv[2], "rb");
    if (kfp == NULL) {
        printf("failed to open keys\n");
        return -1;
    }
    unsigned char keys[0x400];
    fread(&keys, sizeof(keys), 1, kfp);
    fclose(kfp);
    OTP *otp = NULL;
    // detect if we're an OTP file or keys.bin file
    // first byte of the common key
    if (keys[0x14] == 0xEB) {
        otp = &keys;
    } else {
        KeysBIN *bin = &keys;
        otp = &(bin->otp);
    }
    printf("PRNG key for Console ID %08x: ", BE(otp->ng_id));
    for (int i = 0; i < 0x10; i++) {
        printf("%02x", otp->rng_seed[i]);
    }
    printf("\n");

    if (BE(otp->ng_id) != BE(header.console_id)) {
        printf("!! WARNING! CONSOLE ID DOES NOT MATCH! !!\n");
    }

    unsigned char *content_buffer = malloc(BE(header.content_data_size));
    fread(content_buffer, 1, BE(header.content_data_size), fp);
    for (int i = BE64(content_ptr->size); i < BE(header.content_data_size); i++) {
        printf("%02x", content_buffer[i]);
    }
    printf("\n");

    // TODO: check if we should use nulled prng key 
    DecryptContent(otp->rng_seed, index, content_buffer, BE(header.content_data_size));
    // TODO: verify SHA-1

    // append .app to the existing filename
    char*newname = malloc(strlen(argv[1]) + 5);
    sprintf(newname, "%s.app", argv[1]);
    // open the new file for writing
    FILE *newfile = fopen(newname, "wb+");
    if (newfile == NULL) {
        printf("failed to create output file\n");
        return -1;
    }
    // write the decrypted contents to the new file
    // use the size from the TMD for the hash to match properly
    fwrite(content_buffer, 1, BE64(content_ptr->size), newfile);
    // free our files and buffers
    fclose(newfile);
    fclose(fp);
    free(content_buffer);
    free(tmd_buffer);
    free(newname);

    return 0;
}
