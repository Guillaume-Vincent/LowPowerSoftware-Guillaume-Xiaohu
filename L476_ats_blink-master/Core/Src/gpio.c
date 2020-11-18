#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_gpio.h"
#include "gpio.h"

#define LED_PORT GPIOA
#define LED_PIN LL_GPIO_PIN_5
#define BUT_PORT GPIOC
#define BUT_PIN LL_GPIO_PIN_13
#define SQUARE_PORT GPIOC
#define SQUARE_PIN LL_GPIO_PIN_10


void GPIO_init(void) {
	// PORT A Clock Enable
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
	// Green LED (user LED) - PA5
	LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT );
	LL_GPIO_SetPinOutputType(LED_PORT, LED_PIN, LL_GPIO_OUTPUT_PUSHPULL);

	// PORT C Clock Enable
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
	// Blue button - PC13
	LL_GPIO_SetPinMode(BUT_PORT, BUT_PIN, LL_GPIO_MODE_INPUT);
	// Square Signal - PC10
	LL_GPIO_SetPinMode(SQUARE_PORT, SQUARE_PIN, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType(SQUARE_PORT, SQUARE_PIN, LL_GPIO_OUTPUT_PUSHPULL);
}

void LED_GREEN(int val) {
	if (val)
		LL_GPIO_SetOutputPin(LED_PORT, LED_PIN);
	else
		LL_GPIO_ResetOutputPin(LED_PORT, LED_PIN);
}

void SQUARE_SIGNAL(int val) {
	if (val)
		LL_GPIO_SetOutputPin(SQUARE_PORT, SQUARE_PIN);
	else
		LL_GPIO_ResetOutputPin(SQUARE_PORT, SQUARE_PIN);
}


int BLUE_BUTTON() {
	return(!LL_GPIO_IsInputPinSet(BUT_PORT, BUT_PIN));
}
