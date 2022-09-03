#include "hardware/i2c.h"
#include "ccs811.h"

#define CCS811_STATUS 0x00
#define CCS811_MEAS_MODE 0x01
#define CCS811_ALG_RESULT_DATA 0x02
#define CCS811_RAW_DATA 0x03
#define CCS811_ENV_DATA 0x05
#define CCS811_NTC 0x06
#define CCS811_THRESHOLDS 0x10
#define CCS811_BASELINE 0x11
#define CCS811_HW_ID 0x20
#define CCS811_HW_VERSION 0x21
#define CCS811_FW_BOOT_VERSION 0x23
#define CCS811_FW_APP_VERSION 0x24
#define CCS811_ERROR_ID 0xE0
#define CCS811_SW_RESET 0xFF

#define CCS811_HW_ID_CODE 0x81
#define CCS811_REF_RESISTOR 100000

#define CCS811_BOOTLOADER_APP_ERASE 0xF1
#define CCS811_BOOTLOADER_APP_DATA 0xF2
#define CCS811_BOOTLOADER_APP_VERIFY 0xF3
#define CCS811_BOOTLOADER_APP_START 0xF4

static uint8_t CCS811_ADDR = DEFAULT_CCS811_ADDR;
i2c_inst_t *ccs811_i2c = i2c_default;
uint16_t ccs811_sda = PICO_DEFAULT_I2C_SDA_PIN;
uint16_t ccs811_scl = PICO_DEFAULT_I2C_SCL_PIN;

// Measuring mode
uint8_t CCS_INT_DATARDY = 0, CCS_DRIVE_MODE = 1, CCS_INT_THRESH = 1;

// Status
uint8_t CCS_ERROR, CCS_DATA_READY, CCS_APP_VALID, CCS_FW_MODE;

uint8_t ccsGetMeasMode()
{
    return (CCS_INT_THRESH << 2) | (CCS_INT_DATARDY << 3) | (CCS_DRIVE_MODE << 4);
}

void CCS811_setI2C(i2c_inst_t *i, uint16_t sda, uint16_t scl, uint8_t addr)
{
    ccs811_i2c = i;
    ccs811_sda = sda;
    ccs811_scl = scl;
    CCS811_ADDR = addr;
}

uint8_t ccsRead8(uint8_t a)
{
    uint8_t r;
    i2c_write_blocking(ccs811_i2c, CCS811_ADDR, &a, 1, true);
    i2c_read_blocking(ccs811_i2c, CCS811_ADDR, &r, 1, false);
    return r;
}

void ccsWrite8(uint8_t a, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = a;
    buf[1] = data;
    i2c_write_blocking(ccs811_i2c, CCS811_ADDR, buf, 2, false);
}

void ccsRead(uint8_t a, uint8_t *buf, uint8_t num)
{
    i2c_write_blocking(ccs811_i2c, CCS811_ADDR, &a, 1, true);
    sleep_ms(25);
    i2c_read_blocking(ccs811_i2c, CCS811_ADDR, buf, num, false);
}

void ccsWrite(uint8_t a, uint8_t *buf, uint8_t num)
{
    if (num)
    {
        i2c_write_blocking(ccs811_i2c, CCS811_ADDR, &a, 1, true);
        i2c_write_blocking(ccs811_i2c, CCS811_ADDR, buf, num, false);
    }
    else
        i2c_write_blocking(ccs811_i2c, CCS811_ADDR, &a, 1, false);
}

void ccsGetStatus()
{
    uint8_t data = ccsRead8(CCS811_STATUS);
    CCS_ERROR = data & 0x01;
    CCS_DATA_READY = (data >> 3) & 0x01;
    CCS_APP_VALID = (data >> 4) & 0x01;
    CCS_FW_MODE = (data >> 7) & 0x01;
}

void ccsSWReset()
{
    uint8_t seq[] = {0x11, 0xE5, 0x72, 0x8A};
    ccsWrite(CCS811_SW_RESET, seq, 4);
}

void ccsDisableInt()
{
    CCS_INT_DATARDY = 0;
    uint8_t mode = ccsGetMeasMode();
    ccsWrite8(CCS811_MEAS_MODE, mode);
}

void ccsEnableInt()
{
    CCS_INT_DATARDY = 1;
    uint8_t mode = ccsGetMeasMode();
    ccsWrite8(CCS811_MEAS_MODE, mode);
}

void CCS811_setDriveMode(uint8_t d)
{
    CCS_DRIVE_MODE = d;
    uint8_t set = ccsGetMeasMode();
    ccsWrite8(CCS811_MEAS_MODE, set);
}

bool CCS811_available()
{
    ccsGetStatus();
    if (CCS_DATA_READY)
        return true;
    return false;
}

uint8_t CCS811_init(uint8_t mode)
{

    i2c_init(ccs811_i2c, 400 * 1000);
    gpio_set_function(ccs811_sda, GPIO_FUNC_I2C);
    gpio_set_function(ccs811_scl, GPIO_FUNC_I2C);
    gpio_pull_up(ccs811_sda);
    gpio_pull_up(ccs811_scl);

    ccsSWReset();
    sleep_ms(100);

    if (ccsRead8(CCS811_HW_ID) != CCS811_HW_ID_CODE)
        return 0;

    ccsWrite(CCS811_BOOTLOADER_APP_START, NULL, 0);
    sleep_ms(100);

    ccsGetStatus();
    if (CCS_ERROR)
        return 0;
    if (!CCS_FW_MODE)
        return 0;

    ccsDisableInt();
    sleep_ms(100);
    CCS811_setDriveMode(mode);

    return 1;
}

uint16_t _TVOC;
uint16_t _eCO2;

uint16_t _currentSelected;
uint16_t _rawADCreading;

uint8_t CCS811_readData()
{
    if (!CCS811_available())
        return false;

    uint8_t buf[8];
    //  this->read(CCS811_ALG_RESULT_DATA, buf, 8);
    ccsRead(CCS811_ALG_RESULT_DATA, buf, 8);

    _eCO2 = ((uint16_t)buf[0] << 8) | ((uint16_t)buf[1]);
    _TVOC = ((uint16_t)buf[2] << 8) | ((uint16_t)buf[3]);
    _currentSelected = ((uint16_t)buf[6] >> 2);
    _rawADCreading = ((uint16_t)(buf[6] & 3) << 8) | ((uint16_t)buf[7]);

    if (CCS_ERROR)
        return buf[5];

    else
        return 0;
}

uint16_t CCS811_getTVOC() { return _TVOC; }
uint16_t CCS811_geteCO2() { return _eCO2; }