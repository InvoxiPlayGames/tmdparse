#ifndef _TMD_H
#define _TMD_H

typedef struct _TMDContent {
    unsigned int id;
    unsigned short index;
    unsigned short type;
    unsigned long long size;
    unsigned char SHA1[0x14];
} __attribute__((packed)) TMDContent;

typedef struct _TMDHeader {
    unsigned int signature_type; 
    unsigned char signature[0x100];
    unsigned char padding[0x3C];
    unsigned char issuer[0x40];
    unsigned char version;
    unsigned char ca_crl_version;
    unsigned char signer_crl_version;
    unsigned char vwii;
    unsigned long long ios_version;
    unsigned long long title_id;
    unsigned int title_type;
    unsigned short group_id;
    unsigned short padding2;
    unsigned short region;
    unsigned char ratings[0x10];
    unsigned char padding3[0xC];
    unsigned char ipc_mask[0xC];
    unsigned char padding4[0x12];
    unsigned int access_rights;
    unsigned short title_version;
    unsigned short num_contents;
    unsigned short boot_index;
    unsigned short padding5;
 } __attribute__((packed)) TMDHeader;

 #endif // _TMD_H