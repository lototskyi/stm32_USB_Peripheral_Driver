/*
 * usbd_driver.c
 *
 *  Created on: Jun 17, 2025
 *      Author: olexandr
 */

#include "usbd_driver.h"

void initialize_gpio_pins()
{
	// Enable the clock for GPIOB
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);

	// Set alternate function 12 for: PB14 (-), and PB15 (+)
	MODIFY_REG(GPIOB->AFR[1],
		GPIO_AFRH_AFSEL14 | GPIO_AFRH_AFSEL15,
		_VAL2FLD(GPIO_AFRH_AFSEL14, 0xC) | _VAL2FLD(GPIO_AFRH_AFSEL14, 0xC)
	);

	// Configure USB pins (in GPIOB) to work in alternate function mode
	MODIFY_REG(GPIOB->MODER,
		GPIO_MODER_MODER14 | GPIO_MODER_MODER15,
		_VAL2FLD(GPIO_MODER_MODER14, 2) | _VAL2FLD(GPIO_MODER_MODER15, 2)
	);
}

void initialize_core()
{
	// Enable the clock for USB core
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_OTGHSEN);

	// Configure the USB core to run in device mode, and to use the embedded full-speed PHY
	MODIFY_REG(USB_OTG_HS->GUSBCFG,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | USB_OTG_GUSBCFG_TRDT,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | _VAL2FLD(USB_OTG_GUSBCFG_TRDT, 0x09)
	);

	// Configure the device to run in full speed mode
	MODIFY_REG(USB_OTG_HS_DEVICE->DCFG,
		USB_OTG_DCFG_DSPD,
		_VAL2FLD(USB_OTG_DCFG_DSPD, 0x03)
	);

	// Enable VBUS sensing device
	SET_BIT(USB_OTG_HS->GCCFG, USB_OTG_GCCFG_VBUSBSEN);

	// Unmask the main USB core interrupts
	SET_BIT(USB_OTG_HS->GINTMSK,
		USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_SOFM |
		USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_WUIM | USB_OTG_GINTMSK_IEPINT |
		USB_OTG_GINTMSK_OEPINT | USB_OTG_GINTMSK_RXFLVLM
	);

	// Clear all pending core interrupts
	WRITE_REG(USB_OTG_HS->GINTSTS, 0xFFFFFFFF);

	// Unmask USB global interrupt
	SET_BIT(USB_OTG_HS->GAHBCFG, USB_OTG_GAHBCFG_GINT);

	// Unmask transfer completed interrupts for all endpoints
	SET_BIT(USB_OTG_HS_DEVICE->DOEPMSK, USB_OTG_DOEPMSK_XFRCM);
	SET_BIT(USB_OTG_HS_DEVICE->DIEPMSK, USB_OTG_DIEPMSK_XFRCM);
}

/**
 * Connect the USB device to the bus
 */
void connect()
{
	// Power the transceiver on
	SET_BIT(USB_OTG_HS->GCCFG, USB_OTG_GCCFG_PWRDWN);

	// Connect the device to the bus
	CLEAR_BIT(USB_OTG_HS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);
}

/**
 * Disconnect the USB device from the bus
 */
void disconnect()
{
	// Disconnect the device from the bus
	SET_BIT(USB_OTG_HS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);

	// Power the transceiver off
	CLEAR_BIT(USB_OTG_HS->GCCFG, USB_OTG_GCCFG_PWRDWN);
}

/**
 * @brief Flushes the RxFIFO of all OUT endpoints
 */
static void flush_rxfifo()
{
	SET_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_RXFFLSH);
}

/**
 * @brief Flushes the TxFIFO of an IN endpoint
 * @param endpoint_number The number of an IN endpoint to flush its TxFIFO
 */
static void flush_txfifo(uint8_t endpoint_number)
{
	// Sets the number of the TxFIFO to be flushed and then triggers the flush
	MODIFY_REG(USB_OTG_HS->GRSTCTL,
		USB_OTG_GRSTCTL_TXFNUM,
		_VAL2FLD(USB_OTG_GRSTCTL_TXFNUM, endpoint_number) | USB_OTG_GRSTCTL_TXFFLSH
	);
}

