#include "wakeup.h"


void RTC_Config() {
	LL_RTC_DisableWriteProtection(RTC);
	LL_RTC_EnableInitMode(RTC);

	while (!LL_RTC_IsActiveFlag_INIT(RTC)) {};

	LL_RTC_SetAsynchPrescaler(RTC, 127);
	LL_RTC_SetSynchPrescaler(RTC, 255);

	LL_RTC_TIME_Config(RTC, LL_RTC_TIME_FORMAT_AM_OR_24, 12, 0, 0);
	LL_RTC_DATE_Config(RTC, LL_RTC_WEEKDAY_TUESDAY, 17, LL_RTC_MONTH_NOVEMBER, 2020);

	LL_RTC_DisableInitMode(RTC);
	LL_RTC_EnableWriteProtection(RTC);
}

// partie commune a toutes les utilisations du wakeup timer
static void RTC_wakeup_init(int delay) {
	LL_RTC_DisableWriteProtection(RTC);
	LL_RTC_WAKEUP_Disable(RTC);
	while (!LL_RTC_IsActiveFlag_WUTW(RTC)) {};

	// connecter le timer a l'horloge 1Hz de la RTC
	LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_CKSPRE);

	// fixer la duree de temporisation
	LL_RTC_WAKEUP_SetAutoReload(RTC, delay);	// 16 bits
	LL_RTC_ClearFlag_WUT(RTC);
	LL_RTC_EnableIT_WUT(RTC);
	LL_RTC_WAKEUP_Enable(RTC);
	LL_RTC_EnableWriteProtection(RTC);
}

// Dans le cas des modes STANDBY et SHUTDOWN, le MPU sera reveille par reset
// causé par 1 wakeup line (interne ou externe) (le NVIC n'est plus alimenté)
void RTC_wakeup_init_from_standby_or_shutdown(int delay) {
	RTC_wakeup_init(delay);

	// enable the Internal Wake-up line
	LL_PWR_EnableInternWU();	// ceci ne concerne que Standby et Shutdown, pas STOPx
}

// Dans le cas des modes STOPx, le MPU sera reveille par interruption
// le module EXTI et une partie du NVIC sont encore alimentes
// le contenu de la RAM et des registres étant préservé, le MPU
// reprend l'execution après l'instruction WFI
void RTC_wakeup_init_from_stop(int delay) {
	RTC_wakeup_init(delay);

	// valider l'interrupt par la ligne 20 du module EXTI, qui est réservée au wakeup timer
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);

	// valider l'interrupt chez NVIC
	NVIC_SetPriority(RTC_WKUP_IRQn, 1);
	NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

// wakeup timer interrupt Handler (inutile mais doit etre defini)
void RTC_WKUP_IRQHandler() {
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
}
