/********************************************************************
 * LCD(1602) Clock
 *
 * Modules: Arduino nano (ATmega328P)
 *          1602 LCD
 *          LCD I2C
 *          DS1302 (Arduino Zero에는 RTC 내장됨 추후 이전 고려)
 *          Buzzer
 *          DHT22 (AM2302)
 * 배선:
 *       LCD I2C : 5v, SDA:A4 SCL:A5 (아두이노 Wire 라이브러리 특성)
 *       DS1302  : 3.3v, SCLK:9, IO:10, RST:11
 *       Buzzer  : D7
 *       DHT22   : D8
 */

#define BUZZER_PIN    7         // 부저 핀
#define DHT22_PIN     8         // DHT22 데이터핀
#define DEBUG_LED     13        // 아두이노 기판실장 LED

#define LCD_CHARS     16        // LCD 글자수
#define LCD_LINES      2        // LCD 라인수
#define LCD_I2C_ADDR  0x27      // LCD I2C 주소(모듈에 설정되어 있음)

#include <stdio.h>              // standard C library. sprintf ... etc
#include <string.h>             // standard C library. memset ... etc
#include <ctype.h>              // standard C library. isdigit ... etc

#include <Wire.h>               // standard Arduino I2C library

// LCD I2C 라이브러리는 https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library 에서 다운로드해서
// <내 문서>/Arduino/libraries 에 설치한다.
#include <LiquidCrystal_I2C.h>  // LCD I2C library

// Time by Michael Margolis       http://playground.arduino.cc/code/time
#include <TimeLib.h>

// DHTLib                         https://playground.arduino.cc/Main/DHTLib
#include <dht.h>

#include "DS1302.h"
#include "LCDDisplayModule.h"
#include "TallNumeric.h"

// LCD 컨트롤 class
LiquidCrystal_I2C   lcd(LCD_I2C_ADDR, LCD_CHARS, LCD_LINES);

// DS1302 RTC 구조체
ds1302_struct rtc;

// dht 온습도센서 class
dht DHT;

int last_sync_hour = 0;            // RTC와 마지막 싱크시각
int dht_result = 1;                // DHT 센서 처리결과
int dht_check_second = -1;         // DHT 센서 처리초

int debug_led_status = 0;          // 디버그 LED 상태
int last_second_chk = -1;          // 디버그 LED 상태체크를 위한 초


/* setup 초기화 */
void setup() {
  // init lcd
  lcd.begin();
  lcd.noBacklight();
  lcd.noCursor();
  lcd.home();  // x, y
  lcd.clear();
  lcd.print("Init");
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DEBUG_LED, OUTPUT);
  pinMode(DHT22_PIN, INPUT);

  Serial.begin(9600);  // for debug
  while (!Serial) ; // wait for Arduino Serial Monitor // Needed for Leonardo only

  // init rtc
  // Start by clearing the Write Protect bit
  // Otherwise the clock data cannot be written
  // The whole register is written,
  // but the WP-bit is the only bit in that register.
  DS1302_write (DS1302_ENABLE, 0);
  // Disable Trickle Charger.
  DS1302_write (DS1302_TRICKLE, 0x00);

  // init timer
  get_rtc();

  // test buzzer
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.backlight();
}


/* main loop function */
void loop() {
  int current_second = second();

  // 시리얼포트를 통한 시간 조정
  if (Serial.available()) {
    digitalWrite(DEBUG_LED, HIGH);

    // 부저로 시간조정모드 시작 알림
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);

    time_setup_serial();

    // 부저로 시간조정모드 종료 알림
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);

    // 처리결과를 4초간 display
    delay(3 * 1000);
    digitalWrite(DEBUG_LED, LOW);
  }

  if (timeStatus() != timeNotSet) {
    lcd_TimeDisplay();
  }

  // resync time
  if (last_sync_hour != hour()) {
    get_rtc();
  }

  // 디버그 LED 깜빡이기
  if (last_second_chk != current_second) {
    last_second_chk = current_second;
    if (debug_led_status == 0) {
      digitalWrite(DEBUG_LED, LOW);
      debug_led_status = 1;
    }
    else {
      digitalWrite(DEBUG_LED, HIGH);
      debug_led_status = 0;
    }
  }

  // 온습도 읽기 (5초단위: 2, 7, 12, 17 ...)
  if (current_second % 5 == 2 && current_second != dht_check_second) {
    dht_check_second = current_second;

    dht_result = DHT.read22(DHT22_PIN);
/*
    if (dht_result == DHTLIB_OK) {
      Serial.print("DHT READ OK RH:");
      Serial.print(DHT.humidity, 1);
      Serial.print(", TEMP:");
      Serial.print(DHT.temperature, 1);
      Serial.println(".");
    }
    else {
      Serial.println("DHT READ ERROR.");
    }
*/
  }

//Serial.println(current_second);
  delay(100);
}


