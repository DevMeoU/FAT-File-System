#include <string.h>
#include <stdio.h>
#include "ip_driver.h"

static ip_config_t ip_config;
static bool is_initialized = false;
static FILE *storage_file = NULL;

int32_t ip_driver_init(const ip_config_t *config) {
    if (!config || !config->file_path) {
        return IP_ERROR;
    }

    // Open storage file
    storage_file = fopen(config->file_path, "rb+");
    if (!storage_file) {
        return IP_ERROR;
    }

    memcpy(&ip_config, config, sizeof(ip_config_t));
    is_initialized = true;
    return IP_SUCCESS;
}

int32_t ip_read_sector(uint32_t sector, uint8_t *data) {
    if (!is_initialized || !data || !storage_file) {
        return IP_ERROR;
    }

    // Seek to sector position
    if (fseek(storage_file, sector * 512, SEEK_SET) != 0) {
        return IP_ERROR;
    }

    // Read sector data
    if (fread(data, 1, 512, storage_file) != 512) {
        return IP_ERROR;
    }

    return IP_SUCCESS;
}

int32_t ip_write_sector(uint32_t sector, const uint8_t *data) {
    if (!is_initialized || !data || !storage_file) {
        return IP_ERROR;
    }

    // Seek to sector position
    if (fseek(storage_file, sector * 512, SEEK_SET) != 0) {
        return IP_ERROR;
    }

    // Write sector data
    if (fwrite(data, 1, 512, storage_file) != 512) {
        return IP_ERROR;
    }

    // Flush changes to disk
    fflush(storage_file);

    return IP_SUCCESS;
}

int32_t ip_close(void) {
    if (!is_initialized || !storage_file) {
        return IP_SUCCESS;
    }

    // Close storage file
    if (fclose(storage_file) != 0) {
        return IP_ERROR;
    }

    // Reset state
    storage_file = NULL;
    is_initialized = false;
    memset(&ip_config, 0, sizeof(ip_config_t));

    return IP_SUCCESS;
} 