/*
 * usb_standards.h
 *
 *  Created on: Jun 19, 2025
 *      Author: olexandr
 */

#ifndef USB_STANDARDS_H_
#define USB_STANDARDS_H_

typedef enum UsbEndpointType
{
	USB_ENDPOINT_TYPE_CONTROL,
	USB_ENDPOINT_TYPE_ISOCHRONOUS,
	USB_ENDPOINT_TYPE_BULK,
	USB_ENDPOINT_TYPE_INTERRUPT
} UsbEndpointType;

typedef struct
{
 void (*on_usb_reset_received)();
 void (*on_setup_data_received)(uint8_t endpoint_number, uint16_t bcnt);
 void (*on_out_data_received)(uint8_t endpoint_number, uint16_t bcnt);
 void (*on_in_transfer_completed)(uint8_t endpoint_number);
 void (*on_out_transfer_completed)(uint8_t endpoint_number);
 void (*on_usb_polled)();
} UsbEvents;

typedef enum
{
	USB_DEVICE_STATE_DEFAULT,
	USB_DEVICE_STATE_ADDRESSED,
	USB_DEVICE_STATE_CONFIGURED,
	USB_DEVICE_STATE_SUSPENDED
} UsbDeviceState;

typedef enum
{
	USB_CONTROL_STAGE_SETUP, // Can also be called USB_CONTROL_STAGE_IDLE
	USB_CONTROL_STAGE_DATA_OUT,
	USB_CONTROL_STAGE_DATA_IN,
	USB_CONTROL_STAGE_DATA_IN_IDLE,
	USB_CONTROL_STAGE_DATA_IN_ZERO,
	USB_CONTROL_STAGE_STATUS_OUT,
	USB_CONTROL_STAGE_STATUS_IN
} UsbControlTransferStage;

/** \brief USB control request */
typedef struct
{
	uint8_t bmRequestType; // The transfer direction, the type of request, and the recipient
	uint8_t bRequest; // Request identity
	uint16_t wValue; // Parameter passed to the device
	uint16_t wIndex; // Parameter passed to the device
	uint16_t wLength; // The count of bytes that will be transmitted in the data stage
} UsbRequest;

/** \defgroup UsbDeviceBitMappedRequestTypeFields
 *
 *@{*/
#define USB_BM_REQUEST_TYPE_DIRECTION_MASK (1 << 7)
#define USB_BM_REQUEST_TYPE_DIRECTION_TODEVICE (0 << 7)
#define USB_BM_REQUEST_TYPE_DIRECTION_TOHOST (1 << 7)

#define USB_BM_REQUEST_TYPE_TYPE_MASK (3 << 5)
#define USB_BM_REQUEST_TYPE_TYPE_STANDARD (0 << 5)
#define USB_BM_REQUEST_TYPE_TYPE_CLASS (1 << 5)
#define USB_BM_REQUEST_TYPE_TYPE_VENDOR (2 << 5)

#define USB_BM_REQUEST_TYPE_RECIPIENT_MASK (3 << 0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_DEVICE (0 << 0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_INTERFACE (1 << 0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_ENDPOINT (2 << 0)
#define USB_BM_REQUEST_TYPE_RECIPIENT_OTHER (3 << 0)
/**@}*/

/** \name USB standard requests
 *@{*/
#define USB_STANDARD_GET_STATUS 0x00 /**<\brief Return the status if the recipient is specified */
#define USB_STANDARD_CLEAR_FEATURE 0x01 /**<\brief Clear or disable a specific feature */
#define USB_STANDARD_SET_FEATURE 0x03 /**<\brief Set or enable a specific feature */
#define USB_STANDARD_SET_ADDRESS 0x05 /**<\brief Set the device address */
#define USB_STANDARD_GET_DESCRIPTOR 0x06 /**<\brief Return the specified descriptor */
#define USB_STANDARD_SET_DESCRIPTOR 0x07 /**<\brief Update existing descriptors or new descriptors */
#define USB_STANDARD_GET_CONFIG 0x08 /**<\brief Return the current device configuration value */
#define USB_STANDARD_SET_CONFIG 0x09 /**<\brief Set the device configuration */
#define USB_STANDARD_GET_INTERFACE 0x0A /**<\brief Return the selected alternate setting */
#define USB_STANDARD_SET_INTERFACE 0x0B /**<\brief Allow the host to select an alternate setting */
#define USB_STANDARD_SYNCH_FRAME 0x0C /**<\brief Set and then report an endpoint's synchronization */
/**@}*/

/** \name USB standard descriptor types
 * @{ */
#define USB_DESCRIPTOR_TYPE_DEVICE 0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION 0x02
#define USB_DESCRIPTOR_TYPE_STRING 0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT 0x05
#define USB_DESCRIPTOR_TYPE_QUALIFIER 0x06
#define USB_DESCRIPTOR_TYPE_OTHER 0x07
#define USB_DESCRIPTOR_TYPE_INTERFACEPOWER 0x08
#define USB_DESCRIPTOR_TYPE_OTG 0x09
#define USB_DESCRIPTOR_TYPE_DEBUG 0x0A
#define USB_DESCRIPTOR_TYPE_INTERFASEASSOC 0x0B
#define USB_DESCRIPTOR_TYPE_CS_INTERFACE 0x24
#define USB_DESCRIPTOR_TYPE_CS_ENDPOINT 0x25
/** @} */

