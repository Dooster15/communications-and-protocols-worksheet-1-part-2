#include "filesystem.h"
#include <stdio.h>
#include "flash_ops.h"
#include <string.h>

// Function to check if a specific cluster is free
int is_cluster_free(uint32_t cluster_id) {
    int clusters_per_sector = SECTOR_SIZE / sizeof(CLUSTER);
    int sector_num = cluster_id / clusters_per_sector;
    int cluster_index = cluster_id % clusters_per_sector;

    SECTOR_BUFFER sb;
    if (sb.buffer == NULL) {
        printf("Failed to allocate memory for sector buffer.\n");
        return -1; // Memory allocation failed
    }

    // Load the sector containing the cluster
    flash_read_safe(sector_num, sb.buffer);

    // Cast the buffer to an array of CLUSTER
    CLUSTER* clusters = (CLUSTER*) sb.buffer;

    // Check if the specific cluster is free
    int is_free = (clusters[cluster_index].next_cluster == CLUSTER_FREE);

    // Cleanup

    return is_free;
}

void fs_init(){
    uint8_t sector_buffer[SECTOR_SIZE];  // Temporary buffer for a whole sector
    int clusters_per_sector = SECTOR_SIZE / CLUSTER_SIZE;
    // Initialize each sector
    for (int sector_num = 0; sector_num < MAX_SECTORS; sector_num++) {
        CLUSTER* clusters = (CLUSTER*)sector_buffer;

        // Initialize all clusters in this sector
        for (int i = 0; i < clusters_per_sector; i++) {
            memset(clusters[i].buffer, 0, CLUSTER_DATA_SIZE);  // Set cluster data to zeros
            clusters[i].next_cluster = CLUSTER_FREE;  // Indicate no further cluster
        }

        // Write the initialized sector back to flash
        flash_write_safe(sector_num, sector_buffer);
        printf("Debug: Initialized sector %d with empty clusters.\n", sector_num);
    }
}



// Initializes the File Allocation Table (FAT).
void fat_init() {
    // Output message indicating the initialization of the FAT.
    printf("Initializing File Allocation Table...\n");

    // Define and initialize a FATable structure.
    FATable fat;
    fat.free_count = MAX_CLUSTERS;  // Set the number of free clusters to the maximum allowed.
    printf("Free clusters calculated: %u\n", fat.free_count);  // Print the number of free clusters.

    // Create a pointer to the FATable and calculate its total size.
    uint8_t* data = (uint8_t*)&fat;
    size_t total_size = sizeof(FATable);
    size_t written = 0;  // Keep track of the number of bytes written.
    int sector_num = 0;  // Initialize sector number for tracking during the write process.

    // Loop until all the data of the FATable is written.
    while (written < total_size) {
        printf("\nInit sector: %d\n", sector_num);  // Output message indicating the current sector being initialized.
        
        // Initialize a sector buffer to temporarily hold data before writing to storage.
        SECTOR_BUFFER sb = { .sector = -1, .dirty = false };
        memset(sb.buffer, 0 ,SECTOR_SIZE);  // Clear the buffer to make sure it is clean before use.
        
        if (sb.buffer == NULL) {
            printf("Failed to allocate memory for sector buffer.\n");
            break;  // Exit the loop if memory allocation failed.
        }

        // Calculate the number of bytes to write in the current loop iteration.
        size_t bytes_to_write = total_size - written > SECTOR_SIZE ? SECTOR_SIZE : total_size - written;
        memcpy(sb.buffer, data + written, bytes_to_write);  // Copy the calculated bytes from data to the buffer.
        sb.dirty = true;  // Mark the buffer as dirty indicating that it has new data to be written.

        // Write back the initialized sector to storage if the buffer is dirty.
        if (sb.dirty) {
            flash_write_safe(sector_num, sb.buffer);  // Safe write operation to ensure data integrity.
            sb.dirty = false;  // Mark the buffer as not dirty after the data is written.
            printf("FATable data written to sector %d successfully.\n", sector_num);  // Confirm successful write operation.
        }

        written += bytes_to_write;  // Update the count of written bytes.
        sector_num++;  // Move to the next sector.
    }
}

void fat_read(FATable* fat) {
    printf("\nreading FATable structure\n");
    SECTOR_BUFFER sb;
    sb.sector = -1;  // Initialize sector to an invalid value

    int total_size = sizeof(FATable);
    int bytes_read = 0;
    int sector_num = 0;

    while (bytes_read < total_size) {
        printf("\nRead loop: Reading sector %d\n", sector_num);

        // Load the sector
        flash_read_safe(sector_num, sb.buffer);
        sb.sector = sector_num;

        // Calculate how much to copy
        int copy_size = total_size - bytes_read;
        if (copy_size > SECTOR_SIZE) {
            copy_size = SECTOR_SIZE;
        }

        // Copy the data from the sector buffer to the FATable structure
        memcpy(((uint8_t*)fat) + bytes_read, sb.buffer, copy_size);
        bytes_read += copy_size;
        sector_num++;  // Move to the next sector
    }

    printf("\nFinished reading FATable structure\n");
}


