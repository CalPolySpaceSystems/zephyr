// I2C Functions for TE MS5607-02BA03 & MS5611-01BA03
// Patrick Chizek
// Written for CPSS, 8/2018

#include <i2c.h>
#include <misc/util.h>
#include <kernel.h>
#include <sensor.h>
#include <misc/__assert.h>
#include <misc/printk.h>
#include <device.h>
#include <i2c.h>
#include <math.h>

#include "ms56xx.h"

/* Calibration values */
uint16_t C[6] = {0};
float last_temp;

static int ms56xx_sample_fetch_chan(struct device *dev, enum sensor_channel chan)
{

    struct ms56xx_data *drv_data = dev->driver_data;
    float press;
    float temp;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_DIE_TEMP ||  chan == SENSOR_CHAN_PRESS || chan == SENSOR_CHAN_ALTITUDE);

    int rc;
    u8_t raw[3];

    /* Read Temperature */
    if (chan == SENSOR_CHAN_DIE_TEMP || chan == SENSOR_CHAN_ALTITUDE || chan == SENSOR_CHAN_ALL){

        ms56xx_prime(drv_data,MS56XX_TEMP);
        k_sleep(10);

        rc = ms56xx_read_raw(drv_data,raw);
        if (rc){return rc;}

        u32_t D2 = (raw[0]<<16) | (raw[1]<<8) | (raw[2]);

        if (chan == SENSOR_CHAN_ALTITUDE || chan == SENSOR_CHAN_ALL){

            s32_t dT = D2-((u32_t)C[4] << 8);     //update '_dT'

            // Below, 'dT' and '_C[6]'' must be casted in order to prevent overflow
            // A bitwise division can not be done since it is unpredictible for signed integers
            s32_t T = 2000 + ((s64_t)dT * C[5])/8388608;
        
            temp = T / 100.0;
            last_temp = temp;

            if (chan == SENSOR_CHAN_TEMP || chan == SENSOR_CHAN_ALL){
                drv_data->die_temp = temp;
            }

        }

    }

    /* Read Pressure */
    if (chan == SENSOR_CHAN_PRESS || chan == SENSOR_CHAN_ALTITUDE || chan == SENSOR_CHAN_ALL){

        ms56xx_prime(drv_data,MS56XX_PRESS);
        k_sleep(10);

        rc = ms56xx_read_raw(drv_data,raw);
        if (rc){return rc;}

        u32_t D1 = (raw[0]<<16) | (raw[1]<<8) | (raw[2]);

        if  (chan == SENSOR_CHAN_ALTITUDE || chan == SENSOR_CHAN_ALL || options == MS56XX_DATA_FMT_FLOAT){

            s32_t dT = (int32_t)(((((last_temp) * 100.0) - 2000) * 8388608) / C[5]);

            #ifdef MS5607

            s64_t OFF  = (int64_t)C[1]*131072 + ((int64_t)C[3]*dT)/64;
            s64_t SENS = (int64_t)C[0]*65536 + ((int64_t)C[2]*dT)/128;

            #endif

            #ifdef MS5611

            s64_t OFF  = (int64_t)C[1]*65536 + ((int64_t)C[3]*dT)/128;
            s64_t SENS = (int64_t)C[0]*32768 + (int64_t)C[2]*dT/256;

            #endif
            
            s32_t P = (D1*SENS/2097152 - OFF)/32768;
            
            press = P / 100.0;
            
            if (chan == SENSOR_CHAN_PRESS || chan == SENSOR_CHAN_ALL){
                drv_data->pressure = press;
            }

        }

    }

    /* Calculate Altitude */ 
    if (chan == SENSOR_CHAN_ALTITUDE || chan == SENSOR_CHAN_ALL){

        drv_data->altitude = ((pow((1013.25 / press), 1/5.257) - 1.0)
        * (temp + 273.15)) / 0.0065;

    }

	return 0;
}

