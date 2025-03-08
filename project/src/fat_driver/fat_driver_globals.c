/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File chứa các biến global của FAT Driver module.
 *********************************************************************/

#define _GNU_SOURCE  /* For strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "fat_driver.h"
#include "fat_driver_private.h"

/* Global variables - defined in fat_driver.c */
extern fat_context_t fat_ctx;
extern fat_boot_sector_t boot_sector;
extern uint32_t fat_type; 