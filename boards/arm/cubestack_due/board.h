/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <soc.h>

/* LEDS (Labelled 1-3 on UpBoard) */
#define LED0_GPIO_PORT  CONFIG_GPIO_ATMEL_SAM3_PORTD_DEV_NAME
#define LED0_GPIO_PIN   7

#define LED1_GPIO_PORT  CONFIG_GPIO_ATMEL_SAM3_PORTD_DEV_NAME
#define LED1_GPIO_PIN   8

#define LED2_GPIO_PORT  CONFIG_GPIO_ATMEL_SAM3_PORTB_DEV_NAME
#define LED2_GPIO_PIN   27

/* LOUDSPEAKER */
#define LS_GPIO_PORT    CONFIG_GPIO_ATMEL_SAM3_PORTC_DEV_NAME
#define LS_GPIO_PIN     20

/* TODO: Add UART, TWI defs */

#endif /* __INC_BOARD_H */
