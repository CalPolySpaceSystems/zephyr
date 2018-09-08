/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <misc/printk.h>
#include <device.h>
#include <i2c.h>
#include <board.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sensor.h>

void main(void)
{
	int rc;
	struct sensor_value press_val,temp_val,alt_val; 
	struct device *baro = device_get_binding(CONFIG_MS56XX_NAME);

	if (baro == NULL){
		printk("Could not initialize MS56XX");
	}

	while (!rc) {

		rc = sensor_sample_fetch_chan(baro,SENSOR_CHAN_ALL);

		rc = sensor_channel_get(baro,SENSOR_CHAN_PRESS,&press_val);
		rc = sensor_channel_get(baro,SENSOR_CHAN_DIE_TEMP,&temp_val);
		rc = sensor_channel_get(baro,SENSOR_CHAN_ALTITUDE,&alt_val);

		// Print values
		printk("%d",temp_val.val1);
		printk(".");
		printk("%d",temp_val.val2);
		printk(", ");
		printk("%d",press_val.val1);
		printk(".");
		printk("%d",press_val.val2);
		printk(", ");
		printk("%d",alt_val.val1);
		printk(".");
		printk("%d",alt_val.val2);
		printk("\n");

		k_sleep(1000);

	}
	
}