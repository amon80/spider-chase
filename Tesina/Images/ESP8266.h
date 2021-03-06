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
#define MAX_LENGTH  100 //lunghezza max messaggi trasmessi dal 
//modulo WiFi, si potrebbe ridurre
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
char* StrStr(const char *str, const char *target);
char *strcat(char *dest, const char *src);
char *strcpy(char *dest, const char *src);
int strlen(const char * str);
void itoa(int n, char s[]);
void reverse(char s[]);

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
static char* ESP8266_SET_AS_ACCESS_POINT = "AT+CWMODE=2\r\n"; // configura 
														//come access point
static char* ESP8266_SET_AS_CLIENT = "AT+CWMODE=1\r\n";
static char* ESP8266_SEND_TCP_DATA = "AT+CIPSEND=";
static char* ESP8266_CLOSE_CONN = "AT+CIPCLOSE=";
char clientID[2];
char command[9];    //could be "M023120" for: motor left 
					//to 23 and motor right to 120
                    //else could be "PXXYY____" for position is xx yy
char request;
static THD_WORKING_AREA(waThread1, 2048);
/**
 * Asynchronous serial SD1
 */
static msg_t Uart1EVT_Thread(void *p) {
  int letterAfterPlus = 0;
  int spaceAfterD = 0;
  int x_charRead = 0, y_charRead = 0;
  int BUFF_SIZE = 1024;
  char received[BUFF_SIZE];
  int pos = 0;
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
        chThdSleepMilliseconds(1);
        if (charbuf != Q_TIMEOUT) {
          chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%c", (char)charbuf);
          if (pos < BUFF_SIZE) {
            received[pos] = (char)charbuf;
            pos++;
          }
        }
      } while (charbuf != Q_TIMEOUT );
      received[pos] = '\0';
      /***********DO SOMETHING WITH RECEIVED MESSAGE************/
      char* clearRequest = StrStr(received, "+IPD"); //HTTP REQUEST
      if (StrStr(received, "+IPD") != NULL){
        chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%s", 
		"Received http request");
        clientID[0] = clearRequest[5];
        clientID[1] = '\0';
        request = clearRequest[17]; //it is c for command (c=M0...)
        command[0] = clearRequest[19];
        command[1] = clearRequest[20];
        command[2] = clearRequest[21];
        command[3] = clearRequest[22];
        command[4] = clearRequest[23];
        command[5] = clearRequest[24];
        command[6] = clearRequest[25];
        command[7] = clearRequest[26];
        command[8] = '\0';
        chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%s", "Client id=");
        chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%s\n", clientID);
        chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%s", "Command=");
        chprintf((BaseSequentialStream*)MONITOR_SERIAL, "%s\n", command);
        printWebPage();
      }
      /***********END************/
      pos = 0;
    }
  }
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
  char cipSend[100] = {"AT+CIPSEND="};
  /*char webPage[512] = {"<html><head></head><body>"
      "<form>"
      "<input type='text' name='c'></input>"
      "<input type='submit' value='Send'>"
      "</form>"};*/
  char webPage[600] = {"<html><head></head><body><table border=0>"
"<tr><td></td><td><form><input type='hidden' name='c' value='M0127127'>
</input><input type='submit' value='^'></form></td><td></td></tr>"
"<tr><td><form><input type='hidden' name='c' value='M0255255'></input>
<input type='submit' value='<'></form></td><td></td><td><form>
<input type='hidden' name='c' value='M0127000'></input><input type='submit'
value='>'></form></td></tr>"
"<tr><td></td><td><form><input type='hidden' name='c' value='M0000127'>
</input><input type='submit' value='-'></form></td><td></td></tr></table>"
  };
  char webPage1[20] = {"</body></html>"};
  if (request == 'c')
    strcat(webPage,command);
  strcat(webPage,webPage1);
  strcat(cipSend,clientID);
  strcat(cipSend,",");
  int pageLength = strlen(webPage);
  char pageLengthAsString[100];
  itoa(pageLength,pageLengthAsString);
  strcat(cipSend,pageLengthAsString);
  strcat(cipSend,"\r\n");
  sendToESP8266(cipSend, COMMAND_SLEEP);
  sendToESP8266(webPage, COMMAND_SLEEP);
  char closeCommand[40];
  strcpy(closeCommand,ESP8266_CLOSE_CONN);
  strcat(closeCommand,clientID);
  strcat(closeCommand,"\r\n");
  sendToESP8266(closeCommand, COMMAND_SLEEP);
}
/**
 * Return length of str as char (es. '47')
 */
int strlen(const char * str){
    int len;
    for (len = 0; str[len]; len++);
    return len;
}

char *strcpy(char *dest, const char *src){
  unsigned i;
  for (i=0; src[i] != '\0'; ++i)
    dest[i] = src[i];
  dest[i] = '\0';
  return dest;
}
char* StrStr(const char *str, const char *target) {
  if (!*target) return str;
  char *p1 = (char*)str, *p2 = (char*)target;
  char *p1Adv = (char*)str;
  while (*++p2)
    p1Adv++;
  while (*p1Adv) {
    char *p1Begin = p1;
    p2 = (char*)target;
    while (*p1 && *p2 && *p1 == *p2) {
      p1++;
      p2++;
    }
    if (!*p2)
      return p1Begin;
    p1 = p1Begin + 1;
    p1Adv++;
  }
  return NULL;
}
char *strcat(char *dest, const char *src){
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}
static void println(char *p) {

  while (*p) {
    chSequentialStreamPut(MONITOR_SERIAL, *p++);
  }
  chSequentialStreamWrite(MONITOR_SERIAL, (uint8_t * )"\r\n", 2);
}
/* itoa:  convert n to characters in s */
 void itoa(int n, char s[]){
     int i, sign;
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
 /* reverse:  reverse string s in place */
void reverse(char s[]){
    int i, j;
    char c;
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