void time_setup_serial() {
  char buffer[17];
  int bufferRead = 0;

  int l_hour, l_minute, l_second;
  int l_day, l_month, l_year;
  char c;

  last_disp_func = LCD_DEBUG;
  lcd.clear();
  lcd.print("Serial Time Set");
  lcd.setCursor(0, 1);

  //Serial.setTimeout(5 * 1000);

  // Time Set Command: SETyyyymmddHHmiss.
  if (Serial.find("T")) {
    memset(buffer, 0, sizeof(buffer));

    bufferRead = Serial.readBytes(buffer, sizeof(buffer) - 1);
    if (bufferRead == 0) {
      lcd.print("Read TimeOut!");
      return;
    }

    lcd.print(String(buffer));
    // format: yyyymmddHHmiss.

    bufferRead = sscanf(buffer, "%04d%02d%02d%02d%02d%02d%c",
      &l_year, &l_month, &l_day, &l_hour, &l_minute, &l_second, &c);

    if (bufferRead != 7) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Format Error! 01");
      return;
    }
    if (c != '.') {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Format Error! 02");
      return;
    }

    if (l_year < 2017 || l_year > 2099) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Year Error!     ");
      return;
    }

    if (l_month < 1 || l_month > 12) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Month Error!    ");
      return;
    }

    if (l_day < 1 || l_day > 31) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Day Error!      ");
      return;
    }

    if (l_hour < 0 || l_hour > 23) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Hour Error!     ");
      return;
    }

    if (l_minute < 0 || l_minute > 59) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Minute Error!   ");
      return;
    }

    if (l_second < 0 || l_second > 59) {
      delay(2 * 1000);
      lcd.setCursor(0, 1);
      lcd.print("Second Error!   ");
      return;
    }

    setTime(l_hour, l_minute, l_second, l_day, l_month, l_year);
    // RTC save
    set_rtc();

    delay(2 * 1000);
    lcd.setCursor(0, 1);
    lcd.print("OK!             ");
  }
  else {
    lcd.print("Command Error!");
  }

}

void get_rtc() {
  int l_hour, l_minute, l_second, l_ampm, fmt12_24;
  int l_day, l_weekday, l_month, l_year;

  // Read all clock data at once (burst mode).
  DS1302_clock_burst_read( (uint8_t *) &rtc);

  l_hour = bcd2bin( rtc.h24.Hour10, rtc.h24.Hour);
  l_minute = bcd2bin( rtc.Minutes10, rtc.Minutes);
  l_second = bcd2bin( rtc.Seconds10, rtc.Seconds);

  fmt12_24 = rtc.h24.hour_12_24;
  // 12 hour format
  if (fmt12_24 == 1) {
    l_ampm = rtc.h12.AM_PM;      // AM:0, PM:1
    if (l_ampm == 1) {
      l_hour += 12;
    }
  }
  // 24 hour format
  /*
  else {
    fmt12_24 = 1;              // use 12 hour format
    if (hour > 12) {
      l_ampm = 1;
      l_hour -= 12;
    }
  }
  */

  l_day = bcd2bin( rtc.Date10, rtc.Date);
  l_weekday = rtc.Day;
  l_month = bcd2bin( rtc.Month10, rtc.Month);
  l_year = 2000 + bcd2bin( rtc.Year10, rtc.Year);

  // set time
  setTime(l_hour, l_minute, l_second, l_day, l_month, l_year);

  // save synchronized hour
  last_sync_hour = hour();
}

void set_rtc() {
  // Fill these variables with the date and time.
  int l_seconds, l_minutes, l_hours, l_dayofweek, l_dayofmonth, l_month, l_year;

  l_seconds    = second();          // the second now (0-59)
  l_minutes    = minute();          // the minute now (0-59)
  l_hours      = hour();            // the hour now  (0-23)
  l_dayofweek  = weekday() + 1;     // (TimeLib) day of the week, Sunday is day 0  // (DS1302) Day of week, any day can be first, counts 1...7
  l_dayofmonth = day();             // the day now (1-31)
  l_month      = month();           // the month now (1-12)
  l_year       = year();            // the full four digit year: (2009, 2010 etc)

  // Set a time and date
  // This also clears the CH (Clock Halt) bit,
  // to start the clock.

  // Fill the structure with zeros to make
  // any unused bits zero
  memset ((char *) &rtc, 0, sizeof(rtc));

  rtc.Seconds    = bin2bcd_l( l_seconds);
  rtc.Seconds10  = bin2bcd_h( l_seconds);
  rtc.CH         = 0;      // 1 for Clock Halt, 0 to run;
  rtc.Minutes    = bin2bcd_l( l_minutes);
  rtc.Minutes10  = bin2bcd_h( l_minutes);
  // To use the 12 hour format,
  // use it like these four lines:
  //    rtc.h12.Hour   = bin2bcd_l( hours);
  //    rtc.h12.Hour10 = bin2bcd_h( hours);
  //    rtc.h12.AM_PM  = 0;     // AM = 0
  //    rtc.h12.hour_12_24 = 1; // 1 for 24 hour format
  rtc.h24.Hour   = bin2bcd_l( l_hours);
  rtc.h24.Hour10 = bin2bcd_h( l_hours);
  rtc.h24.hour_12_24 = 0; // 0 for 24 hour format
  rtc.Date       = bin2bcd_l( l_dayofmonth);
  rtc.Date10     = bin2bcd_h( l_dayofmonth);
  rtc.Month      = bin2bcd_l( l_month);
  rtc.Month10    = bin2bcd_h( l_month);
  rtc.Day        = l_dayofweek;
  rtc.Year       = bin2bcd_l( l_year - 2000);
  rtc.Year10     = bin2bcd_h( l_year - 2000);
  rtc.WP = 0;

  // Write all clock data at once (burst mode).
  DS1302_clock_burst_write( (uint8_t *) &rtc);

}
