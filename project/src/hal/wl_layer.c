/**
 * @file wl_layer.c
 * @author Le Duc Son
 * @date 2026-07-02
 * @brief ESP-IDF Wear Levelling (WL) translation layer - read support
 */

#include "wl_layer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WL_STATE_HEADER_SIZE  64u  /* sizeof(wl_state_t) on target        */
#define WL_CONFIG_SIZE        36u  /* sizeof(wl_config_t) on target       */
#define WL_SUPPORTED_VERSION  2u

/* Local helpers */
static int wl_read_raw(IPDriver* driver, uint64_t offset, void* buf, uint32_t size);
static int wl_try_parse_cfg(IPDriver* driver, uint64_t file_size, uint32_t sector_size, WLConfig* out);
static int wl_load_state(WLLayer* wl, IPDriver* driver);
static uint32_t wl_recover_pos(WLLayer* wl, IPDriver* driver);

/**
 * CRC32 exactly as ESP-IDF crc32_le(0xFFFFFFFF, buf, len):
 * reflected CRC-32, polynomial 0xEDB88320, implemented as
 * ~crc_update(~init) with init = 0xFFFFFFFF.
 */
uint32_t wl_crc32(const void* buf, uint32_t len) {
    const uint8_t* p = (const uint8_t*)buf;
    uint32_t crc = 0x00000000u; /* == ~0xFFFFFFFF */
    for (uint32_t i = 0; i < len; i++) {
        crc ^= p[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xEDB88320u : 0u);
        }
    }
    return ~crc;
}

/**
 * Read raw bytes from the image file at an absolute byte offset,
 * bypassing the sector-based API.
 */
static int wl_read_raw(IPDriver* driver, uint64_t offset, void* buf, uint32_t size) {
    if (!driver || !driver->img_file || !buf) return -1;
    if (fseek(driver->img_file, (long)offset, SEEK_SET) != 0) return -1;
    if (fread(buf, 1, size, driver->img_file) != size) return -1;
    return 0;
}

/**
 * Try to read + validate a wl_config located cfg_size bytes before EOF,
 * assuming the given flash sector size.
 */
static int wl_try_parse_cfg(IPDriver* driver, uint64_t file_size, uint32_t sector_size, WLConfig* out) {
    uint8_t raw[WL_CONFIG_SIZE];
    uint32_t cfg_size = ((WL_CONFIG_SIZE + sector_size - 1) / sector_size) * sector_size;

    if (file_size < cfg_size) return -1;
    if (wl_read_raw(driver, file_size - cfg_size, raw, WL_CONFIG_SIZE) != 0) return -1;

    WLConfig cfg;
    memcpy(&cfg, raw, WL_CONFIG_SIZE);

    /* CRC covers the first 32 bytes (all fields except crc itself) */
    if (wl_crc32(raw, WL_CONFIG_SIZE - 4) != cfg.crc) return -1;

    /* Sanity checks */
    if (cfg.version != WL_SUPPORTED_VERSION) return -1;
    if (cfg.full_mem_size != file_size) return -1;
    if (cfg.sector_size != sector_size) return -1;
    if (cfg.page_size == 0 || (cfg.page_size % 512) != 0) return -1;
    if (cfg.wr_size == 0 || cfg.wr_size > sector_size) return -1;

    *out = cfg;
    return 0;
}

/**
 * Load wl_state (copy 1, falling back to copy 2) and validate its CRC.
 */
static int wl_load_state(WLLayer* wl, IPDriver* driver) {
    uint8_t raw[WL_STATE_HEADER_SIZE];
    uint64_t addrs[2] = { wl->addr_state1, wl->addr_state2 };

    for (int i = 0; i < 2; i++) {
        if (wl_read_raw(driver, addrs[i], raw, WL_STATE_HEADER_SIZE) != 0) continue;

        WLState st;
        memcpy(&st, raw, WL_STATE_HEADER_SIZE);

        /* CRC covers the first 60 bytes (all fields except crc itself) */
        if (wl_crc32(raw, WL_STATE_HEADER_SIZE - 4) != st.crc) continue;
        if (st.version != WL_SUPPORTED_VERSION) continue;
        if (st.block_size != wl->cfg.page_size) continue;

        wl->state = st;
        return 0;
    }
    return -1;
}

/**
 * Recover the true dummy-page position by scanning the pos-update records
 * that follow the state header (mirrors WL_Flash::recoverPos()).
 * A record is "set" when it is not fully erased (not all 0xFF).
 */
