/* Project L476_ats_blink for STM32L476 mounted on Nucleo board:
 * the user LED (mounted on pin PA-5) is flashed every second for 50 ms.
 * The time base is provided by Systick (100 ticks per second).
 * The clock configuration is the default one (Sysclk = 80 MHz, derived from MSI and PLL).
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"
#include "wakeup.h"
#include "main.h"


volatile int systick_cnt = 0;
volatile int blue_mode = 0;
uint32_t expe;


// Systick interrupt handler
void SysTick_Handler() {
	// Checking if blue button is pressed
	if (BLUE_BUTTON())
		blue_mode = 1;

	// Green LED Management
	if (systick_cnt == 0)
		LED_GREEN(1);
	else if (systick_cnt == 5*expe)
		LED_GREEN(0);
	else if (systick_cnt == 199)
		systick_cnt = -1;
	systick_cnt ++;

	// Square Signal Management
	SQUARE_SIGNAL(systick_cnt % 2);
}


void SystemClock_Config();
void VoltageScaling_Config();
void SleepMode_Config();
void Calibration_Config();
void Systick_Config();



int main(void) {
	/* Configure GPIOs */
	GPIO_init();

    /* Configure system clock */
    SystemClock_Config();

    /* Configure voltage scaling */
    VoltageScaling_Config();

    /* Configure Systick timer (tick period at 10 ms) and activate interrupts */
    Systick_Config();

    /* Configure sleep modes*/
    SleepMode_Config();

    /* Configure calibration for expe [5..8]*/
    Calibration_Config();

    // Infinite Loop
    while (1) {
    	switch (expe) {
    	case 1: case 3:
    		if (blue_mode) __WFI();
    		break;

    	case 2: case 4:
    		if (blue_mode) LL_RCC_MSI_EnablePLLMode();
    		break;

    	case 5: case 6: case 7:
    		__WFI();
    		if (blue_mode) {
    			blue_mode = 0;
    			LL_LPM_EnableDeepSleep();
    			RTC_wakeup_init_from_stop(20);
    			__WFI();
    			LL_LPM_EnableSleep();
    			blue_mode = 0;
    		}
    		break;

    	case 8:
    		__WFI();
    		if (blue_mode) {
    			blue_mode = 0;
    			LL_LPM_EnableDeepSleep();
    			RTC_wakeup_init_from_standby_or_shutdown(20);
    			__WFI();
    			LL_LPM_EnableSleep();
    			blue_mode = 0;
    		}
    		break;

    	default:
    		break;
    	}
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config() {
	if (LL_RCC_LSE_IsReady()) { // Démarrage à chaud
		// Récupération de l'ancienne valeur de expe stockée dans le backup domain
		expe = LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0);

		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
		LL_PWR_EnableBkUpAccess();

		// Check si le blue button est appuyé au moment du RESET
		if (BLUE_BUTTON()) {
			// Incrémentation de la valeur de expe (ou remise à 1 si expe max)
			if (expe == 8)
				expe = 1;
			else
				expe ++;

			// Sauvegarde la nouvelle valeur de expe dans le backup domain

			LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, expe);

			// Attendre que le blue button soit relâché pour continuer
			while (BLUE_BUTTON()) {};
		}
	}
	else { // Démarrage à froid
		// Activation du LSE
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
		LL_PWR_EnableBkUpAccess();
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();
		LL_RCC_LSE_Enable();

		// Attendre que le LSE soit ready pour continuer
		while (!LL_RCC_LSE_IsReady()) {};

		/* Configure RTC */
		LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
		LL_RCC_EnableRTC();
		RTC_Config();

		// Initialisation de la valeur de expe
		expe = 1;

		// Sauvegarde de la valeur de expe dans le backup domain
		LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, expe);
	}

    /* MSI configuration and activation */
	if (expe == 1) { // MSI 4MHz  -  FLash Latency 4
		LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
		LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
	}
	else if (expe == 2) { // MSI 24MHz  -  FLash Latency 1
		LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
		LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
	}
	else { // MSI 24MHz  -  FLash Latency 3
		LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
		LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
	}

	// Configuration du MSI
	LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_Enable();
    while (LL_RCC_MSI_IsReady() != 1) {};

    if (expe == 1) {
		/* Main PLL configuration and activation */
		LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
		LL_RCC_PLL_Enable();
		LL_RCC_PLL_EnableDomain_SYS();
		while(LL_RCC_PLL_IsReady() != 1) {};

		/* Sysclk activation on the main PLL */
		LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
		while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {};
    }
    else {
    	/* Sysclk activation on the main PLL */
    	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);
    	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI) {};
    }

    /* Set APB1 & APB2 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Update the global variable called SystemCoreClock */
    SystemCoreClockUpdate();
}

void VoltageScaling_Config() {
	/* Configuration du voltage scaling selon expe */
	if (expe == 1 || expe == 2)
		LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
	else
		LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
}

void SleepMode_Config() {
	/* Configuration du sleep mode selon expe */
	switch (expe) {
	case 1:
	case 3:
		LL_LPM_EnableSleep();
		break;

	case 5:
		LL_LPM_EnableSleep();
		LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
		break;

	case 6:
		LL_LPM_EnableSleep();
		LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
		break;

	case 7:
		LL_LPM_EnableSleep();
		LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
		break;

	case 8:
		LL_LPM_EnableSleep();
		LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
		break;

	default:
		break;
	}
}

void Calibration_Config() {
	if (expe >= 5) LL_RCC_MSI_EnablePLLMode();
}

void Systick_Config() {
	/* Configuration du Systick */
	LL_InitTick(SystemCoreClock, 100U);
	LL_SYSTICK_EnableIT();
}