static void configure_endpoint0(uint8_t endpoint_size)
{
	// Unmask all interrupts of IN and OUT endpoint0
	SET_BIT(USB_OTG_HS_DEVICE->DAINTMSK, 1 << 0 | 1 << 16);

	// Configure the maximum packet size, activate endpoint, and NAK the endpoint (cannot send data)
	MODIFY_REG(IN_ENDPOINT(0)->DIEPCTL,
		USB_OTG_DIEPCTL_MPSIZ,
		USB_OTG_DIEPCTL_USBAEP | _VAL2FLD(USB_OTG_DIEPCTL_MPSIZ, endpoint_size) | USB_OTG_DIEPCTL_SNAK
	);

	// Clear NAK, and enable endpoint data transmission
	SET_BIT(OUT_ENDPOINT(0)->DOEPCTL,
		USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK
	);

	// Note: 64 bytes is the maximum packet size for full speed USB devices
	configure_rxfifo_size(64);
	configure_txfifo_size(0, endpoint_size);
}

static void configure_in_endpoint(uint8_t endpoint_number, UsbEndpointType endpoint_type, uint16_t endpoint_size)
{
	// Unmask all interrupts of the targeted IN endpoint
	SET_BIT(USB_OTG_HS_DEVICE->DAINTMSK, 1 << endpoint_number);

	// Activate the endpoint, set endpoint handshake to NAK (not ready to send data), set DATA0 packet identifier,
	// configure its type, its maximum packet size and assign it a TxFIFO
	MODIFY_REG(IN_ENDPOINT(endpoint_number)->DIEPCTL,
		USB_OTG_DIEPCTL_MPSIZ | USB_OTG_DIEPCTL_EPTYP | USB_OTG_DIEPCTL_TXFNUM,
		USB_OTG_DIEPCTL_USBAEP | _VAL2FLD(USB_OTG_DIEPCTL_MPSIZ, endpoint_size) | USB_OTG_DIEPCTL_SNAK |
		_VAL2FLD(USB_OTG_DIEPCTL_EPTYP, endpoint_type) | _VAL2FLD(USB_OTG_DIEPCTL_TXFNUM, endpoint_number) | USB_OTG_DIEPCTL_SD0PID_SEVNFRM
	);

	configure_txfifo_size(endpoint_number, endpoint_size);
}

static void deconfigure_endpoint(uint8_t endpoint_number)
{
	USB_OTG_INEndpointTypeDef *in_endpoint = IN_ENDPOINT(endpoint_number);
	USB_OTG_OUTEndpointTypeDef *out_endpoint = OUT_ENDPOINT(endpoint_number);

	// Mask all interrupts of the target IN and OUT endpoints
	CLEAR_BIT(USB_OTG_HS_DEVICE->DAINTMSK,
		(1 << endpoint_number) | (1 << 16 << endpoint_number)
	);

	// Clear all interrupt of the endpoint
	SET_BIT(in_endpoint->DIEPINT, 0x29FF);
	SET_BIT(out_endpoint->DOEPINT, 0x715F);

	// Disable the endpoints if possible
	if (in_endpoint->DIEPCTL & USB_OTG_DIEPCTL_EPENA) {
		// Disable endpoint transmission
		SET_BIT(in_endpoint->DIEPCTL, USB_OTG_DIEPCTL_EPDIS);
	}

	// Deactivate the endpoint
	CLEAR_BIT(in_endpoint->DIEPCTL, USB_OTG_DIEPCTL_USBAEP);


	if (endpoint_number != 0) {

		if (out_endpoint->DOEPINT & USB_OTG_DOEPCTL_EPENA) {
			// Disable endpoint transmission
			SET_BIT(out_endpoint->DOEPCTL, USB_OTG_DOEPCTL_EPDIS);
		}

		// Deactivate the endpoint
		CLEAR_BIT(out_endpoint->DOEPCTL, USB_OTG_DOEPCTL_USBAEP);
	}

	// Flush the FIFOs
	flush_txfifo(endpoint_number);
	flush_rxfifo();
}

/**
 * @brief Update the start addresses of all FIFOs according to the size of each FIFO
 */
