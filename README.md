# pico-ccs811Driv
CCS811 library for pico-sdk 
Based on [Adafruit_CCS811](https://github.com/adafruit/Adafruit_CCS811) 
## Usage
Add the _pico-ccs811Driv_ subdirectory to the CMakeLists.txt of your project and include the library in _target_link_libraries_.
### Initializing the sensor
Before reading the sensor, it must be initialized with _CCS811_init_.\
_CCS811_init_ takes the sensor drive mode as a parameter and returns `true` if the sensor was initialized successfully\
_Example:_ `bool ok = CCS811_init(CCS811_DRIVE_MODE_1SEC);`\
Other usable modes are `CCS811_DRIVE_MODE_10SEC`,  `CCS811_DRIVE_MODE_60SEC`,  `CCS811_DRIVE_MODE_250MS`. 
###
By default, the library uses pins 4 (SDA) and 5 (SCL). `CCS811_setI2C(i2c_inst_t *i, uint16_t sda, uint16_t scl, uint8_t addr);` can be used to select the I2C peripheral, pins and sensor adress.
### Reading the sensor
`CCS811_available()` returns `true` if there is data to be read from the sensor. \
`CCS811_readData();` reads the data from the sensor and stores it in some internal variables. It returns zero if data was read succesfully and an error code otherwise. \
`CCS811_getTVOC();` returns the last read TVOC value as `uint16_t`, in ppm \
`CCS811_geteco2();` returns the last read eCO₂ value as `uint16_t`, in ppb \
