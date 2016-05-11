/**
 * @author Simone Romano - s.romano1992@gmail.com
 */
#include "ch.h"
#include "hal.h"
#include "test.h"
#include "chprintf.h"

#define EOF '\377'
#define WIFI_SERIAL &SD1
#define MONITOR_SERIAL &SD2
#define MAX_LENGTH  100 //lunghezza max messaggi trasmessi dal modulo WiFi, si potrebbe ridurre
#define TIME_IMMEDIATE  ((systime_t)0)
#define MSG_TIMEOUT (msg_t)-1
#define Q_TIMEOUT   MSG_TIMEOUT
#define COMMAND_SLEEP 500
#define COMMAND_LONG_SLEEP 20000

char * readResponse(void);
void printWebPage(void);
int mystrcontains(char* text, char* toFind);
void sendToESP8266(char* command, int delay);
void readAndPrintResponse(void);
int mystrlen(char* text);
static void println(char *p);
void blinkBoardLed(void);

static SerialConfig uartCfgWiFi = {115200, // bit rate
    };

static char* ESP8266_HELLO = "AT\r\n";
static char* ESP8266_RESET = "AT+RST\r\n";
static char* ESP8266_LIST_WIFI = "AT+CWLAP\r\n";
static char* ESP8266_CONNECT_TO_WIFI =
    "AT+CWJAP=\"Romano Wi-Fi\",\"160462160867\"\r\n";
static char* ESP8266_CHECK_IP = "AT+CIFSR\r\n";
static char* ESP8266_GET_IP_ADD = "AT+CIFSR\r\n"; // get ip address
static char* ESP8266_CHECK_VERSION = "AT+GMR\r\n";
static char* ESP8266_MULTIPLE_CONNECTION = "AT+CIPMUX=1\r\n";
static char* ESP8266_START_SERVER = "AT+CIPSERVER=1,80\r\n";
static char* ESP8266_SET_AS_ACCESS_POINT = "AT+CWMODE=2\r\n"; // configura come access point
static char* ESP8266_SET_AS_CLIENT = "AT+CWMODE=1\r\n";
static char* ESP8266_SEND_TCP_DATA = "AT+CIPSEND=";
static char* ESP8266_CLOSE_CONN = "AT+CIPCLOSE=";
static char FIND_PLUS = 0, FIND_I = 0, FIND_P = 0, FIND_D = 0; //+IPD,0,89:GET /?pos=10.1,20.3 HTTP/1.1 User-Agent: curl/7.39.0 Host: 192.168.4.1Accept: */*
static char x[3], y[3];
static THD_WORKING_AREA(waThread1, 128);
/**
 * Asynchronous serial SD1
 */
static msg_t Uart1EVT_Thread(void *p) {
  int letterAfterPlus = 0;
  int spaceAfterD = 0;
  int x_charRead = 0, y_charRead = 0;
  event_listener_t el1;
  eventflags_t flags;

  chEvtRegisterMask(chnGetEventSource(WIFI_SERIAL), &el1, 1);
  while (TRUE) {
    chEvtWaitOne(1);

    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();  //wait for events;

    if (flags & CHN_INPUT_AVAILABLE) {  //events received
      msg_t charbuf;
      do {
        charbuf = chnGetTimeout(WIFI_SERIAL, TIME_IMMEDIATE);
        if (charbuf != Q_TIMEOUT) {
          //check if +IPD command in
          if (charbuf == '+') {
            FIND_PLUS = 1;
            letterAfterPlus = 1;
          }
          if (charbuf == 'I') {
            if (letterAfterPlus == 1) {
              FIND_I = 1;
              letterAfterPlus++;
            }
            else {
              FIND_PLUS = 0;
              letterAfterPlus = 0;
            }
          }
          if (charbuf == 'P') {
            if (letterAfterPlus == 2) {
              FIND_P = 1;
              letterAfterPlus++;
            }
            else {
              FIND_I = 0;
              FIND_PLUS = 0;
              letterAfterPlus = 0;
            }
          }
          if (charbuf == 'D') {
            if (letterAfterPlus == 3) {
              FIND_D = 1;
              letterAfterPlus++;
            }
          }
          if (FIND_D == 1) {
            if (spaceAfterD >= 15 && x_charRead < 2) {
              //we are after pos=: expected 10,20 (example)
              x[x_charRead] = charbuf;
              x_charRead++;
            }
            else if (spaceAfterD >= 15 && x_charRead == 2 && y_charRead < 2) {
              x[x_charRead] = '\0';
              y[y_charRead] = charbuf;
              y_charRead++;
            }
            else if (spaceAfterD >= 15 && x_charRead == 2 && y_charRead == 2) {
              y[y_charRead] = '\0';
              letterAfterPlus = 0;
              FIND_PLUS = 0;
              FIND_I = 0;
              FIND_P = 0;
              FIND_D = 0;
              x_charRead = 0;
              y_charRead = 0;
              spaceAfterD = 0;
              /*chprintf((BaseChannel *)MONITOR_SERIAL, x, 3);
               chprintf((BaseChannel *)MONITOR_SERIAL, y, 3);*/
              blinkBoardLed();
              printWebPage();
            }
            else {
              spaceAfterD++;
              letterAfterPlus++;
            }
          }
          else {
            FIND_PLUS = 0;
            FIND_I = 0;
            FIND_P = 0;
            FIND_D = 0;
            x_charRead = 0;
            y_charRead = 0;
            spaceAfterD = 0;
          }
          chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%c", (char)charbuf);
        }
      } while (charbuf != Q_TIMEOUT );
    }
  }

  //println((char)flags);
  /*char c = chSequentialStreamGet(WIFI_SERIAL);
   chprintf((BaseSequentialStream *) MONITOR_SERIAL, "%c", c);*/
  //readAndPrintResponse();
}

