void (*functioPtrLeftUP)();
void (*functioPtrLeftDOWN)();
void (*functioPtrRightUP)();
void (*functioPtrRightDOWN)();

//1 positivo motore sinistro
//2 negativo motore sinistro

//3 positivo motore destro
//4 negativo motore destro

static struct Mapping_GPIO {
  stm32_gpio_t * type1;
  unsigned int port1;

  stm32_gpio_t * type2;
  unsigned int port2;

  stm32_gpio_t * type3;
  unsigned int port3;

  stm32_gpio_t * type4;
  unsigned int port4;
} mapping;

static void Sinistra_Avanti_up() {
  palClearPad(mapping.type2, mapping.port2);
  palSetPad(mapping.type1, mapping.port1);
}

static void Sinistra_Avanti_Down() {
  palClearPad(mapping.type2, mapping.port2);
  palClearPad(mapping.type1, mapping.port1);
}

static void Sinistra_Dietro_up() {
  palClearPad(mapping.type1, mapping.port1);
  palSetPad(mapping.type2, mapping.port2);
}

static void Sinistra_Dietro_Down() {
  palClearPad(mapping.type1, mapping.port1);
  palClearPad(mapping.type2, mapping.port2);
}

static void Destra_Avanti_up() {
  palSetPad(mapping.type3, mapping.port3);
  palClearPad(mapping.type4, mapping.port4);
}

static void Destra_Avanti_Down() {
  palClearPad(mapping.type3, mapping.port3);
  palClearPad(mapping.type4, mapping.port4);
}

static void Destra_Dietro_up() {
  palClearPad(mapping.type3, mapping.port3);
  palSetPad(mapping.type4, mapping.port4);
}

static void Destra_Dietro_Down() {
  palClearPad(mapping.type3, mapping.port3);
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
static PWMConfig pwm1cfg = {10000, /* 10kHz PWM clock frequency.   */
                            500, /* Initial PWM period 1S.       */
                            pwmpcb, { {PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb}, {
                                PWM_OUTPUT_DISABLED, NULL},
                                     {PWM_OUTPUT_DISABLED, NULL}, {
                                         PWM_OUTPUT_DISABLED, NULL}},
                            0, 0};

//configuration for right engine
static PWMConfig pwm2cfg = {10000, /* 10kHz PWM clock frequency.   */
                            500, /* Initial PWM period 1S.       */
                            pwm2pcb, { {PWM_OUTPUT_ACTIVE_HIGH, pwm2c1cb}, {
                                PWM_OUTPUT_DISABLED, NULL},
                                      {PWM_OUTPUT_DISABLED, NULL}, {
                                          PWM_OUTPUT_DISABLED, NULL}},
                            0, 0};

void parse_string(char *command, int *velocity) {
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

  int le, ri;

  le = atoi(left);
  ri = atoi(right);

  velocity[0] = le;
  velocity[1] = ri;

}

void init_motor() {
  mapping.type1 = GPIOA;
  mapping.port1 = GPIOA_PIN8;

  mapping.type2 = GPIOB;
  mapping.port2 = GPIOB_PIN10;

  mapping.type3 = GPIOB;
  mapping.port3 = GPIOB_PIN4;

  mapping.type4 = GPIOB;
  mapping.port4 = GPIOB_PIN5;

  palSetPadMode(mapping.type1, mapping.port1,
                PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type1, mapping.port1);
  palSetPadMode(mapping.type2, mapping.port2,
                PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type2, mapping.port2);
  palSetPadMode(mapping.type3, mapping.port3,
                PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type3, mapping.port3);
  palSetPadMode(mapping.type4, mapping.port4,
                PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(mapping.type4, mapping.port4);
  functioPtrLeftUP = &Sinistra_Avanti_up;
  functioPtrLeftDOWN = &Sinistra_Avanti_Down;

  functioPtrRightUP = &Destra_Avanti_up;
  functioPtrRightDOWN = &Destra_Avanti_Down;

  //start pwm1 (left engine)
  pwmStart(&PWMD1, &pwm1cfg);
  pwmEnablePeriodicNotification(&PWMD1);
  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 5000));
  pwmEnableChannelNotification(&PWMD1, 0);

  //start pwm2 (right engine)
  pwmStart(&PWMD3, &pwm2cfg);
  pwmEnablePeriodicNotification(&PWMD3);
  pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 5000));
  pwmEnableChannelNotification(&PWMD3, 0);
}

void control_motor(char* command) {

  int velocity[2];
  parse_string(command, velocity);
  int pwm1 = 1, pwm2 = 1;

  /***********************DEMO************************/
  //PIN D7-D6
  /*int vel = 0;
  functioPtrLeftUP = &Sinistra_Avanti_up;
  functioPtrLeftDOWN = &Sinistra_Avanti_Down;
  while (vel < 127) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD1, 0, pwm1);
    vel++;
    chThdSleepMilliseconds(10);
  }
  while (vel > 0) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD1, 0, pwm1);
    vel--;
    chThdSleepMilliseconds(10);
  }

  functioPtrLeftUP = &Sinistra_Dietro_up;
  functioPtrLeftDOWN = &Sinistra_Dietro_Down;
  while (vel < 127) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD1, 0, pwm1);
    vel++;
    chThdSleepMilliseconds(10);
  }
  while (vel > 0) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD1, 0, pwm1);
    vel--;
    chThdSleepMilliseconds(10);
  }

  //PIN D5-D4
  vel = 0;
  functioPtrRightUP = &Destra_Avanti_up;
  functioPtrRightDOWN = &Destra_Avanti_Down;
  while (vel < 127) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD3, 0, pwm1);
    vel++;
    chThdSleepMilliseconds(10);
  }
  while (vel > 0) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD3, 0, pwm1);
    vel--;
    chThdSleepMilliseconds(10);
  }

  functioPtrRightUP = &Destra_Dietro_up;
  functioPtrRightDOWN = &Destra_Dietro_Down;
  while (vel < 127) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD3, 0, pwm1);
    vel++;
    chThdSleepMilliseconds(10);
  }
  while (vel > 0) {
    pwm1 = 8 * vel + 1;
    chprintf(MONITOR_SERIAL, "%d - ", pwm1);
    pwmEnableChannel(&PWMD3, 0, pwm1);
    vel--;
    chThdSleepMilliseconds(10);
  }
  /*******************END DEMO************************/

  if (velocity[0] >= 128) {
    velocity[0] = velocity[0] - 128;    //[0-127]
    functioPtrLeftUP = &Sinistra_Avanti_up;
    functioPtrLeftDOWN = &Sinistra_Avanti_Down;

    pwm1 =  10000 - 77.95 * velocity[0] + 100;

    pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, pwm1));
  }
  else {    //<128
    functioPtrLeftUP = &Sinistra_Dietro_up;
    functioPtrLeftDOWN = &Sinistra_Dietro_Down;

    pwm1 = 10000 - 77.95 * velocity[0] + 100;

    pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, pwm1));
  }

  if (velocity[1] >= 128) {
    velocity[1] = velocity[1] - 128;    //[0-127]
    functioPtrRightUP = &Destra_Avanti_up;
    functioPtrRightDOWN = &Destra_Avanti_Down;

    pwm2 =  10000 - 77.95 * velocity[1] + 100;

    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, pwm2));
  }
  else {
    functioPtrRightUP = &Destra_Dietro_up;
    functioPtrRightDOWN = &Destra_Dietro_Down;

    pwm2 = 10000 - 77.95 * velocity[1] + 100;

    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, pwm2));
  }

}
