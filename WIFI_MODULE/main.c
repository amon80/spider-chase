/*
 ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "chprintf.h"
#include "ESP8266.h"


static SerialConfig uartCfgMonitor = {
    9600,
};

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(MONITOR_SERIAL, &uartCfgMonitor);
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // USART1 TX.
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // USART1 RX.
  sdStart(WIFI_SERIAL, &uartCfgWiFi);
  ESP8266_setAsAP();

  while(true){
    chThdSleepMilliseconds(100);    //without this, serial events will not be received (priority issue, maybe)
  }
}