void blinkBoardLed() {
  palSetPad(GPIOA, GPIOA_LED_GREEN);
  chThdSleepMilliseconds(500);
  palClearPad(GPIOA, GPIOA_LED_GREEN);
}

/**
 * This function set the ESP8266 (connected to serial SD1)
 * as access point and start a server on port 80.
 * It will be ready to accept http request.
 * NOTE: first to call this function, be sure that:
 *  -you started serial SD2 to read response and debug
 *  -you started serial SD1 to send request to ESP8266
 * The code:
 * sdStart(MONITOR_SERIAL, &uartCfgMonitor);
 * palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
 * palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
 * sdStart(WIFI_SERIAL, &uartCfgWiFi);
 * ESP8266_setAsAP();
 */
void ESP8266_setAsAP(void) {
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Uart1EVT_Thread,
                    NULL);
  sendToESP8266(ESP8266_RESET, COMMAND_SLEEP);
  sendToESP8266(ESP8266_SET_AS_ACCESS_POINT, COMMAND_SLEEP);
  sendToESP8266(ESP8266_GET_IP_ADD, COMMAND_SLEEP);
  sendToESP8266(ESP8266_MULTIPLE_CONNECTION, COMMAND_SLEEP);
  sendToESP8266(ESP8266_START_SERVER, COMMAND_SLEEP);
}

/**
 * This function set the ESP8266 (connected to serial SD1)
 * as a client to the wifi connection defined in command
 * ESP8266_CONNECT_TO_WIFI and start a server on port 80.
 * It will be ready to accept http request.
 * NOTE: first to call this function, be sure that:
 *  -you started serial SD2 to read response and debug
 *  -you started serial SD1 to send request to ESP8266
 * The code:
 * sdStart(MONITOR_SERIAL, &uartCfgMonitor);
 * palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7));
 * palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7));
 * sdStart(WIFI_SERIAL, &uartCfgWiFi);
 * ESP8266_setAsClient();
 */
void ESP8266_setAsClient(void) {
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Uart1EVT_Thread,
                    NULL);
  sendToESP8266(ESP8266_RESET, COMMAND_SLEEP);
  sendToESP8266(ESP8266_SET_AS_CLIENT, COMMAND_LONG_SLEEP);
  sendToESP8266(ESP8266_MULTIPLE_CONNECTION, COMMAND_LONG_SLEEP);
  sendToESP8266(ESP8266_LIST_WIFI, COMMAND_LONG_SLEEP);
  sendToESP8266(ESP8266_CONNECT_TO_WIFI, COMMAND_LONG_SLEEP);
  sendToESP8266(ESP8266_START_SERVER, COMMAND_LONG_SLEEP);
  sendToESP8266(ESP8266_CHECK_IP, COMMAND_LONG_SLEEP);
}

int mystrlen(char* text) {
  int length = 0;
  while (true) {
    if (text[length] == '\0')
      return length;
    length++;
  }
}

/**
 * This function send a command on serial port SD1 to
 * ESP8266. You can listen for the response by calling
 * readAndPrintResponse() function.
 */
void sendToESP8266(char* command, int delay) {
  chprintf((BaseChannel *)WIFI_SERIAL, command);
  chThdSleepMilliseconds(delay);
}

/**
 * This method reads input from SD1 and print on Serial usb
 * monitor (SD2).
 */
void readAndPrintResponse() {
  char buff[1];
  int pos = 0;
  char charbuf;
  while (true) {
    charbuf = chnGetTimeout(WIFI_SERIAL, TIME_IMMEDIATE);
    if (charbuf == EOF) {
      break;
    }
    buff[0] = charbuf;
    chprintf((BaseChannel *)MONITOR_SERIAL, buff, 1);
  }
  buff[0] = '\0';
  chprintf((BaseChannel *)MONITOR_SERIAL, '\0', 1);
}

void printWebPage() {
  //char webpage[] = "<h1>Hello</h1><button>LED1</button>\r\n";
  char webpage[] = "<h1>Hello</h1>";
  char cipSend[] = "AT+CIPSEND=0,14\r\n"; //"AT+CIPSEND=52\r\n";
  char cipClose[] = "AT+CIPCLOSE=0\r\n";
  sendToESP8266(cipSend, COMMAND_SLEEP);
  sendToESP8266(webpage, COMMAND_SLEEP);
  //sendToESP8266(cipClose, COMMAND_SLEEP);
}

int mystrcontains(char* text, char* toFind) {
  int i = 0;
  int j = 0;
  while (text[i] != '\0' && toFind[j] != '\0') {
    if (i > j && j != 0) {
      break;
    }
    if (text[i] == toFind[j]) {
      j++;
      if (j == mystrlen(toFind))
        return 1;
    }
    i++;
  }
  return 0;
}

static void println(char *p) {

  while (*p) {
    chSequentialStreamPut(MONITOR_SERIAL, *p++);
  }
  chSequentialStreamWrite(MONITOR_SERIAL, (uint8_t * )"\r\n", 2);
}

