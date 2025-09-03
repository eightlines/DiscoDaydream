#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#define LCD_H_RES              360
#define LCD_V_RES              360

#define LCD_BIT_PER_PIXEL              16
#define LCD_RGB_ELEMENT_ORDER_RGB      1
#define LCD_RGB_ELEMENT_ORDER_BGR      2

#define PIN_NUM_LCD_CS      14
#define PIN_NUM_LCD_PCLK    13
#define PIN_NUM_LCD_DATA0   15
#define PIN_NUM_LCD_DATA1   16
#define PIN_NUM_LCD_DATA2   17
#define PIN_NUM_LCD_DATA3   18
#define PIN_NUM_LCD_RST     21
#define PIN_NUM_BK_LIGHT    47

#define LVGL_BUF_HEIGHT        (LCD_V_RES / 10)
#define LVGL_TICK_PERIOD_MS    2                          //Timer time
#define LVGL_TASK_MAX_DELAY_MS 500                        //LVGL Indicates the maximum time for a task to run
#define LVGL_TASK_MIN_DELAY_MS 1                          //LVGL Minimum time to run a task
#define LVGL_TASK_STACK_SIZE   (4 * 1024)                 //LVGL runs the task stack
#define LVGL_TASK_PRIORITY     2                          //LVGL Running task priority

#define TOUCH_ADDR                0x15
#define PIN_NUM_TOUCH_SCL 12
#define PIN_NUM_TOUCH_SDA 11

#define  LCD_PWM_MODE_0   0
#define  LCD_PWM_MODE_25  25
#define  LCD_PWM_MODE_50  50
#define  LCD_PWM_MODE_75  75
#define  LCD_PWM_MODE_100 100
#define  LCD_PWM_MODE_125 125
#define  LCD_PWM_MODE_150 150
#define  LCD_PWM_MODE_175 175
#define  LCD_PWM_MODE_200 200
#define  LCD_PWM_MODE_225 225
#define  LCD_PWM_MODE_255 255

//#define Backlight_Testing
//#define Rotate_90
#endif