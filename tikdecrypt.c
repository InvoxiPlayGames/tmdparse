#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#define CBC 1
#define ECB 0
#define CTR 0
#include "crypto/aes.h"
#include "tmd.h"
#include "cetk.h"

#ifndef SHOULD_BE_BE
#define BE16(i) ((((i) & 0xFF) << 8 | ((i) >> 8) & 0xFF) & 0xFFFF)
#define BE(i)   (((i) & 0xff) << 24 | ((i) & 0xff00) << 8 | ((i) & 0xff0000) >> 8 | ((i) >> 24) & 0xff)
#define BE64(i) (BE((i) & 0xFFFFFFFF) << 32 | BE(((i) >> 32) & 0xFFFFFFFF))
#else
#define BE16(i) i
#define BE(i) i
#define BE64(i) i
#endif

static unsigned char common_keys[3][0x10] = {
    // index 0, Wii common key
    { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 },
    // index 1, Wii korean key
    { 0x63, 0xb8, 0x2b, 0xb4, 0xf4, 0x61, 0x4e, 0x2e, 0x13, 0xf2, 0xfe, 0xfb, 0xba, 0x4c, 0x9b, 0x7e },
    // index 2, vWii common key
    { 0x30, 0xbf, 0xc7, 0x6e, 0x7c, 0x19, 0xaf, 0xbb, 0x23, 0x16, 0x33, 0x30, 0xce, 0xd7, 0xc2, 0x8d }
};

void DeriveTitleKey(CETK cetk, unsigned char * derived_key) {
    struct AES_ctx ctx;
    unsigned char iv[0x10] = { 0 };
    // copy the title key into the buffer
    memcpy(derived_key, cetk.title_key, 0x10);
    // copy the title id into the IV
    memcpy(iv, &cetk.title_id, sizeof(cetk.title_id));
    // initialise our aes context using the common key specified in the cetk
    AES_init_ctx(&ctx, common_keys[cetk.key_index]);
    AES_ctx_set_iv(&ctx, iv);
    // decrypt the title key
    AES_CBC_decrypt_buffer(&ctx, derived_key, 0x10);
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

int main(int argc, char **argv) {
    int index = 0;
    unsigned char title_key[0x10] = { 0 };

    if (argc < 5) {
        printf("usage: %s /path/to/ticket /path/to/content /path/to/tmd content_index\n", argv[0]);
        return -1;
    }

    // parse the index
    sscanf(argv[4], "%i", &index);

    // open the cetk file
    FILE *tikfile = fopen(argv[1], "rb");
    if (tikfile == NULL) {
        printf("failed to open ticket\n");
        return -1;
    }
    // parse the cetk and get the title key
    CETK ticket;
    fread(&ticket, sizeof(CETK), 1, tikfile);
    DeriveTitleKey(ticket, title_key);
    fclose(tikfile);

    // open the TMD file
    FILE *tmdfile = fopen(argv[3], "rb");
    if (tmdfile == NULL) {
        printf("failed to open tmd\n");
        return -1;
    }
    // parse the tmd and find our content
    TMDHeader tmd;
    // skip over the signature
    int signature_type;
    fread(&signature_type, sizeof(signature_type), 1, tmdfile);
    int signature_size = GetCertTypeSize(BE(signature_type));
    if (signature_size < 0) {
        printf("invalid TMD signature type: %08x - can't continue\n", BE(signature_type));
        return -1;
    }
    fseek(tmdfile, signature_size, SEEK_CUR);
    // read the TMD header
    fread(&tmd, sizeof(TMDHeader), 1, tmdfile);
    TMDContent cnt;
    bool found_cnt = false;
    // read each individual content from the file and print the information
    for (int i = 0; i < BE16(tmd.num_contents); i++) {
        fread(&cnt, sizeof(TMDContent), 1, tmdfile);
        if (BE16(cnt.index) == index) {
            found_cnt = true;
            break;
        }
    }
    if (!found_cnt) {
        printf("couldn't find equivalent content in provided TMD file - can't continue\n");
        return -1;
    }
    fclose(tmdfile);

    // open the content file
    FILE *cntfile = fopen(argv[2], "rb");
    if (cntfile == NULL) {
        printf("failed to open content\n");
        return -1;
    }
    // get the filesize
    fseek(cntfile, 0, SEEK_END);
    int filesize = ftell(cntfile);
    fseek(cntfile, 0, SEEK_SET);
    // read the entire file into memory
    unsigned char* buffer = malloc(filesize);
    fread(buffer, 1, filesize, cntfile);
    // decrypt the file in memory
    DecryptContent(title_key, index, buffer, filesize);
    // append .app to the existing filename
    char*newname = malloc(strlen(argv[2]) + 5);
    sprintf(newname, "%s.app", argv[2]);
    // open the new file for writing
    FILE *newfile = fopen(newname, "wb+");
    if (newfile == NULL) {
        printf("failed to create output file\n");
        return -1;
    }
    // write the decrypted contents to the new file
    // use the filesize from the TMD to get the correct output
    fwrite(buffer, 1, BE64(cnt.size), newfile);
    // free our files and buffers
    fclose(newfile);
    fclose(cntfile);
    free(buffer);
    free(newname);
    return 0;
}