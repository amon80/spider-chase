#include <stdlib.h>     /* atoi */

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

void parse_string(char *command, int *velocity){
	char left[3];
	char right[3];
	int toret[2];
	char type = command[0];

	left[0] = command[2];
	left[1] = command[3];
	left[2] = command[4];

	right[0] = command[5];
	right[1] = command[6];
	right[2] = command[7];

	int le,ri;

	le = atoi(left);
	ri = atoi(right);

	le-=127;
	ri-=127;

	velocity[0]=le;
	velocity[1]=ri;

}

void init_motor(){
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

	pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 5000));
	pwmEnableChannelNotification(&PWMD1, 0);
	//chThdSleepMilliseconds(1000);

	//start pwm2 (right engine)
	pwmStart(&PWMD3, &pwm2cfg);
	pwmEnablePeriodicNotification(&PWMD3);
	pwmEnableChannelNotification(&PWMD3, 0);

	pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 5000));
	pwmEnableChannelNotification(&PWMD3, 0);
	chThdSleepMilliseconds(1000);
}

void control_motor(char* command){

	int velocity[2];
	parse_string(command, velocity);
	int pwm1=1,pwm2=1;

	if(velocity[0]>=0){
		functioPtrLeftUP = &Sinistra_Avanti_up;
		functioPtrLeftDOWN = &Sinistra_Avanti_Down;

		if (velocity[0] == 0){
			pwm1 = 10000;
		}else{
			if (velocity[0]==128){
				pwm1 = 100;
			}else{
				pwm1 = 10000-(int)((10000*velocity[0])/128);
			}
		}

	    pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, pwm1));

	}else{
		functioPtrLeftUP = &Sinistra_Dietro_up;
		functioPtrLeftDOWN = &Sinistra_Dietro_Down;


		pwm1 = 10000-(int)((10000*-velocity[0])/127);

		pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, pwm1));
		pwmEnableChannelNotification(&PWMD1, 0);
	}

	if(velocity[1]>=0){
		functioPtrRightUP = &Destra_Avanti_up;
		functioPtrRightDOWN = &Destra_Avanti_Down;

		if (velocity[0] == 0){
			pwm2 = 10000;
		}else{
			if (velocity[0]==128){
				pwm2 = 100;
			}else{
				pwm2 = 10000-(int)((10000*velocity[0])/128);
			}
		}

		pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, pwm2));
		pwmEnableChannelNotification(&PWMD3, 0);

	}else{
		functioPtrRightUP = &Destra_Dietro_up;
		functioPtrRightDOWN = &Destra_Dietro_Down;

		pwm2 = 10000 - (int)((10000*-velocity[1])/127);

		pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, pwm2));
		pwmEnableChannelNotification(&PWMD3, 0);
	}

}