/** \name USB classes
 * @{ */
#define USB_CLASS_PER_INTERFACE 0x00 /**<\brief Class defined on interface level */
#define USB_CLASS_AUDIO 0x01 /**<\brief Audio device class */
#define USB_CLASS_PHYSICAL 0x05 /**<\brief Physical device class */
#define USB_CLASS_STILL_IMAGE 0x06 /**<\brief Still Imaging device class */
#define USB_CLASS_PRINTER 0x07 /**<\brief Printer device class */
#define USB_CLASS_MASS_STORAGE 0x08 /**<\brief Mass Storage device class */
#define USB_CLASS_HUB 0x09 /**<\brief HUB device class */
#define USB_CLASS_CSCID 0x0B /**<\brief Smart Card device class */
#define USB_CLASS_CONTENT_SEC 0x0D /**<\brief Content Security device class */
#define USB_CLASS_VIDEO 0x0E /**<\brief Video device class */
#define USB_CLASS_HEALTHCARE 0x0F /**<\brief Personal Healthcare device class */
#define USB_CLASS_AV 0x10 /**<\brief Audio/Video device class */
#define USB_CLASS_BILLBOARD 0x11 /**<\brief Billboard device class */
#define USB_CLASS_CBRIDGE 0x12 /**<\brief USB Type-C Bridge device class */
#define USB_CLASS_DIAGNOSTIC 0xDC /**<\brief Diagnostic device class */
#define USB_CLASS_WIRELESS 0xE0 /**<\brief Wireless controller class */
#define USB_CLASS_MISC 0xEF /**<\brief Miscellaneous device class */
#define USB_CLASS_IAD 0xEF /**<\brief Class defined on interface association level */
#define USB_CLASS_APP_SPEC 0xFE /**<\brief Application Specific class */
#define USB_CLASS_VENDOR 0xFE /**<\brief Vendor specific class */

#define USB_SUBCLASS_NONE 0x00 /**<\brief No subclass defined */
#define USB_SUBCLASS_IAD 0x02 /**<\brief Subclass defined on interface association level */
#define USB_SUBCLASS_VENDOR 0xFF /**<\brief Vendor specific subclass */

#define USB_PROTOCOL_NONE 0x00 /**<\brief No protocol defined */
#define USB_PROTOCOL_IAD 0x01 /**<\brief Protocol defined on interface association level */
#define USB_PROTOCOL_VENDOR 0xFF /**<\brief Vendor specific protocol */
/** @} */

/**\brief Represent a USB standard device descriptor
 * \details A device descriptor describes general information about a USB device. It includes
 * information that applies globally to the device and all of the device's configurations. A USB
 * device has only one device descriptor. A high-speed capable device that has different device
 * information for full-speed and high-speed must also have a \ref usb_qualifier_descriptor.
 */
typedef struct {
	uint8_t bLength; /**<\brief Size of the descriptor (in bytes). */
	uint8_t bDescriptorType; /**<\brief \ref USB_DESCRIPTOR_TYPE_DEVICE device descriptor. */
	uint16_t bcdUSB; /**<\brief USB specification release number. */
	uint8_t bDeviceClass; /**<\brief USB device class. */
	uint8_t bDeviceSubClass; /**<\brief USB device subclass. */
	uint8_t bDeviceProtocol; /**<\brief USB device protocol. */
	uint8_t bMaxPacketSize0; /**<\brief Max packet size for the control endpoint 0 (8, 16, 32 or 64). */
	uint16_t idVendor; /**<\brief Vendor ID for the USB device. */
	uint16_t idProduct; /**<\brief Product ID for the USB device. */
	uint16_t bcdDevice; /**<\brief Device release number. */
	uint8_t iManufacturer; /**<\brief String descriptor index for the manufacturer's name. */
	uint8_t iProduct; /**<\brief String descriptor index for the product name. */
	uint8_t iSerialNumber; /**<\brief String descriptor index for the product serial number. */
	uint8_t bNumConfigurations; /**<\brief Total number of configurations supported by the USB device. */
} UsbDeviceDescriptor;

typedef struct {
	uint8_t bLength; /**<\brief Size of the descriptor (in bytes). */
	uint8_t bDescriptorType; /**<\brief \ref USB_DESCRIPTOR_TYPE_CONFIGURATION descriptor. */
	uint8_t wTotalLength; /**<\brief Size of the configuration descriptor header, and all subdescriptors. */
	uint8_t bNumInterfaces; /**<\brief Total number of interfaces in the configuration. */
	uint8_t bConfigurationValue; /**<\brief Configuration value of the current configuration descriptor. */
	uint8_t iConfiguration; /**<\brief Index of a string descriptor describing this configuration. */
	uint8_t bmAttributes; /**<\brief Configuration attributes: Self Powered and Remote Wake Up. */
	uint8_t bMaxPower; /**<\brief Maximum power consumption of the device. */
} UsbConfigurationDescriptor;

#endif /* USB_STANDARDS_H_ */
