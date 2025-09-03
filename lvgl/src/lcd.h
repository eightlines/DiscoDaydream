#ifndef LCD_H
#define LCD_H

#include "Arduino.h"
#include <lvgl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_check.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif 

void lcd_init();
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
void lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void increase_lvgl_tick(void *arg);
static void lvgl_port_task(void *arg);
static void lvgl_unlock(void);
static bool lvgl_lock(int timeout_ms);

void lcd_bl_pwm_bsp_init(uint16_t duty);
void setUpdutySubdivide(uint16_t duty);

void drawBox();

#ifdef __cplusplus
}
#endif

#endif