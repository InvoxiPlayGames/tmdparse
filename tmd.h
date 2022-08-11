#ifndef _TMD_H
#define _TMD_H

#define TMD_CERT_TYPEMASK 0xFFFF
#define TMD_CERT_RSA_4096 0
#define TMD_CERT_RSA_2048 1
#define TMD_CERT_ECC_B233 2

#define TMD_RSA_2048_SIZE 0x100
#define TMD_RSA_4096_SIZE 0x200
#define TMD_ECC_B233_SIZE 0x3C

typedef struct _TMDContent {
    unsigned int id;
    unsigned short index;
    unsigned short type;
    unsigned long long size;
    unsigned char SHA1[0x14];
} __attribute__((packed)) TMDContent;

typedef struct _TMDHeader {
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

 typedef struct _TMDPublicKeyRSA4096 {
    unsigned char modulus[TMD_RSA_4096_SIZE];
    unsigned int public_exponent;
    unsigned char padding[0x38];
 } __attribute__((packed)) TMDPublicKeyRSA4096;

 typedef struct _TMDPublicKeyRSA2048 {
    unsigned char modulus[TMD_RSA_2048_SIZE];
    unsigned int public_exponent;
    unsigned char padding[0x38];
 } __attribute__((packed)) TMDPublicKeyRSA2048;

 typedef struct _TMDPublicKeyECCB233 {
    unsigned char modulus[TMD_ECC_B233_SIZE];
    unsigned char padding[0x3C];
 } __attribute__((packed)) TMDPublicKeyECCB233;

 typedef struct _TMDCertificateData {
    unsigned char padding[0x3C];
    unsigned char issuer[0x40];
    unsigned int key_type;
    unsigned char name[0x40];
 } __attribute__((packed)) TMDCertificateData;

 #endif // _TMD_H