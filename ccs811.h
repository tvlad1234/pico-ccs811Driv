#ifndef _CCS811_H
#define _CCS811_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define DEFAULT_CCS811_ADDR 0x5A

#define CCS811_DRIVE_MODE_IDLE 0x00
#define CCS811_DRIVE_MODE_1SEC 0x01
#define CCS811_DRIVE_MODE_10SEC 0x02
#define CCS811_DRIVE_MODE_60SEC 0x03
#define CCS811_DRIVE_MODE_250MS 0x04

void CCS811_setI2C(i2c_inst_t *i, uint16_t sda, uint16_t scl, uint8_t addr);
uint8_t CCS811_init(uint8_t mode);

bool CCS811_available();
uint8_t CCS811_readData();
void CCS811_setDriveMode(uint8_t drivemode);

uint16_t CCS811_getTVOC();
uint16_t CCS811_geteCO2();

#endif