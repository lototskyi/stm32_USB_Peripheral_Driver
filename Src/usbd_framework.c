/*
 * usbd_framework.c
 *
 *  Created on: Jun 17, 2025
 *      Author: olexandr
 */

#include "usbd_framework.h"


void usbd_initialize()
{
	initialize_gpio_pins();
	initialize_core();
	connect();
}
