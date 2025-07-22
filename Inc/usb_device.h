/*
 * usb_device.h
 *
 *  Created on: Jun 30, 2025
 *      Author: olexandr
 */

#ifndef USB_DEVICE_H_
#define USB_DEVICE_H_

typedef struct
{
	/// \brief The current USB device state
	UsbDeviceState device_state;
	/// \brief The current control transfer stage (for endpoint0)
	UsbControlTransferStage control_transfer_stage;
	/// \brief The selected USB configuration
	uint8_t configuration_value;

	/** \defgroup usbDeviceOutInVufferPointers
	 *@{*/
	void const *ptr_out_buffer;
	uint32_t out_data_size;
	void const *ptr_in_buffer;
	uint32_t in_data_size;
	/**@}*/
} UsbDevice;

#endif /* USB_DEVICE_H_ */
