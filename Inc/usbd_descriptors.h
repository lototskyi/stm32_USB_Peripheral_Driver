/*
 * usbd_descriptors.h
 *
 *  Created on: Jul 4, 2025
 *      Author: olexandr
 */

#ifndef USBD_DESCRIPTORS_H_
#define USBD_DESCRIPTORS_H_

#include "usb_standards.h"

const UsbDeviceDescriptor device_descriptor = {
	.bLength = sizeof(UsbDeviceDescriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB = 0x0200, // 0xJJMN
	.bDeviceClass = USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass = USB_SUBCLASS_NONE,
	.bDeviceProtocol = USB_PROTOCOL_NONE,
	.bMaxPacketSize0 = 8,
	.idVendor = 0x6666,
	.idProduct = 0x13AA,
	.bcdDevice = 0x0100,
	.iManufacturer = 0,
	.iProduct = 0,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};

const UsbConfigurationDescriptor configuration_descriptor = {

};

typedef struct {
	UsbConfigurationDescriptor usb_configuration_descriptor;
	//TODO: Add the remaining necessary descriptors
} UsbConfigurationDescriptorCombination;

const UsbConfigurationDescriptorCombination configuration_descriptor_combination = {
	.usb_configuration_descriptor = configuration_descriptor
};

#endif /* USBD_DESCRIPTORS_H_ */
