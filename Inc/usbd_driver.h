/*
 * usbd_driver.h
 *
 *  Created on: Jun 17, 2025
 *      Author: olexandr
 */

#ifndef USBD_DRIVER_H_
#define USBD_DRIVER_H_

#include "stm32f4xx.h"

#define USB_OTG_HS_GLOBAL  ((USB_OTG_GlobalTypeDef *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_GLOBAL_BASE))
#define USB_OTG_HS_DEVICE  ((USB_OTG_DeviceTypeDef *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USB_OTG_HS_PCGCCTL ((uint32_t *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE))

// Total count of IN or OUT endpoints
#define ENDPOINT_COUNT	6

void initialize_gpio_pins();
void initialize_core();
void connect();
void disconnect();
static void configure_rxfifo_size(uint16_t size);
static void configure_txfifo_size(uint8_t endpoint_number, uint16_t size);

/**
 * @brief Return the structure contains the registers of a specific IN endpoint
 * @param endpoint_number The number of the IN endpoint we want to access its registers
 */
inline static USB_OTG_INEndpointTypeDef * IN_ENDPOINT(uint8_t endpoint_number)
{
	return (USB_OTG_INEndpointTypeDef *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (endpoint_number * 0x20));
}

/**
 * @brief Return the structure contains the registers of a specific OUT endpoint
 * @param endpoint_number The number of the OUT endpoint we want to access its registers
 */
inline static USB_OTG_OUTEndpointTypeDef * OUT_ENDPOINT(uint8_t endpoint_number)
{
	return (USB_OTG_OUTEndpointTypeDef *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + (endpoint_number * 0x20));
}

inline static __IO uint32_t *FIFO(uint8_t endpoint_number)
{
	return (__IO uint32_t *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_FIFO_BASE + (endpoint_number * 0x1000));
}

#endif /* USBD_DRIVER_H_ */