// Writes the updated File Allocation Table (FAT) to storage.
void fat_write(const FATable* fat) {
    printf("\nWriting FATable structure\n");  // Notify start of write process

    SECTOR_BUFFER sb;
    sb.sector = -1;  // Initialize sector to an invalid value for safety

    int total_size = sizeof(FATable);  // Calculate the total size of the FATable
    int bytes_written = 0;
    int sector_num = 0;

    while (bytes_written < total_size) {
        printf("\nWrite loop: Writing sector %d\n", sector_num);  // Notify which sector is being written

        // Clear the sector buffer to prepare for new data
        memset(sb.buffer, 0, SECTOR_SIZE);
        
        // Determine the number of bytes to write in this iteration
        int write_size = total_size - bytes_written;
        if (write_size > SECTOR_SIZE) {
            write_size = SECTOR_SIZE;
        }

        // Copy the necessary data from the FATable to the sector buffer
        memcpy(sb.buffer, ((uint8_t*)fat) + bytes_written, write_size);

        // Mark the buffer as dirty since it now contains new data
        sb.dirty = true;

        // Write the buffer to storage if it is marked dirty
        if (sb.dirty) {
            flash_write_safe(sector_num, sb.buffer);  // Safe write operation to ensure data integrity
            sb.dirty = false;  // Reset the dirty flag after writing
            printf("FATable data written to sector %d successfully.\n", sector_num);  // Confirm successful write operation
        }

        bytes_written += write_size;  // Update the count of bytes written
        sector_num++;  // Increment to the next sector
    }

    printf("\nFinished writing FATable structure\n");  // Notify end of write process
}
/**
 * Opens a file with the specified path and mode.
 *
 * @param path The path of the file to be opened.
 * @param mode The mode in which the file should be opened.
 * @return A pointer to the opened file, or NULL if an error occurred.
 */
// FS_FILE* fs_open(const char* path, const char* mode) {
//     // TODO: Implement fs_open
    

//     return NULL;
// }

// Function to open a file within a filesystem.
// Parameters:
//   filename: Name of the file to be opened or created.
//   mode: Mode in which the file should be opened (like "rw", "rwc").
//   fat: Pointer to the File Allocation Table where file entries are stored.
FS_FILE* fs_open(const char *filename, const char *mode, FATable *fat) {
    // The 'if' condition checks if the mode is "rw".
    if (strcmp(mode, "rw") == 0) {
        // Iterate over all possible file entries in the FAT.
        for (int i = 0; i < MAX_CLUSTERS; i++) {
            FS_FILE *file = &fat->entries[i];  // Get a pointer to the file entry in the FAT.

            // Output the filename and size of each file in the FAT for debugging.
            printf("Filename: %s\n", file->filename);
            printf("File size: %u bytes\n", file->size);

            // Check if the current file's name matches the requested filename.
            if (strcmp(filename, file->filename) == 0) {
                printf("File size: %u\n", file->size); // Print the size of the found file.
                return file;  // Return the pointer to the found file.
            }
        }
        return NULL; // Return NULL if the file was not found in the FAT.
    } else if (strcmp(filename, "rwc") == 0) {
        // The 'else if' condition checks if the mode is "rwc", but the body is empty.
        // This part is intended to handle file creation or opening with clearing content,
        // but it needs implementation.
    }

    return NULL; // Return NULL by default if no conditions are met or file is not found.
}


FS_FILE* fat_fs_new(FATable *fat){
    
}

/**
 * Closes the specified file.
 *
 * @param file A pointer to the file to be closed.
 */
void fs_Close(FS_FILE* file) {

    //set last access time
    datetime_t t;
    rtc_get_datetime(&t);
    file->last_access_datetime = t;

    // Mark the file as not in use
    file->in_use = false;
}

