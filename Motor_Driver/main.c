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

void (*functioPtrLeftUP)();
void (*functioPtrLeftDOWN)();
void (*functioPtrRightUP)();
void (*functioPtrRightDOWN)();

//1 positivo motore sinistro
//2 negativo motore sinistro

//3 positivo motore destro
//4 negativo motore destro

static struct Mapping_GPIO{
	stm32_gpio_t * type1;
	unsigned int port1;

	stm32_gpio_t * type2;
	unsigned int port2;

	stm32_gpio_t * type3;
	unsigned int port3;

	stm32_gpio_t * type4;
	unsigned int port4;
}mapping;

static void Sinistra_Avanti_up(){
	palSetPad(mapping.type1, mapping.port1);
}

static void Sinistra_Avanti_Down(){
	palClearPad(mapping.type1, mapping.port1);
}

static void Destra_Avanti_up(){
	palSetPad(mapping.type3, mapping.port3);
}

static void Destra_Avanti_Down(){
	palClearPad(mapping.type3, mapping.port3);
}

static void Sinistra_Dietro_up(){
	palSetPad(mapping.type2, mapping.port2);
}

static void Sinistra_Dietro_Down(){
	palClearPad(mapping.type2, mapping.port2);
}

static void Destra_Dietro_up(){
	palSetPad(mapping.type4, mapping.port4);
}

static void Destra_Dietro_Down(){
	palClearPad(mapping.type4, mapping.port4);
}


//pwm callbacks for left engine
static void pwmpcb(PWMDriver *pwmp) {

  (void)pwmp;
  (*functioPtrLeftDOWN)();
}

static void pwmc1cb(PWMDriver *pwmp) {

  (void)pwmp;
  (*functioPtrLeftUP)();
}


//pwm callbacks for right engine
static void pwm2pcb(PWMDriver *pwmp) {

  (void)pwmp;
  (*functioPtrRightDOWN)();
}

static void pwm2c1cb(PWMDriver *pwmp) {

  (void)pwmp;
  (*functioPtrRightUP)();
}


//configuration for left engine
static PWMConfig pwm1cfg = {
  10000,                                    /* 10kHz PWM clock frequency.   */
  500,                                    /* Initial PWM period 1S.       */
  pwmpcb,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};

//configuration for right engine
static PWMConfig pwm2cfg = {
  10000,                                    /* 10kHz PWM clock frequency.   */
  500,                                    /* Initial PWM period 1S.       */
  pwm2pcb,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, pwm2c1cb},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};


int main(void) {

  halInit();
  chSysInit();

  mapping.type1 = GPIOA;
  mapping.port1 = GPIOA_PIN8;

  mapping.type2 = GPIOB;
  mapping.port2 = GPIOB_PIN10;

  mapping.type3 = GPIOB;
  mapping.port3 = GPIOB_PIN4;

  mapping.type4 = GPIOB;
  mapping.port4 = GPIOB_PIN5;

  palSetPadMode(mapping.type1, mapping.port1, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type1, mapping.port1);
  palSetPadMode(mapping.type2, mapping.port2, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type2, mapping.port2);
  palSetPadMode(mapping.type3, mapping.port3 , PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type3, mapping.port3);
  palSetPadMode(mapping.type4, mapping.port4 , PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type4, mapping.port4);


  functioPtrLeftUP = &Sinistra_Avanti_up;
  functioPtrLeftDOWN = &Sinistra_Avanti_Down;

  functioPtrRightUP = &Destra_Avanti_up;
  functioPtrRightDOWN = &Destra_Avanti_Down;

  /*
   * Activates the serial driver 2 using the driver default configuration.
  */
  sdStart(&SD2, NULL);

  //start pwm1 (left engine)
  pwmStart(&PWMD1, &pwm1cfg);
  pwmEnablePeriodicNotification(&PWMD1);
  pwmEnableChannelNotification(&PWMD1, 0);

  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 100));
  pwmEnableChannelNotification(&PWMD1, 0);
  //chThdSleepMilliseconds(1000);

  //start pwm2 (right engine)
  pwmStart(&PWMD3, &pwm2cfg);
  pwmEnablePeriodicNotification(&PWMD3);
  pwmEnableChannelNotification(&PWMD3, 0);

  pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 50));
  pwmEnableChannelNotification(&PWMD3, 0);
  chThdSleepMilliseconds(1000);

  int duty = 1;

  while(1){
	  /*
	  chThdSleepMilliseconds(5000);
	  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 7500));
	  chThdSleepMilliseconds(5000);
	  */
  }
  /*
  int val = 1;

  while(1){
	  if (val){
		  //SPEGNENDO
		  while(duty!=10000){
			  duty += step;
			  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, duty));
			  chThdSleepMilliseconds(200);
		  }
		  functioPtrUP = &Dietro_Up;
		  functioPtrDOWN = &Dietro_Down;
		  //ACCENDENDO
		  while(duty!=1000){
			  duty -= step;
			  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, duty));
			  chThdSleepMilliseconds(200);
		  }
		  //chThdSleepMilliseconds(1000);
		  val = 0;
	  }else{
		  //SPEGNENDO
		  while(duty!=10000){
			  duty += step;
			  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, duty));
			  chThdSleepMilliseconds(200);
		  }
		  Dietro_Down();
		  functioPtrUP = &Avanti_Up;
		  functioPtrDOWN = &Avanti_Down;
		  //ACCENDO
		  while(duty!=1000){
			  duty -= step;
			  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, duty));
			  chThdSleepMilliseconds(200);
		  }
		  //chThdSleepMilliseconds(1000);
		  val = 1;
	  }
	  //
  }
  */


}