static int ms56xx_channel_get(struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{

    struct ms56xx_data *drv_data = dev->driver_data;
    double buf;

    if (chan == SENSOR_CHAN_PRESS){

        val->val2 = (u16_t)(modf(drv_data->pressure,&buf)*10000);
        val->val1 = (s16_t)buf;

        return 0;

    }

    if (chan == SENSOR_CHAN_DIE_TEMP){

        val->val2 = (u16_t)(modf(drv_data->die_temp,&buf)*10000);
        val->val1 = (s16_t)buf;

        return 0;

    }

    if (chan == SENSOR_CHAN_ALTITUDE){

	    val->val2 = (u16_t)(modf(drv_data->altitude,&buf)*10000);
        val->val1 = (s16_t)buf;

        return 0;

    }

	return -EINVAL;

}

/* Reset Barometer and Read PROM for calibration values */
int ms56xx_init_device(struct device *dev) {
    
    struct ms56xx_data *drv_data = dev->driver_data;

    int rc;
    u8_t i2buf[2];
    u8_t prom;

    ms56xx_reset(drv_data);

    /* Get calibration data */
    for(u8_t i=0;i<6;i++) {

       /* Get address of calibration coefficients */
        prom = BARO_PROM_READ + 2 + (i*2);

        /* Send PROM read command */
        rc = i2c_write(drv_data->i2c,&prom,1,BARO_ADDR);
        //if (rc){return rc;}

        k_sleep(5);

        /* Read 16-Bit PROM data */
        rc = i2c_read(drv_data->i2c,i2buf,2,BARO_ADDR);
        if (rc){return rc;}

        /* Store Calibration Data */
        C[i] = ((i2buf[0] << 8) | i2buf[1]);
        
        k_sleep(1);
    }
    
    #ifdef MS56XX_DEBUG
    printk("%d",C[0]);
    printk(" , ");
    printk("%d",C[1]);
    printk(" , ");
    printk("%d",C[2]);
    printk(" , ");
    printk("%d",C[3]);
    printk(" , ");
    printk("%d",C[4]);
    printk(" , ");
    printk("%d",C[5]);
    printk("\n");
    #endif

    return rc;
}

/* Initialize device with sensor API */
int ms56xx_init(struct device *dev){

    //dev = device_get_binding(CONFIG_MS56XX_NAME);

    struct ms56xx_data *drv_data = dev->driver_data;

    drv_data->i2c = device_get_binding(CONFIG_MS56XX_I2C_MASTER_DEV_NAME);

    if (drv_data->i2c == NULL) {
		SYS_LOG_ERR("Failed to get pointer to %s device!",
			    CONFIG_MS56XX_I2C_MASTER_DEV_NAME);
		return -EINVAL;
	}

	if (ms56xx_init_device(dev) < 0) {
		SYS_LOG_ERR("Failed to initialize device!");
		return -EIO;
	}

    return 0;

}

/* Exports calibration coefficients (must be called after ms56xx_init) */
int ms56xx_calibration_fetch(struct device *dev, u16_t cal[]){

    for (int i=0;i<6;i++){
        cal[i] = C[i];
    }

    return 0;

}

/* Reset Barometer */
int ms56xx_reset(struct ms56xx_data *drv_data){

    u8_t reset = BARO_RESET;
    int rc = i2c_write(drv_data->i2c,&reset,1,BARO_ADDR);
    k_sleep(3);
    return rc;

}

/* Take Reading from Barometer */
int ms56xx_prime(struct ms56xx_data *drv_data,u8_t reading) {

    int rc;

    /* Only prime if user is attempting a legal reading */
    if ((reading==MS56XX_PRESS) || (reading==MS56XX_TEMP)){
        rc = i2c_write(drv_data->i2c,&reading,1,BARO_ADDR);
    }
    else{
        return EINVAL;
    }

    return 0;

}

/* Reads the raw values based on the previous priming method (Temperature or Pressure) */
int ms56xx_read_raw(struct ms56xx_data *drv_data,u8_t raw[]) {
  
    int rc;
    u8_t buf = 0x00;

    /* Send Read Commmand (0x00) */
    rc = i2c_write(drv_data->i2c,&buf,1,BARO_ADDR);

    if (rc){return rc;}
  
    /* Request 24-Bit Reading */ 
    rc = i2c_read(drv_data->i2c,raw, 3,BARO_ADDR);

    return rc;

}

struct ms56xx_data ms56xx_driver;

static const struct sensor_driver_api ms56xx_driver_api = {
	.sample_fetch = ms56xx_sample_fetch_chan,
    .channel_get = ms56xx_channel_get,
};

DEVICE_AND_API_INIT(ms56xx, CONFIG_MS56XX_NAME, ms56xx_init,
		    &ms56xx_driver, NULL,
		    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
		    &ms56xx_driver_api);