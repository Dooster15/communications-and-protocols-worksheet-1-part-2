#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "flash_ops.h"
#include "filesystem.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"


int main() {
    run_tests();
    return 0;
}

void test_stdio_init_all() {
    stdio_init_all();
    printf("stdio_init_all() called successfully.\n");
}

void test_rtc_init_and_set() {
    datetime_t t = {
        .year = 2020, .month = 6, .day = 5, .dotw = 5, .hour = 15, .min = 45, .sec = 0
    };
    rtc_init();
    int set_result = rtc_set_datetime(&t);
    printf("RTC initialized and datetime set with result: %d\n", set_result);
}

void test_usb_connection() {
    while (!stdio_usb_connected()) {
        printf("Waiting for USB connection...\n");
        sleep_ms(100);
    }
    printf("USB connection established.\n");
}
void test_fat_init() {
    fat_init();
    printf("FAT initialized successfully.\n");
}

void test_fat_read() {
    FATable fat;
    fat_read(&fat);
    printf("Read FAT. Free clusters available: %u\n", fat.free_count);
}

void test_fs_write_and_read() {
    FS_FILE file = {
        .filename = "testfile", .extension = "txt", .attributes = 0,
        .create_datetime = 0, .last_access_datetime = 0, .last_mod_datetime = 0,
        .first_cluster = 100, .size = 0, .in_use = true
    };
    const uint8_t data[] = "Hello, world!Hello, world!Hello, world!Hello, world!Hello, world!Hello";
    int write_result = fs_write(&file, data, sizeof(data));
    if (write_result == 0) {
        printf("Data written successfully.\n");
    } else {
        printf("Error writing data.\n");
    }

    FS_FILE* found_file = fs_open("testfile", "w", &fat);
    if (found_file != NULL) {
        uint8_t* buffer = fs_read(found_file); // Implement fs_read function
        if (buffer != NULL) {
            printf("File read successfully, content: %s\n", buffer);
            free(buffer);
        } else {
            printf("Error reading file content.\n");
        }
    } else {
        printf("File not found or error opening file.\n");
    }
}

void test_fs_create_and_delete() {
    FS_FILE new_file = {
        .filename = "newfile", .extension = "tmp", .attributes = 0,
        .create_datetime = 0, .last_access_datetime = 0, .last_mod_datetime = 0,
        .first_cluster = 100, .size = 0, .in_use = true
    };
    // Simulate file creation
    int create_result = fs_create(&new_file);
    if (create_result == 0) {
        printf("File created successfully.\n");
    } else {
        printf("Error creating file.\n");
    }

    // Simulate file deletion
    int delete_result = fs_delete(&new_file);
    if (delete_result == 0) {
        printf("File deleted successfully.\n");
    } else {
        printf("Error deleting file.\n");
    }
}
void test_fs_error_handling() {
    FS_FILE* null_file = NULL;
    const uint8_t data[] = "Temporary data";
    // Attempt to write using a NULL file pointer
    int write_result = fs_write(null_file, data, sizeof(data));
    printf("Writing to a NULL file pointer resulted in: %d\n", write_result);

    // Attempt to open a file that does not exist
    FS_FILE* file = fs_open("nonexistent", "r", NULL);
    if (file == NULL) {
        printf("Correctly handled attempt to open a nonexistent file.\n");
    } else {
        printf("Error: Managed to open a file that should not exist.\n");
    }
}


void run_tests() {
    test_stdio_init_all();
    test_rtc_init_and_set();
    test_usb_connection();
    test_rtc_set_edge_dates();
    test_fat_init();
    test_fat_read();
    test_fs_write_and_read();
    test_fs_create_and_delete();
    test_fs_error_handling();
}
