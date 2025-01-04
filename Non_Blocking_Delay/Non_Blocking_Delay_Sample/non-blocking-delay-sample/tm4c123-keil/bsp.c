/*----------------------------------------------------------------------------------*/
/* Copyright (C) 2025 Sigma eLabs, <https:/github.com/Sigma-eLabs>.                 */
/*                                                                                  */
/*                               S i g m a  e L a b s                               */
/*                    -------------------------------------------                   */
/*                    Embedded Software Education & Research Labs                   */
/*                                                                                  */
/* This is an opened source software for studying and researching purposes with     */
/* a non-comercial license.                                                         */
/*                                                                                  */
/* However, redistributions in source code must retain this top-level               */
/* comment block, to respect the contributions of the Sigma eLabs's authors.        */
/*                                                                                  */
/* Contact information:                                                             */
/*  <thienaiho95@gmail.com>                                                         */
/*  <https:/www.linkedin.com/in/thien-ai-ho>                                        */
/*----------------------------------------------------------------------------------*/

/* Board Support Package implementation for EK-TM4C123GXL */

#include "bsp.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

static uint32_t volatile l_tickCounter;

void BSP_init(void) {
	/* configuration to enable GPIOF module */
	SYSCTL->RCGCGPIO  |= (1U << 5);	/* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5);	/* enable AHB bus for GPIOF */
	
	/* even PF2, PF3 pins unused, but explicitly configured them as output pins to prevent unwanted behavior */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN); /* configure PF1, PF2, PF3 pins - red, blue, green LEDs as output pin */
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN); /* configure PF1, PF2, PF3 pins - red, blue, green LEDs with digital mode */
	
	/* configure User button switch 1 - PF4 pin as digital input pin with pull-up resistor */
	GPIOF_AHB->DIR &= ~USR_SW1;	/* configure PF4 pin - red LED as input pin */
    GPIOF_AHB->DEN |= USR_SW1;	/* configure PF4 pin - red LED with digital mode */
	GPIOF_AHB->PUR |= USR_SW1;	/* confgiure PF4 pin with pull-up resistor */
	
	/* Interrupt setup for User button switch 1 - PF4 pin: falling edge interrupt detection */
	GPIOF_AHB->IS &= ~USR_SW1;	/* edge-sensitive interrupt detection */
	GPIOF_AHB->IBE &= ~USR_SW1;	/* interrupt event is controller by GPIOIEV */
	GPIOF_AHB->IEV &= ~USR_SW1;	/* falling edge interrupt trigger */
	GPIOF_AHB->IM |= USR_SW1;	/* enable GPIOF interrupt for this pin */
	
	/* interrupt set up for System Tick */
	SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); /* by this setting, the Sytem Tick will occured 100 (100Hz) times every second */
    NVIC_SetPriority(SysTick_IRQn, 0U); /* set the SysTick interrupt priority (highest) */
	
	/* interrupt control setup for SW1 button pressed */
	NVIC_SetPriority(GPIOF_IRQn, 1U); /* set interrupt priority for GPIOF */
	NVIC_EnableIRQ(GPIOF_IRQn); 	/* enable GPIOF_IRQn in NVIC */
}

void BSP_setRedledState(LedState ledState) {
	if (ledState == LED_ON) {
		GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
	}
	else {
		GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
	}
}

void BSP_setBlueledState(LedState ledState) {
	if (ledState == LED_ON) {
		GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
	}
	else {
		GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
	}
}

void BSP_setGreenledState(LedState ledState) {
	if (ledState == LED_ON) {
		GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
	}
	else {
		GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
	}
}

void SysTick_Handler(void) {
    ++l_tickCounter;
}

uint32_t BSP_getTickCounter(void) {
	uint32_t tickCounter;
	
	/* enter critical context here is to prevent
	interrupt occurs and modifies the value of l_tickCounter */
	__disable_irq();
	tickCounter = l_tickCounter;
	__enable_irq();
	
	return tickCounter;
}

/* interrupt function is invoked when GPIOF interrupt happened */
void GPIOPortF_IRQHandler(void) {
	if ((GPIOF_AHB->RIS & USR_SW1) != 0U) { /* check if the interrupt caused by User button switch 1 - PF4 pin pressed */
		blinkyJobStat = JOB_REQUESTED;	/* blinky job requested */
	}
	
	GPIOF_AHB->ICR = USR_SW1;	/* clear the interrupt source caused by User button switch 1, otherwise it is remained forever */
}

void assert_failed(char const * const module, int const id) {
    (void)module; /* unused parameter */
    (void)id;     /* unused parameter */
    NVIC_SystemReset();
}