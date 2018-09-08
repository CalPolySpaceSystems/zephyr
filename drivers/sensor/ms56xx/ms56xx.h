// I2C Library for TE MS5607-02BA03 & MS5611-01BA03
// Patrick Chizek
// Written for CPSS, 8/2018

#ifndef _BARO_H
#define _BARO_H

/* Barometer Commands */
#define MS56XX_PRESS 	(0x48)
#define MS56XX_TEMP 	(0x58)
#define BARO_ADDR 	    (0x76)
#define BARO_RESET 		(0x1E)
#define BARO_PROM_READ 	(0xA0)

/* Settings */
#define MS56XX_DATA_FMT_FLOAT (0x01)
#define MS56XX_DATA_FMT_RAW (0x00)

/* Part Definition */
#define MS5607
//#define MS5611

/* Debug */
//#define MS56XX_DEBUG

#include <device.h>
#include <misc/util.h>

/* NOTE: all data reads must be primed an allow for a 10 ms
 *       delay after priming before reading.
 */

struct ms56xx_data {
  struct device *i2c;
  float die_temp;
  float pressure;
  float altitude;
};

/* Fetch samples from the specified channel(s) */
//static int ms56xx_sample_fetch_chan(struct device *dev, enum sensor_channel chan);

/* Reset Barometer and Read PROM for calibration values */
int ms56xx_init_device(struct device *dev);

/* Initialize device with sensor API */
int ms56xx_init(struct device *dev);

/* Exports calibration coefficients (must be called after ms56xx_init) */
int ms56xx_calibration_fetch(struct device *dev, u16_t cal[]);

/* Reset Barometer */
int ms56xx_reset(struct ms56xx_data *drv_data);

/* Take Reading from Barometer */
int ms56xx_prime(struct ms56xx_data *drv_data,u8_t reading);

/* Reads the raw values based on the previous priming method (Temperature or Pressure) */
int ms56xx_read_raw(struct ms56xx_data *drv_data,u8_t raw[]);

#define SYS_LOG_DOMAIN "MS56XX"
#define SYS_LOG_LEVEL CONFIG_SYS_LOG_SENSOR_LEVEL

#include <logging/sys_log.h>

#endif
