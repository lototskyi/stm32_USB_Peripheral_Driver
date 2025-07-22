/*
 * usbd_framework.c
 *
 *  Created on: Jun 17, 2025
 *      Author: olexandr
 */

#include <stddef.h>
#include "Helpers/logger.h"
#include "usbd_framework.h"
#include "usbd_driver.h"
#include "usb_device.h"
#include "usbd_descriptors.h"
#include "usb_standards.h"
#include "Helpers/math.h"

static UsbDevice *usbd_handle;

void usbd_initialize(UsbDevice *usb_device)
{
	usbd_handle = usb_device;
	usb_driver.initialize_gpio_pins();
	usb_driver.initialize_core();
	usb_driver.connect();
}

void usbd_configure()
{
	//TODO: Configure the device (e.g. the endpoints active in this configuration)
}

static void process_standard_device_request()
{
	UsbRequest const *request = (UsbRequest *)usbd_handle->ptr_out_buffer;

	switch(request->bRequest)
	{
		case USB_STANDARD_GET_DESCRIPTOR:
			log_info("Standard Get Descriptor request received");
			const uint8_t descriptor_type = request->wValue >> 8;
			const uint16_t descriptor_length = request->wLength;
			//const uint8_t descriptor_index = request->wValue & 0xFF;

			switch(descriptor_type)
			{
			case USB_DESCRIPTOR_TYPE_DEVICE:
				log_info("- Get Device Descriptor.");
				usbd_handle->ptr_in_buffer = &device_descriptor;
				usbd_handle->in_data_size = descriptor_length;

				log_info("Switching control stage to IN-DATA.");
				usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_DATA_IN;
				break;
			case USB_DESCRIPTOR_TYPE_CONFIGURATION:
				log_info("- Get Configuration Descriptor.");
				usbd_handle->ptr_in_buffer = &configuration_descriptor_combination;
				usbd_handle->in_data_size = descriptor_length;

				log_info("Switching control stage to IN-DATA.");
				usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_DATA_IN;
				break;
			}

			break;
		case USB_STANDARD_SET_ADDRESS:
			log_info("Standard Set Address request received");
			const uint16_t device_address = request->wValue;

			usb_driver.set_device_address(device_address);
			usbd_handle->device_state = USB_DEVICE_STATE_ADDRESSED;

			log_info("Switching control transfer stage to IN-STATUS");
			usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_STATUS_IN;
			break;
		case USB_STANDARD_SET_CONFIG:
			log_info("Standard Set Configuration request received.");
			usbd_handle->configuration_value = request->wValue;

			usbd_configure();

			usbd_handle->device_state = USB_DEVICE_STATE_CONFIGURED;
			log_info("Switching control transfer stage to IN-STATUS");
			usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_STATUS_IN;
			break;
	}
}

static void process_request()
{
	UsbRequest const *request = (UsbRequest *)usbd_handle->ptr_out_buffer;

	switch(request->bmRequestType & (USB_BM_REQUEST_TYPE_TYPE_MASK | USB_BM_REQUEST_TYPE_RECIPIENT_MASK))
	{
		case USB_BM_REQUEST_TYPE_TYPE_STANDARD | USB_BM_REQUEST_TYPE_RECIPIENT_DEVICE:
			process_standard_device_request();
			break;
	}
}

static void process_control_transfer_stage()
{
	switch(usbd_handle->control_transfer_stage)
	{
	case USB_CONTROL_STAGE_SETUP:
		break;
	case USB_CONTROL_STAGE_DATA_IN:
		log_info("Processing IN-DATA stage.");

		uint8_t data_size = MIN(usbd_handle->in_data_size, device_descriptor.bMaxPacketSize0);

		usb_driver.write_packet(0, usbd_handle->ptr_in_buffer, data_size);
		usbd_handle->in_data_size -= data_size;
		usbd_handle->ptr_in_buffer += data_size;

		log_info("Switching control stage to IN-DATA IDLE");
		usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_DATA_IN_IDLE;

		if (usbd_handle->in_data_size == 0) {

			if (data_size == device_descriptor.bMaxPacketSize0) {
				log_info("Switching control stage to IN-DATA ZERO");
				usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_DATA_IN_ZERO;
			} else {
				log_info("Switching control stage to OUT-STATUS");
				usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_STATUS_OUT;
			}
		}

		break;
	case USB_CONTROL_STAGE_DATA_IN_IDLE:
		break;
	case USB_CONTROL_STAGE_STATUS_OUT:
		log_info("Switching control stage to SETUP");
		usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_SETUP;
		break;
	case USB_CONTROL_STAGE_STATUS_IN:
		usb_driver.write_packet(0, NULL, 0);
		log_info("Switching control stage to SETUP");
		usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_SETUP;
		break;
	}
}

static void usb_polled_handler()
{
	process_control_transfer_stage();
}

static void in_transfer_completed_handler(uint8_t endpoint_number)
{
	if (usbd_handle->in_data_size) {
		log_info("Switching control stage to IN-DATA");
		usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_DATA_IN;
	} else if (usbd_handle->control_transfer_stage == USB_CONTROL_STAGE_DATA_IN_ZERO) {
		usb_driver.write_packet(0, NULL, 0);
		log_info("Switching control stage to OUT-STATUS");
		usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_STATUS_OUT;
	}
}

static void out_transfer_completed_handler(uint8_t endpoint_number)
{

}

void usbd_poll()
{
	usb_driver.poll();
}

static void usb_reset_received_handler()
{
	usbd_handle->in_data_size = 0;
	usbd_handle->out_data_size = 0;
	usbd_handle->configuration_value = 0;
	usbd_handle->device_state = USB_DEVICE_STATE_DEFAULT;
	usbd_handle->control_transfer_stage = USB_CONTROL_STAGE_SETUP;
	usb_driver.set_device_address(0);
}

static void setup_data_received_handler(uint8_t endpoint_number, uint16_t byte_count)
{
	usb_driver.read_packet(usbd_handle->ptr_out_buffer, byte_count);

	// Print out the received data
	log_debug_array("SETUP data: ", usbd_handle->ptr_out_buffer, byte_count);

	process_request();
}

UsbEvents usb_events = {
	.on_usb_reset_received = &usb_reset_received_handler,
	.on_setup_data_received = &setup_data_received_handler,
	.on_usb_polled = &usb_polled_handler,
	.on_in_transfer_completed = &in_transfer_completed_handler,
	.on_out_transfer_completed = &out_transfer_completed_handler
};

