#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#define APP_LOCATION 0x8004000

typedef enum
{
    READY_TO_WRITE = 0,
    WRITE_TO_FLASH = 1,
    ERASE_SECTOR = 2,
    CHECK_FLASH = 3,
    READ_DEVCE_ID = 4,
    READ_BOOT_VERSION = 5,
    IAP_CMP = 6
} iap_cmd_et;

void iap_jump_to_app(void);
uint32_t iap_write_app(uint8_t *app_data, uint32_t app_size);
uint32_t iap_read_app(uint8_t *app_data, uint32_t app_size);

#endif