// Edits a file by writing data to its clusters.
// Parameters:
//   file: Pointer to the file structure.
//   data: Pointer to the data to be written to the file.
//   size: The size of the data to be written.
void fs_edit(FS_FILE* file, const uint8_t *data, int size) {
    uint32_t remaining_size = size;  // Amount of data left to write.
    uint32_t offset = 0;  // Offset in the data buffer.
    uint16_t cluster_id = file->first_cluster;  // First cluster of the file.
    int clusters_per_sector = SECTOR_SIZE / CLUSTER_SIZE;  // Number of clusters in each sector.

    // Buffer to hold the current sector's data.
    SECTOR_BUFFER sb = { .sector = -1, .dirty = false };

    // Continue until all data is written.
    while (remaining_size > 0) {
        int sector_num = cluster_id / clusters_per_sector;  // Calculate sector number.
        int cluster_num = cluster_id % clusters_per_sector;  // Calculate cluster number within the sector.

        // If the sector in the buffer is not the sector we need, or if it's dirty, update it.
        if (sb.sector != sector_num) {
            if (sb.dirty) {
                flash_write_safe(sb.sector, sb.buffer);  // Write the dirty buffer to flash.
                sb.dirty = false;  // Clear the dirty flag.
            }
            flash_read_safe(sector_num, sb.buffer);  // Read the needed sector into the buffer.
            sb.sector = sector_num;  // Update the buffer's sector number.
        }

        // Access the cluster within the buffer.
        CLUSTER* cluster_array = (CLUSTER*)sb.buffer;
        CLUSTER* cluster = &cluster_array[cluster_num];

        // Check if the cluster is free before writing.
        if (is_cluster_free(cluster_id)) {
            uint32_t bytes_to_copy = remaining_size < CLUSTER_DATA_SIZE ? remaining_size : CLUSTER_DATA_SIZE;  // Determine the number of bytes to copy.
            memcpy(cluster->buffer, data + offset, bytes_to_copy);  // Copy data to the cluster.
            offset += bytes_to_copy;  // Increment the offset by the number of bytes copied.
            remaining_size -= bytes_to_copy;  // Decrement the remaining size.
            sb.dirty = true;  // Mark the sector buffer as dirty.

            printf("Debug: Copied %u bytes to cluster %u at offset %u. Remaining size: %u\n", bytes_to_copy, cluster_id, offset, remaining_size);

            // If there's more data to write, find the next free cluster.
            if (remaining_size > 0) {
                do {
                    cluster_id++;  // Move to the next cluster ID.
                } while (!is_cluster_free(cluster_id) && cluster_id < MAX_CLUSTERS);

                // If a free cluster is found, update the cluster linkage.
                if (is_cluster_free(cluster_id)) {
                    cluster->next_cluster = cluster_id;  // Set the next cluster in the file's chain.
                    printf("Debug: Assigned next free cluster ID %u\n", cluster_id);
                } else {
                    printf("Error: No free clusters available.\n");
                    return;  // Return if no free clusters are available.
                }
            } else {
                cluster->next_cluster = CLUSTER_EOF;  // Mark the end of the file's cluster chain.
            }
        } else {
            printf("Error: Cluster %u is not free.\n", cluster_id);
            return;  // Stop if the current cluster is not free.
        }
    }

    // Write the last modified sector if it's dirty.
    if (sb.dirty) {
        flash_write_safe(sb.sector, sb.buffer);
    }

    // Update the file size if it has increased.
    if (file->size < size) {
        file->size = size;
    }

    // Update the file's last modified timestamp.
    datetime_t t;
    rtc_get_datetime(&t);  // Get the current date and time.
    file->last_mod_datetime = t;  // Set the last modified datetime of the file.

    return;
}


/**
 * Reads data from the specified file into the provided buffer.
 *
 * @param file   A pointer to the file from which to read.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param size   The maximum number of bytes to read.
 * @return The number of bytes read, or -1 if an error occurred.
 */
