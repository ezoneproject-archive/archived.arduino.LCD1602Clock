// lcd display module

/**
 * display date time
 */
void lcd_TimeDisplay() {
  char *weekdayStr;
  char twoDigitsStr[3];

  // 최종 실행한 함수가 아닌 경우
  if (last_disp_func != LCD_TIME_DISPLAY) {
    last_disp_func = LCD_TIME_DISPLAY;

    // 전체 초기화
    last_year = 0;
    last_month = 0;
    last_day = 0;
    last_hour = -1;
    last_minute = -1;
    last_second = -1;

    lcd.clear();

    // 변경되지 않는 항목 표시
    lcd.setCursor(9, 0);
    lcd.write('/');        // 월/일 구분

    lcd.setCursor(2, 1);
    lcd.write(':');        // 시/분 구분
  }

  // 분이 변경된 경우
  if (last_minute != minute()) {
    last_minute = minute();

    // 0열
    lcd.setCursor(3, 0);
    //if (last_minute < 10) {
    //  lcd.write(' ');
    //}
    //else {
      lcd.write(LCD_LN0_MIN1);
      lcd.createChar((uint8_t)LCD_LN0_MIN1, (uint8_t*)TALL_NUMERIC[last_minute / 10][0]);
    //}

    lcd.setCursor(4, 0);
    lcd.write(LCD_LN0_MIN2);
    lcd.createChar((uint8_t)LCD_LN0_MIN2, (uint8_t*)TALL_NUMERIC[last_minute % 10][0]);

    // 1열
    lcd.setCursor(3, 1);
    //if (last_minute < 10) {
    //  lcd.write(' ');
    //}
    //else {
      lcd.write(LCD_LN1_MIN1);
      lcd.createChar((uint8_t)LCD_LN1_MIN1, (uint8_t*)TALL_NUMERIC[last_minute / 10][1]);
    //}

    lcd.setCursor(4, 1);
    lcd.write(LCD_LN1_MIN2);
    lcd.createChar((uint8_t)LCD_LN1_MIN2, (uint8_t*)TALL_NUMERIC[last_minute % 10][1]);
  }

  // 시간이 변경된 경우
  if (last_hour != hourFormat12()) {
    last_hour = hourFormat12();

    // 0열
    lcd.setCursor(0, 0);
    if (last_hour < 10) {
      lcd.write(' ');
    }
    else {
      lcd.write(LCD_LN0_HOUR1);
      lcd.createChar((uint8_t)LCD_LN0_HOUR1, (uint8_t*)TALL_NUMERIC[last_hour / 10][0]);
    }

    lcd.setCursor(1, 0);
    lcd.write(LCD_LN0_HOUR2);
    lcd.createChar((uint8_t)LCD_LN0_HOUR2, (uint8_t*)TALL_NUMERIC[last_hour % 10][0]);

    // 1열
    lcd.setCursor(0, 1);
    if (last_hour < 10) {
      lcd.write(' ');
    }
    else {
      lcd.write(LCD_LN1_HOUR1);
      lcd.createChar((uint8_t)LCD_LN1_HOUR1, (uint8_t*)TALL_NUMERIC[last_hour / 10][1]);
    }

    lcd.setCursor(1, 1);
    lcd.write(LCD_LN1_HOUR2);
    lcd.createChar((uint8_t)LCD_LN1_HOUR2, (uint8_t*)TALL_NUMERIC[last_hour % 10][1]);

    // am/pm 표시
    lcd.setCursor(5, 0);
    if (isPM()) {
      lcd.print("p");
    }
    else {
      lcd.print("a");
    }
  }

  // 초가 변경된 경우
  if (last_second != second()) {
    last_second = second();
    sprintf(twoDigitsStr, "%02d", last_second);
    lcd.setCursor(5, 1);
    lcd.print(String(twoDigitsStr));

    tempHumDisplayMini();
  }

  // 년도가 변경된 경우
  if (last_year != year()) {
    last_year = year();
  }

  // 월이 변경된 경우
  if (last_month != month()) {
    last_month = month();
    sprintf(twoDigitsStr, "%2d", last_month);
    lcd.setCursor(7, 0);
    lcd.print(String(twoDigitsStr));
  }

  // 일자가 변경된 경우
  if (last_day != day()) {
    last_day = day();
    sprintf(twoDigitsStr, "%02d", last_day);
    lcd.setCursor(10, 0);
    lcd.print(String(twoDigitsStr));

    // 일자가 변경되면 요일도 변경
    weekdayStr = dayShortStr(weekday());
    //lcd.setCursor(8, 1);
    lcd.setCursor(13, 0);
    lcd.print(String(weekdayStr));
  }

}

/**
 * 온습도 디스플레이
 */
void tempHumDisplayMini() {
  if (dht_result != DHTLIB_OK) {
    return;
  }

  lcd.setCursor(10, 1);

  // 00 ~ 05 초는 온도(온도를 6초간 표시)
  if (last_second % 10 <= 5) {
    lcd.print(DHT.temperature, 1);
    lcd.write(223); // 온도 o 표시
    lcd.write('C');
  }
  // 그 외는 습도
  else {
    lcd.print(DHT.humidity, 1);
    lcd.write('%');
    lcd.write('H');
  }



}

/**
 * 온습도 디스플레이
 */
void tempHumDisplay() {
    //if (dht_result == DHTLIB_OK) {
      //Serial.print(DHT.humidity, 1);
      //Serial.print(DHT.temperature, 1);
    //}

  // 최종 실행한 함수가 아닌 경우
  if (last_disp_func != LCD_TEMPHUM_DISPLAY) {
    last_disp_func = LCD_TEMPHUM_DISPLAY;

    lcd.clear();

    // 변경되지 않는 항목 표시
    lcd.setCursor(0, 0);
    lcd.print("Temperature:");

    lcd.setCursor(0, 1);
    lcd.print("Humidity:");
  }

  lcd.setCursor(12, 0);
  lcd.print(DHT.humidity, 1);
  lcd.setCursor(9, 1);
  lcd.print(DHT.temperature, 1);

}



