#ifndef SNIOCTL_H
#define SNIOCTL_H

#define IOCTL_SND_CMD               0x01
#define IOCTL_SET_FLASH_OFFS        0x02
#define IOCTL_READ_FLASH            0x03
#define IOCTL_WRITE_FLASH           0x04
#define IOCTL_VERIFY_FLASH          0x05
#define IOCTL_ERASE_FLASH           0x06
#define IOCTL_CHECK_BOOT            0x07
#define IOCTL_READ_FLASH_STORE      0x08
#define IOCTL_WRITE_FLASH_STORE     0x09
#define IOCTL_ERASE_FLASH_STORE     0x0A
#define IOCTL_RESET_DISP_BASE       0x0B
#define IOCTL_CHECK_NVRAM_BATTERY   0x0C
#define IOCTL_SET_BUZZER            0x0D
#define IOCTL_RESET_DSP             0x0E
#define IOCTL_CAN_TEST              0x0F

#define FLASH_PAGE_SIZE             0x20000
#define FLASH_DEFAULT_OFFSET        0xC0000
#define FLASH_UPPER_LIMIT           0x1000000

#define FLASH_TESTAPP_OFFSET        0x80000
#define FLASH_TOUCH_DATA_OFFSET     0xA0000
#define FLASH_SHAVER_STORE_OFFSET   0xC0000
#define FLASH_SW_UPDATE_OFFSET      0x100000

#define FLASH_ALIGNED_OFFSET(x)     ((x) & ~(FLASH_PAGE_SIZE-1))
#define IS_FLASH_PAGE_ALIGNED(x)    (((x) & (FLASH_PAGE_SIZE-1)) == 0)
#define FLASH_BYTES_TO_PAGES(x)     (((x)+FLASH_PAGE_SIZE-1)/FLASH_PAGE_SIZE)

typedef struct {
    SnAddr aFlashStoreBase;
    SnAddr aFlashStoreTop;
    SnQByte qMaxStoreBufSize;
} SnFlashStore;

#define CMD_RD_VAR_WORD     0x1
#define CMD_WR_VAR_WORD     0x2
#define CMD_RD_VAR_WORDS    0x3
#define CMD_WR_VAR_WORDS    0x4
#define CMD_WR_FLASH_BLK    0x5
#define CMD_SERIAL          0x6

#define CMD_CRC             0xC
#define CMD_ACK             0xD
#define CMD_NACK            0xE

#define MAX_MSG_ATTEMPTS    3

#endif // SNIOCTL_H