// Reads the entire content of a file and returns it as a byte array.
// Parameters:
//   file: Pointer to the FS_FILE structure representing the file to read.
uint8_t* fs_read(FS_FILE* file) {
    // Allocate memory for the buffer to hold the file's data.
    uint8_t* buffer = malloc(file->size);
    if (buffer == NULL) {
        printf("Memory allocation failed.\n"); // Check for successful memory allocation.
        return NULL;  // Return NULL if memory allocation fails.
    }
    printf("\nreading");  // Debug print to indicate the reading process starts.

    // Initialize a sector buffer for reading data from storage.
    SECTOR_BUFFER sb;
    sb.sector = -1;  // Initialize the sector number as -1 to ensure the first read.

    // Calculate the number of clusters per sector based on sector and cluster sizes.
    int clusters_per_sector = SECTOR_SIZE / CLUSTER_SIZE;
    uint16_t cluster_id = file->first_cluster;  // Start from the first cluster of the file.
    int offset = 0;  // Initialize offset for data copying.

    // Loop through each cluster in the file's cluster chain.
    for (int i = 0; i <= file->size / CLUSTER_DATA_SIZE; i++) {
        int sector_num = cluster_id / clusters_per_sector;  // Calculate the sector number of the current cluster.
        int cluster_num = cluster_id % clusters_per_sector;  // Calculate the cluster index within the sector.

        printf("\nreadloop");  // Debug print for each loop iteration.
        
        // If the needed sector is not already loaded, load it.
        if (sb.sector != sector_num) {
            flash_read_safe(sector_num, sb.buffer);  // Read the sector safely into the buffer.
            sb.sector = sector_num;  // Update the sector buffer's current sector number.
        }

        // Access the specific cluster within the sector.
        CLUSTER* cluster_array = (CLUSTER*) sb.buffer;
        CLUSTER* cluster = &cluster_array[cluster_num];

        // Determine the size of data to copy from the cluster.
        int copy_size = (i == file->size / CLUSTER_DATA_SIZE && file->size % CLUSTER_DATA_SIZE != 0) ?
                        file->size % CLUSTER_DATA_SIZE : CLUSTER_DATA_SIZE;

        // Copy data from the cluster to the buffer.
        memcpy(buffer + offset, cluster->buffer, copy_size);
        offset += copy_size;  // Increment the offset by the size of the data copied.

        // Check if the end of the file cluster chain is reached.
        if (cluster->next_cluster == CLUSTER_EOF) {
            return buffer; // Return the buffer if end of file is reached.
        }
        cluster_id = cluster->next_cluster;  // Move to the next cluster in the chain.
    }

    // Print the entire file content as characters for debugging.
    for (int i = 0; i < file->size; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");

    return buffer;  // Return the buffer containing the file data.
}


/**
 * Writes data from the provided buffer to the specified file.
 *
 * @param file   A pointer to the file to which to write.
 * @param buffer A pointer to the buffer containing the data to be written.
 * @param size   The number of bytes to write.
 * @return The number of bytes written, or -1 if an error occurred.
 */
int fs_write(FS_FILE* file, const uint8_t *data, int size) {
    uint32_t remaining_size = size;  // Track the amount of data left to write.
    uint32_t offset = 0;  // Offset in the input data buffer.
    uint16_t cluster_id = file->first_cluster;  // Start at the first cluster of the file.
    int clusters_per_sector = SECTOR_SIZE / CLUSTER_SIZE;  // Determine how many clusters each sector holds.
    SECTOR_BUFFER sb = { .sector = -1, .dirty = false };  // Initialize a sector buffer.

    while (remaining_size > 0) {  // Loop until all data is written.
        int sector_num = cluster_id / clusters_per_sector;  // Calculate the sector number for the current cluster.
        int cluster_num = cluster_id % clusters_per_sector;  // Determine the cluster's position within its sector.

        // Load the sector if it's not already loaded, or if it's dirty.
        if (sb.sector != sector_num) {
            if (sb.dirty) {
                flash_write_safe(sb.sector, sb.buffer);  // Write back the dirty sector to storage.
                sb.dirty = false;
            }
            flash_read_safe(sector_num, sb.buffer);  // Read the new sector into the buffer.
            sb.sector = sector_num;  // Update the sector number in the buffer.
        }

        // Access the cluster within the sector.
        CLUSTER* cluster_array = (CLUSTER*)sb.buffer;
        CLUSTER* cluster = &cluster_array[cluster_num];

        // Only write to the cluster if it's free.
        if (is_cluster_free(cluster_id)) {
            // Determine how much data to copy to this cluster.
            uint32_t bytes_to_copy = remaining_size < CLUSTER_DATA_SIZE ? remaining_size : CLUSTER_DATA_SIZE;
            memcpy(cluster->buffer, data + offset, bytes_to_copy);
            offset += bytes_to_copy;
            remaining_size -= bytes_to_copy;
            sb.dirty = true;  // Mark the sector as dirty.

            printf("Debug: Copied %u bytes to cluster %u at offset %u. Remaining size: %u\n", bytes_to_copy, cluster_id, offset, remaining_size);

            // If there is still data left, find the next free cluster.
            if (remaining_size > 0) {
                do {
                    cluster_id++;
                } while (!is_cluster_free(cluster_id) && cluster_id < MAX_CLUSTERS);

                if (is_cluster_free(cluster_id)) {
                    cluster->next_cluster = cluster_id;  // Update the file's cluster chain.
                    printf("Debug: Assigned next free cluster ID %u\n", cluster_id);
                } else {
                    printf("Error: No free clusters available.\n");
                    return -1;  // Return error if no free clusters are found.
                }
            } else {
                cluster->next_cluster = CLUSTER_EOF;  // Mark the end of the file cluster chain.
            }
        } else {
            printf("Error: Cluster %u is not free.\n", cluster_id);
            return -1;  // Return error if the cluster is not free.
        }
    }

    // Write any remaining dirty sector to storage.
    if (sb.dirty) {
        flash_write_safe(sb.sector, sb.buffer);
    }

    // Update the file size if the new data exceeds the existing file size.
    if (file->size < size) {
        file->size = size;
    }

    // Return the number of bytes written (could be modified to return actual bytes written).
    return offset;
}
