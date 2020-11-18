#ifndef SRC_WAKEUP_H_
#define SRC_WAKEUP_H_


#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_pwr.h"


void RTC_Config();
void RTC_wakeup_init_from_standby_or_shutdown(int delay);
void RTC_wakeup_init_from_stop(int delay);
void RTC_WKUP_IRQHandler();


#endif /* SRC_WAKEUP_H_ */
