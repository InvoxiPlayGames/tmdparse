#ifndef _CETK_H
#define _CETK_H

typedef struct _TicketLimit {
    unsigned int type;
    unsigned int usage;
} TicketLimit;

typedef struct _CETK {
    unsigned int signature_type;
    unsigned char signature[0x100];
    unsigned char padding[0x3C];
    unsigned char issuer[0x40];
    unsigned char ecdh_data[0x3C];
    unsigned char padding2[0x3];
    unsigned char title_key[0x10];
    unsigned char unk;
    unsigned long long ticket_id;
    unsigned int console_id;
    unsigned long long title_id;
    unsigned short unk2;
    unsigned short title_version;
    unsigned int titles_mask;
    unsigned int permit_mask;
    unsigned char export_allowed;
    unsigned char key_index;
    unsigned char unk3[0x30];
    unsigned char access_permissions[0x40];
    unsigned short padding3;
    TicketLimit limits[8];
} __attribute__((packed)) CETK;

#endif