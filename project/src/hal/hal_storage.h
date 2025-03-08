#ifndef __HAL_STORAGE_H
#define __HAL_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"
#include "hal.h"

/*********************************************************************
 * Constants
 *********************************************************************/
#define STORAGE_MAX_SECTOR_SIZE    4096
#define STORAGE_MAX_SECTORS        0x1000000

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Forward declarations */
struct storage_driver;
typedef struct storage_driver storage_driver_t;

/* Storage driver interface */
struct storage_driver {
    /* Initialize storage device */
    int32_t (*init)(const hal_config_t *config);
    
    /* Deinitialize storage device */
    int32_t (*deinit)(void);
    
    /* Read sectors from storage */
    int32_t (*read_sectors)(uint32_t sector,
                           uint32_t count,
                           uint8_t *buffer);
    
    /* Write sectors to storage */
    int32_t (*write_sectors)(uint32_t sector,
                            uint32_t count,
                            const uint8_t *buffer);
    
    /* Get storage status */
    int32_t (*get_status)(void);
    
    /* Register callback */
    int32_t (*register_callback)(callback_t callback,
                                void *param);
};

/*********************************************************************
 * Public Functions
 *********************************************************************/

/**
 * @brief Initialize storage HAL
 * 
 * @param config Configuration parameters
 * @return HAL_SUCCESS if successful, error code otherwise
 */
int32_t hal_storage_init(const hal_config_t *config);

/**
 * @brief Deinitialize storage HAL
 * 
 * @return HAL_SUCCESS if successful, error code otherwise
 */
int32_t hal_storage_deinit(void);

/**
 * @brief Read sectors from storage
 * 
 * @param sector Starting sector number
 * @param count Number of sectors to read
 * @param buffer Buffer to store data
 * @return HAL_SUCCESS if successful, error code otherwise
 */
int32_t hal_storage_read(uint32_t sector,
                        uint32_t count,
                        uint8_t *buffer);

/**
 * @brief Write sectors to storage
 * 
 * @param sector Starting sector number
 * @param count Number of sectors to write
 * @param buffer Data to write
 * @return HAL_SUCCESS if successful, error code otherwise
 */
int32_t hal_storage_write(uint32_t sector,
                         uint32_t count,
                         const uint8_t *buffer);

/**
 * @brief Get storage status
 * 
 * @return HAL_SUCCESS if ready, error code otherwise
 */
int32_t hal_storage_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_STORAGE_H */ 