static void refresh_fifo_start_addresses()
{
	// The first changeable start address begins after the region of RxFIFO
	uint16_t start_address = _FLD2VAL(USB_OTG_GRXFSIZ_RXFD, USB_OTG_HS->GRXFSIZ) * 4;

	// Update the start address of the FIFO0
	MODIFY_REG(USB_OTG_HS->DIEPTXF0_HNPTXFSIZ,
		USB_OTG_TX0FSA,
		_VAL2FLD(USB_OTG_TX0FSA, start_address)
	);

	// The next start address is after where the last TxFIFO ends
	start_address += _FLD2VAL(USB_OTG_TX0FD, USB_OTG_HS->DIEPTXF0_HNPTXFSIZ) * 4;

	// Update the start addresses of the rest TxFOFOs
	for (uint8_t txfifo_number = 0; txfifo_number < ENDPOINT_COUNT - 1; txfifo_number++) {
		MODIFY_REG(USB_OTG_HS->DIEPTXF[txfifo_number],
			USB_OTG_NPTXFSA,
			_VAL2FLD(USB_OTG_NPTXFSA, start_address)
		);

		start_address += _FLD2VAL(USB_OTG_NPTXFD, USB_OTG_HS->DIEPTXF[txfifo_number]) * 4;
	}
}

/*
 * @brief Configure the RxFIFO of OUT endpoints
 * @param size The size of the largest OUT endpoint in bytes
 * @note The RxFIFO is shared between all OUT endpoints
 */
static void configure_rxfifo_size(uint16_t size)
{
	// Consider the space required to save status packets in RxFIFO and get the size in term of 32-bit words
	size = 10 + (2 * ((size / 4) + 1));

	// Configure the depth of the FIFO
	MODIFY_REG(USB_OTG_HS->GRXFSIZ,
		USB_OTG_GRXFSIZ_RXFD,
		_VAL2FLD(USB_OTG_GRXFSIZ_RXFD, size)
	);

	refresh_fifo_start_addresses();
}

/**
 * @brief Configure the TxFIFO of an IN endpoint
 * @param endpoint_number The number of the IN endpoint we want to configure its TxFIFO
 * @param size The size of the IN endpoint in bytes
 * @note Any change on any FIFO will update the registers of all TxFIFO to adapt start offsets
 */
static void configure_txfifo_size(uint8_t endpoint_number, uint16_t size)
{
	// Get the FIFO size in term of 32-bit words
	size = (size + 3) / 4;

	// Configure the depth of the TxFIFO
	if (endpoint_number == 0) {
		MODIFY_REG(USB_OTG_HS->DIEPTXF0_HNPTXFSIZ,
			USB_OTG_TX0FD,
			_VAL2FLD(USB_OTG_TX0FD, size)
		);
	}

	refresh_fifo_start_addresses();
}

static void usbrst_handler()
{
	log_info("USB reset signal was detected");

	for (uint8_t i = 0; i <= ENDPOINT_COUNT; i++) {
		deconfigure_endpoint(i);
	}
}

static void enumdne_handler()
{
	log_info("USB device speed enumeration done");
	configure_endpoint0(8);
}

static void rxflvl_handler()
{
	// Pop the status information word from the RxFIFO
	uint32_t receive_status = USB_OTG_HS_GLOBAL->GRXSTSP;

	// The endpoint that received the data
	uint8_t endpoint_number = _FLD2VAL(USB_OTG_GRXSTSP_EPNUM, receive_status);
	// The count of bytes in the received packet
	uint8_t bcnt = _FLD2VAL(USB_OTG_GRXSTSP_BCNT, receive_status);
	// The status of the received packet
	uint8_t pktsts = _FLD2VAL(USB_OTG_GRXSTSP_PKTSTS, receive_status);

	switch (pktsts)
	{
		case 0x06: // SETUP packet (includes data)
			//TODO
			break;
		case 0x02:
			//TODO
			break; // OUT packet (includes data)

	}
}

/**
 * Handle the USB core interrupts
 */
void gintsts_handler()
{
	volatile uint32_t gintsts = USB_OTG_HS_GLOBAL->GINTSTS;

	if (gintsts & USB_OTG_GINTSTS_USBRST) {
		usbrst_handler();
		// Clear the interrupt
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_USBRST);
	} else if (gintsts & USB_OTG_GINTSTS_ENUMDNE) {
		enumdne_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_ENUMDNE);
	} else if (gintsts & USB_OTG_GINTSTS_RXFLVL) {
		rxflvl_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_RXFLVL);
	} else if (gintsts & USB_OTG_GINTSTS_IEPINT) {

	} else if (gintsts & USB_OTG_GINTSTS_OEPINT) {

	}
}

