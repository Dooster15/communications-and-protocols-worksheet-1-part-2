#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"


#define MAX_SECTORS 1000
#define MAX_FILENAME_LENGTH 214
#define MAX_EXTENSION_LENGTH 10
#define MAX_CLUSTERS 1024
#define CLUSTER_SIZE 1024
#define CLUSTER_DATA_SIZE 1022
#define SECTOR_SIZE 4096
#define CLUSTER_FREE 0xFFFFFFFF
#define CLUSTER_EOF 0xFFFFFFFE
#define META_SIZE 256

// Defines a structure for a file in the filesystem.
typedef struct {
    char filename[MAX_FILENAME_LENGTH];      // Holds the name of the file.
    char extension[MAX_EXTENSION_LENGTH];    // Holds the file extension.
    uint8_t attributes;                      // File attributes like read-only, hidden, etc.
    datetime_t create_datetime;              // Timestamp for file creation.
    datetime_t last_access_datetime;         // Timestamp for the last file access.
    datetime_t last_mod_datetime;            // Timestamp for the last file modification.
    uint16_t first_cluster;                  // The first cluster index in the data area of the file.
    uint32_t size;                           // The size of the file in bytes.
    bool in_use;                             // Flag to indicate if the file entry is currently used.
} FS_FILE;

// Represents the File Allocation Table containing file entries and cluster management information.
typedef struct {
    FS_FILE entries[100];  // Array of FS_FILE to store file information.
    uint32_t free_count;   // Number of free clusters available in the filesystem.
} FATable;

// Represents a single cluster within the filesystem.
typedef struct {
    uint16_t next_cluster;          // Index of the next cluster in the file, or special values like EOF.
    uint8_t buffer[CLUSTER_DATA_SIZE];  // Data buffer corresponding to this cluster's storage.
} CLUSTER;

// Buffer used for sector-level operations, caching data before writing to or reading from storage.
typedef struct {
    uint8_t buffer[SECTOR_SIZE];    // Buffer to hold data before writing to flash storage.
    bool dirty;                     // Flag to indicate if the buffer has modified data that needs saving.
    uint32_t sector;                // Sector number that this buffer corresponds to.
} SECTOR_BUFFER;


void fat_init();
void fs_init();
void fat_read(FATable* fat);
void ls_directory();
FS_FILE* fs_open(const char *filename, const char *mode, FATable *fat);
void fs_close(FS_FILE* file);
uint8_t* fs_read(FS_FILE* file);
int fs_write(FS_FILE* file,  const uint8_t *data, int size);

#endif // FILESYSTEM_H