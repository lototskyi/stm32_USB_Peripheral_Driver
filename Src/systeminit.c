/*
 * systeminit.c
 *
 *  Created on: Jun 13, 2025
 *      Author: olexandr
 */
#include <stdint.h>
#include "system_stm32f4xx.h"
#include "stm32f4xx.h"
#include "Helpers/logger.h"

LogLevel system_log_level = LOG_LEVEL_DEBUG;
uint32_t SystemCoreClock = 72000000; // 72 MHz

// HCLK  = 72 MHz
// PLL: M = 4, N = 72, P = 2, Q = 3
// AHB prescaler  = 1
// APB prescaler1 = 2, APB prescaler2 = 1
// MCO1 prescaler = 2

static void configure_clock()
{
	// Configure flash latency
	MODIFY_REG(FLASH->ACR,
			FLASH_ACR_LATENCY,
			//FLASH_ACR_LATENCY_2WS << FLASH_ACR_LATENCY_Pos
			_VAL2FLD(FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2WS)
	);

	// Enable HSE
	SET_BIT(RCC->CR, RCC_CR_HSEON);

	// wait until HSE is stable
	while(!READ_BIT(RCC->CR, RCC_CR_HSERDY));

	// Configure PLL: source = HSE, SYSCLK = 72 MHz
	MODIFY_REG(RCC->PLLCFGR,
			RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLP,
			_VAL2FLD(RCC_PLLCFGR_PLLM, 4) | _VAL2FLD(RCC_PLLCFGR_PLLN, 72) | _VAL2FLD(RCC_PLLCFGR_PLLQ, 3) | _VAL2FLD(RCC_PLLCFGR_PLLSRC, 1)
	);

	// Enable PLL
	SET_BIT(RCC->CR, RCC_CR_PLLON);

	// Wait until PLL is table
	while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY));

	// Switch system clock to PLL
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, _VAL2FLD(RCC_CFGR_SW, RCC_CFGR_SW_PLL));

	// Configure the PPRE1
	MODIFY_REG(RCC->CFGR,
			RCC_CFGR_PPRE1,
			_VAL2FLD(RCC_CFGR_PPRE1, 4)
	);

	// Wait until PLL is used
	while(READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

	// Disable HSI
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);
}

void configure_mco1()
{
	// Configure MCO1: source = PLL, MCO1PRE = 2;
	MODIFY_REG(RCC->CFGR,
		RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE,
		_VAL2FLD(RCC_CFGR_MCO1, 3) | _VAL2FLD(RCC_CFGR_MCO1PRE, 4)
	);

	// Enable GPIOA (MCO1 is connected to PA8)
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

	// Configure PA8 as medium speed
	MODIFY_REG(GPIOA->OSPEEDR,
		GPIO_OSPEEDR_OSPEED8,
		_VAL2FLD(GPIO_OSPEEDR_OSPEED8, 1)
	);

	// Configure PA8 to work in alternate function mode
	MODIFY_REG(GPIOA->MODER,
		GPIO_MODER_MODER8,
		_VAL2FLD(GPIO_MODER_MODER8, 2)
	);
}

void SystemInit(void)
{
	//configure_mco1();
	configure_clock();
}
