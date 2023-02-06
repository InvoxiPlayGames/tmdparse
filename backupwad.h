#ifndef _BACKUPWAD_H
#define _BACKUPWAD_H

typedef struct _BackupWadHeader {
    unsigned int header_size;
    unsigned short type;
    unsigned short version;
    unsigned int console_id;
    unsigned int save_file_count;
    unsigned int save_file_data_size;
    unsigned int content_tmd_size;
    unsigned int content_data_size;
    unsigned int backup_area_size;
    unsigned char included_contents[0x40]; //bitfield
    union {
        unsigned long long id;
        struct {
            unsigned int pad;
            unsigned char code[4];
        } name;
    } title_id;
    unsigned char mac_address[0x06];
    unsigned char reserved[0x02];
    unsigned char pad[0x10];
} BackupWadHeader;

#endif // _BACKUPWAD_H
