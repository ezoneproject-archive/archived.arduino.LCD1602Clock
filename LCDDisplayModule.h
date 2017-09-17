// lcd display module

#define LCD_TIME_DISPLAY       1
#define LCD_TEMPHUM_DISPLAY    2

#define LCD_DEBUG              99


#define LCD_LN0_HOUR1          0
#define LCD_LN0_HOUR2          1
#define LCD_LN1_HOUR1          2
#define LCD_LN1_HOUR2          3
#define LCD_LN0_MIN1           4
#define LCD_LN0_MIN2           5
#define LCD_LN1_MIN1           6
#define LCD_LN1_MIN2           7

int last_disp_func = 0;    // 1:lcd_TimeDisplay, 2:tempHumDisplay
int last_year = 0;
int last_month = 0;
int last_day = 0;
int last_hour = -1;
int last_minute = -1;
int last_second = -1;