static uint32_t wl_recover_pos(WLLayer* wl, IPDriver* driver) {
    uint32_t pos = 0;
    uint32_t wr_size = wl->cfg.wr_size;
    uint8_t* rec = malloc(wr_size);
    if (!rec) return wl->state.pos; /* fall back to stored value */

    for (uint32_t i = 0; i < wl->state.max_pos; i++) {
        pos = i;
        if (wl_read_raw(driver, wl->addr_state1 + WL_STATE_HEADER_SIZE + (uint64_t)i * wr_size,
                        rec, wr_size) != 0) {
            break;
        }
        bool set = false;
        for (uint32_t b = 0; b < wr_size; b++) {
            if (rec[b] != 0xFF) { set = true; break; }
        }
        if (!set) break;
    }

    free(rec);
    return pos;
}

/**
 * Detect and initialise the WL layer. See header for contract.
 */
int wl_layer_init(WLLayer* wl, IPDriver* driver) {
    if (!wl || !driver || !driver->img_file) return -1;

    memset(wl, 0, sizeof(WLLayer));

    /* Determine image size */
    if (fseek(driver->img_file, 0, SEEK_END) != 0) return -1;
    long fsize = ftell(driver->img_file);
    if (fsize <= 0) return -1;
    uint64_t file_size = (uint64_t)fsize;

    /* The config sits in the last sector; try common flash sector sizes */
    static const uint32_t candidates[] = { 4096u, 512u };
    int found = -1;
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        if (wl_try_parse_cfg(driver, file_size, candidates[i], &wl->cfg) == 0) {
            found = 0;
            break;
        }
    }
    if (found != 0) {
        return 1; /* Not a wear-levelled image: treat as plain FAT */
    }

    /* Derive the layout exactly as WL_Flash::config() does */
    uint32_t sector_size = wl->cfg.sector_size;
    uint64_t state_size = sector_size;
    uint64_t min_state = WL_STATE_HEADER_SIZE +
                         ((uint64_t)wl->cfg.full_mem_size / sector_size) * wl->cfg.wr_size;
    if (state_size < min_state) {
        state_size = ((min_state + sector_size - 1) / sector_size) * sector_size;
    }
    uint64_t cfg_size = ((WL_CONFIG_SIZE + sector_size - 1) / sector_size) * sector_size;

    if (wl->cfg.full_mem_size < state_size * 2 + cfg_size + wl->cfg.page_size) {
        return 1; /* Layout impossible: not WL */
    }

    wl->addr_state1 = wl->cfg.full_mem_size - state_size * 2 - cfg_size;
    wl->addr_state2 = wl->cfg.full_mem_size - state_size - cfg_size;
    wl->addr_cfg    = wl->cfg.full_mem_size - cfg_size;
    wl->flash_size  = ((wl->cfg.full_mem_size - state_size * 2 - cfg_size) /
                       wl->cfg.page_size - 1) * wl->cfg.page_size;

    if (wl_load_state(wl, driver) != 0) {
        printf("[WL] Config found but state is corrupt - refusing to translate\n");
        return -1;
    }

    /* The stored pos may be stale; recover it from the update records */
    wl->state.pos = wl_recover_pos(wl, driver);
    if (wl->state.pos >= wl->state.max_pos) {
        wl->state.pos = 0;
    }

    wl->enabled = true;

    printf("[WL] ESP-IDF wear-levelling layer detected:\n");
    printf("[WL]   partition size : %u bytes\n", wl->cfg.full_mem_size);
    printf("[WL]   page size      : %u bytes\n", wl->cfg.page_size);
    printf("[WL]   usable size    : %llu bytes\n", (unsigned long long)wl->flash_size);
    printf("[WL]   pos=%u move_count=%u max_pos=%u\n",
           wl->state.pos, wl->state.move_count, wl->state.max_pos);

    return 0;
}

/**
 * Logical -> physical translation (mirrors WL_Flash::calcAddr()).
 */
uint64_t wl_layer_translate(const WLLayer* wl, uint64_t addr) {
    if (!wl || !wl->enabled) return addr;

    uint64_t page_size = wl->cfg.page_size;
    uint64_t result = (wl->flash_size -
                       ((uint64_t)wl->state.move_count * page_size) % wl->flash_size +
                       addr) % wl->flash_size;
    uint64_t dummy_addr = (uint64_t)wl->state.pos * page_size;

    if (result >= dummy_addr) {
        result += page_size; /* skip over the dummy page */
    }

    return wl->cfg.start_addr + result;
}
