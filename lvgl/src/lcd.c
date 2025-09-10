#include "lcd_bsp.h"
#include "esp_lcd_sh8601.h"
#include "lcd_config.h"
#include "cst816.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_heap_caps.h>
#include "libs/tjpgd3/src/tjpgd.h"
#include <Arduino.h>

static SemaphoreHandle_t lvgl_mux = NULL; //mutex semaphores

#define LCD_HOST    SPI2_HOST
#define SH8601_ID   0x86
#define CO5300_ID   0xff

typedef struct {
    uint8_t *data;
    size_t size;
} downloaded_image_t;

static esp_lcd_panel_io_handle_t amoled_panel_io_handle = NULL; 
static SemaphoreHandle_t lvgl_mux = NULL;

void drawRedRectangle(void) {
  lv_obj_t * screen = lv_scr_act();
  lv_obj_t * rect = lv_obj_create(screen);
  lv_obj_set_size(rect, 362, 362);
  lv_obj_set_style_bg_color(rect, lv_color_make(0xFF, 0x00, 0x00), LV_PART_MAIN);
  lv_obj_center(rect);
  return;
}

void drawJPG(void) {
//   Serial.println("Test");
  // JDEC jd;
  // void * work = malloc(3100);

  // if (!work) {
  //   // Serial.println("Failed to allocate memory for JPEG decoder.");
  //   return;
  // }

  // JRESULT = jd_prepare(&jd, [](JDEC* jd, uint8_t* buf, unsigned int len) -> unsigned int {
  //   static size_t pos = 0;
  //   if (pos + len > jpgSize) {
  //     len = jpgSize - pos; // Adjust length to avoid overflow
  //   }

  //   if (buf && len) {
  //     memcpy(buf, jpgData + pos, len);
  //   }

  //   pos += len;
  //   return len;
  // }, work, 3100, (void*)jpgData);

  // if (res != JDR_OK) {
  //   // Serial.printf("Failed to prepare JPEG decoder: %d\n", res);
  //   free(work);
  //   return;
  // }

  // res = jd_decompress(&jd, jpegDrawCallback, 0);

  // if (res != JDR_OK) {
  //   // Serial.printf("Failed to decompress JPEG image: %d\n", res);
  // }
  // free(work);

  // LV_IMG_DECLARE(TESTIMAGE);

  // lv_obj_t *img;
  // img = lv_img_create(lv_scr_act());
  // lv_img_set_src(img, &TESTIMAGE);
  return;
}

static int jpegDrawCallback(JDEC* jd, void* bitmap, JRECT* rect) {
  // Cast bitmap to uint16_t* if using RGB565
  uint16_t* src = (uint16_t*)bitmap;
  // Serial.printf("Drawing rect: (%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);
  return 1;
}
// static unsigned int tjd_read_bytes_cb(JDEC* jdec, uint8_t* buf, unsigned int len);
// static unsigned int tjd_write_pixels_cb(JDEC* jdec, void* bitmap, JRECT* rect);

// typedef struct {
//     const uint8_t* jpeg_data;
//     size_t jpeg_data_len;
//     size_t read_pos;
//     lv_color_t* pixel_buffer; // LVGL color buffer
//     int x_offset;
//     int y_offset;
//     int target_width;
//     int target_height;
// } JpegDecodeContext;

// static unsigned int tjd_read_bytes_cb(JDEC* jdec, uint8_t* buf, unsigned int len) {
//     JpegDecodeContext* ctx = (JpegDecodeContext*)jdec->device;
//     if (!ctx || ctx->read_pos + len > ctx->jpeg_data_len) {
//         return 0; // Error or end of data
//     }
//     if (buf) {
//         memcpy(buf, ctx->jpeg_data + ctx->read_pos, len);
//     }
//     ctx->read_pos += len;
//     return len;
// }

// static unsigned int tjd_write_pixels_cb(JDEC* jdec, void* bitmap, JRECT* rect) {
//     JpegDecodeContext* ctx = (JpegDecodeContext*)jdec->device;

//     // Calculate destination start and end coordinates within the LVGL buffer
//     // Make sure we don't write outside the allocated buffer
//     int start_x = rect->left;
//     int end_x = rect->right + 1;
//     int start_y = rect->top;
//     int end_y = rect->bottom + 1;

//     if (!ctx->pixel_buffer || start_x >= ctx->target_width || start_y >= ctx->target_height) {
//         return 0; // Buffer not ready or outside bounds
//     }

//     // The 'bitmap' provided by TJpgDec is typically RGB888 or BGR888.
//     // We need to convert it to LVGL's color format (e.g., LV_COLOR_RGB565).
//     uint16_t* pixels = (uint16_t*)bitmap; // Assuming RGB565 by default from TJpgDec output

//     for (int y = start_y; y < end_y; y++) {
//         if (y >= ctx->target_height) break; // Avoid writing past allocated buffer height
//         for (int x = start_x; x < end_x; x++) {
//             if (x >= ctx->target_width) break; // Avoid writing past allocated buffer width

//             // Get the pixel data from TJpgDec's output (usually RGB565 if that's configured,
//             // otherwise you might need to convert from RGB888/BGR888)
//             // TJpgDec can directly output RGB565 if configured correctly.
//             lv_color_t color;
//             #if LV_COLOR_DEPTH == 16
//                 // If TJpgDec outputs 16-bit RGB565 directly
//                 color.full = pixels[(y - start_y) * (end_x - start_x) + (x - start_x)];
//             #elif LV_COLOR_DEPTH == 32 || LV_COLOR_DEPTH == 24 // Assuming TJpgDec gives RGB888 in 'pixels' for 24/32 depth
//                 uint8_t r = ((uint8_t*)pixels)[((y - start_y) * (end_x - start_x) + (x - start_x)) * 3 + 0];
//                 uint8_t g = ((uint8_t*)pixels)[((y - start_y) * (end_x - start_x) + (x - start_x)) * 3 + 1];
//                 uint8_t b = ((uint8_t*)pixels)[((y - start_y) * (end_x - start_x) + (x - start_x)) * 3 + 2];
//                 color = lv_color_make(r, g, b);
//             #else
//                 // Handle other color depths or ensure TJpgDec outputs the correct format
//                 // For now, default to assuming TJpgDec outputs something compatible with LV_COLOR_MAKE
//                 uint8_t r = (pixels[(y - start_y) * (end_x - start_x) + (x - start_x)] >> 11) & 0x1F;
//                 uint8_t g = (pixels[(y - start_y) * (end_x - start_x) + (x - start_x)] >> 5) & 0x3F;
//                 uint8_t b = (pixels[(y - start_y) * (end_x - start_x) + (x - start_x)] >> 0) & 0x1F;
//                 color = lv_color_make(r << 3, g << 2, b << 3); // Convert 565 to 888 for lv_color_make if needed
//             #endif

//             // Calculate the index in the LVGL buffer
//             size_t buffer_idx = y * ctx->target_width + x;
//             ctx->pixel_buffer[buffer_idx] = color;
//         }
//     }
//     return 1; // Return 1 to continue decoding
// }

// lv_res_t lv_img_decoder_open_jpeg(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc) {
//     if (dsc->src_type == LV_IMG_SRC_VARIABLE || dsc->src_type == LV_IMG_SRC_UNKNOWN) {
//         const uint8_t* jpeg_data = (const uint8_t*)dsc->src;
//         size_t jpeg_data_len = dsc->len;

//         if (jpeg_data == nullptr || jpeg_data_len == 0) {
//             return LV_RES_INV;
//         }

//         JDEC jdec;
//         JpegDecodeContext ctx;
//         ctx.jpeg_data = jpeg_data;
//         ctx.jpeg_data_len = jpeg_data_len;
//         ctx.read_pos = 0;
//         ctx.pixel_buffer = nullptr; // Will be allocated after header is read

//         // Prepare for decoding - get image info
//         JRESULT jresult = jd_prepare(&jdec, tjd_read_bytes_cb, (void*)&ctx);
//         if (jresult != JDR_OK) {
//             Serial.printf("JPEG jd_prepare failed, error: %d\n", jresult);
//             return LV_RES_INV;
//         }

//         // Allocate memory for the decoded pixels
//         // Assuming RGB565 output from TJpgDec for 16-bit color depth
//         // If LV_COLOR_DEPTH is 32, you might need to allocate 4 bytes per pixel.
//         size_t pixel_size_bytes = jdec.width * jdec.height * (LV_COLOR_DEPTH / 8);
//         ctx.pixel_buffer = (lv_color_t*)malloc(pixel_size_bytes);
//         if (!ctx.pixel_buffer) {
//             Serial.println("Failed to allocate memory for decoded JPEG pixels.");
//             return LV_RES_INV;
//         }
//         ctx.target_width = jdec.width;
//         ctx.target_height = jdec.height;

//         // Decode the JPEG image
//         jresult = jd_decomp(&jdec, tjd_write_pixels_cb, 0); // Scale factor 0 = 1/1
//         if (jresult != JDR_OK) {
//             Serial.printf("JPEG jd_decomp failed, error: %d\n", jresult);
//             free(ctx.pixel_buffer);
//             return LV_RES_INV;
//         }

//         // Fill the LVGL descriptor with the decoded image info
//         dsc->img_data = (uint8_t*)ctx.pixel_buffer;
//         dsc->header.w = jdec.width;
//         dsc->header.h = jdec.height;
        
//         // Set the color format based on your LVGL config.
//         // TJpgDec typically outputs 16-bit RGB565, so LV_IMG_CF_TRUE_COLOR
//         // is appropriate if LV_COLOR_DEPTH is 16.
//         // If you configure TJpgDec for 24-bit output, then LV_IMG_CF_TRUE_COLOR_ALPHA
//         // or LV_IMG_CF_TRUE_COLOR would be used, but generally 16-bit is more common.
//         #if LV_COLOR_DEPTH == 16
//             dsc->header.cf = LV_IMG_CF_TRUE_COLOR; // For RGB565
//         #elif LV_COLOR_DEPTH == 32 // Assuming TJpgDec could output RGB888 or ARGB888
//             dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA; // If it has an alpha channel
//         #else
//             dsc->header.cf = LV_IMG_CF_UNKNOWN; // Let LVGL figure it out if you're unsure
//         #endif

//         return LV_RES_OK;
//     }
//     return LV_RES_INV;
// }

// void lv_img_decoder_close_jpeg(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc) {
//     if (dsc->img_data) {
//         free((void*)dsc->img_data); // Free the memory allocated for decoded pixels
//         dsc->img_data = nullptr;
//     }
// }

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = 
{
  {0xF0, (uint8_t[]){0x28}, 1, 0},
  {0xF2, (uint8_t[]){0x28}, 1, 0},
  {0x73, (uint8_t[]){0xF0}, 1, 0},
  {0x7C, (uint8_t[]){0xD1}, 1, 0},
  {0x83, (uint8_t[]){0xE0}, 1, 0},
  {0x84, (uint8_t[]){0x61}, 1, 0},
  {0xF2, (uint8_t[]){0x82}, 1, 0},
  {0xF0, (uint8_t[]){0x00}, 1, 0},
  {0xF0, (uint8_t[]){0x01}, 1, 0},
  {0xF1, (uint8_t[]){0x01}, 1, 0},
  {0xB0, (uint8_t[]){0x56}, 1, 0},
  {0xB1, (uint8_t[]){0x4D}, 1, 0},
  {0xB2, (uint8_t[]){0x24}, 1, 0},
  {0xB4, (uint8_t[]){0x87}, 1, 0},
  {0xB5, (uint8_t[]){0x44}, 1, 0},
  {0xB6, (uint8_t[]){0x8B}, 1, 0},
  {0xB7, (uint8_t[]){0x40}, 1, 0},
  {0xB8, (uint8_t[]){0x86}, 1, 0},
  {0xBA, (uint8_t[]){0x00}, 1, 0},
  {0xBB, (uint8_t[]){0x08}, 1, 0},
  {0xBC, (uint8_t[]){0x08}, 1, 0},
  {0xBD, (uint8_t[]){0x00}, 1, 0},
  {0xC0, (uint8_t[]){0x80}, 1, 0},
  {0xC1, (uint8_t[]){0x10}, 1, 0},
  {0xC2, (uint8_t[]){0x37}, 1, 0},
  {0xC3, (uint8_t[]){0x80}, 1, 0},
  {0xC4, (uint8_t[]){0x10}, 1, 0},
  {0xC5, (uint8_t[]){0x37}, 1, 0},
  {0xC6, (uint8_t[]){0xA9}, 1, 0},
  {0xC7, (uint8_t[]){0x41}, 1, 0},
  {0xC8, (uint8_t[]){0x01}, 1, 0},
  {0xC9, (uint8_t[]){0xA9}, 1, 0},
  {0xCA, (uint8_t[]){0x41}, 1, 0},
  {0xCB, (uint8_t[]){0x01}, 1, 0},
  {0xD0, (uint8_t[]){0x91}, 1, 0},
  {0xD1, (uint8_t[]){0x68}, 1, 0},
  {0xD2, (uint8_t[]){0x68}, 1, 0},
  {0xF5, (uint8_t[]){0x00, 0xA5}, 2, 0},
  {0xDD, (uint8_t[]){0x4F}, 1, 0},
  {0xDE, (uint8_t[]){0x4F}, 1, 0},
  {0xF1, (uint8_t[]){0x10}, 1, 0},
  {0xF0, (uint8_t[]){0x00}, 1, 0},
  {0xF0, (uint8_t[]){0x02}, 1, 0},
  {0xE0, (uint8_t[]){0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34}, 14, 0},
  {0xE1, (uint8_t[]){0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33}, 14, 0},
  {0xF0, (uint8_t[]){0x10}, 1, 0},
  {0xF3, (uint8_t[]){0x10}, 1, 0},
  {0xE0, (uint8_t[]){0x07}, 1, 0},
  {0xE1, (uint8_t[]){0x00}, 1, 0},
  {0xE2, (uint8_t[]){0x00}, 1, 0},
  {0xE3, (uint8_t[]){0x00}, 1, 0},
  {0xE4, (uint8_t[]){0xE0}, 1, 0},
  {0xE5, (uint8_t[]){0x06}, 1, 0},
  {0xE6, (uint8_t[]){0x21}, 1, 0},
  {0xE7, (uint8_t[]){0x01}, 1, 0},
  {0xE8, (uint8_t[]){0x05}, 1, 0},
  {0xE9, (uint8_t[]){0x02}, 1, 0},
  {0xEA, (uint8_t[]){0xDA}, 1, 0},
  {0xEB, (uint8_t[]){0x00}, 1, 0},
  {0xEC, (uint8_t[]){0x00}, 1, 0},
  {0xED, (uint8_t[]){0x0F}, 1, 0},
  {0xEE, (uint8_t[]){0x00}, 1, 0},
  {0xEF, (uint8_t[]){0x00}, 1, 0},
  {0xF8, (uint8_t[]){0x00}, 1, 0},
  {0xF9, (uint8_t[]){0x00}, 1, 0},
  {0xFA, (uint8_t[]){0x00}, 1, 0},
  {0xFB, (uint8_t[]){0x00}, 1, 0},
  {0xFC, (uint8_t[]){0x00}, 1, 0},
  {0xFD, (uint8_t[]){0x00}, 1, 0},
  {0xFE, (uint8_t[]){0x00}, 1, 0},
  {0xFF, (uint8_t[]){0x00}, 1, 0},
  {0x60, (uint8_t[]){0x40}, 1, 0},
  {0x61, (uint8_t[]){0x04}, 1, 0},
  {0x62, (uint8_t[]){0x00}, 1, 0},
  {0x63, (uint8_t[]){0x42}, 1, 0},
  {0x64, (uint8_t[]){0xD9}, 1, 0},
  {0x65, (uint8_t[]){0x00}, 1, 0},
  {0x66, (uint8_t[]){0x00}, 1, 0},
  {0x67, (uint8_t[]){0x00}, 1, 0},
  {0x68, (uint8_t[]){0x00}, 1, 0},
  {0x69, (uint8_t[]){0x00}, 1, 0},
  {0x6A, (uint8_t[]){0x00}, 1, 0},
  {0x6B, (uint8_t[]){0x00}, 1, 0},
  {0x70, (uint8_t[]){0x40}, 1, 0},
  {0x71, (uint8_t[]){0x03}, 1, 0},
  {0x72, (uint8_t[]){0x00}, 1, 0},
  {0x73, (uint8_t[]){0x42}, 1, 0},
  {0x74, (uint8_t[]){0xD8}, 1, 0},
  {0x75, (uint8_t[]){0x00}, 1, 0},
  {0x76, (uint8_t[]){0x00}, 1, 0},
  {0x77, (uint8_t[]){0x00}, 1, 0},
  {0x78, (uint8_t[]){0x00}, 1, 0},
  {0x79, (uint8_t[]){0x00}, 1, 0},
  {0x7A, (uint8_t[]){0x00}, 1, 0},
  {0x7B, (uint8_t[]){0x00}, 1, 0},
  {0x80, (uint8_t[]){0x48}, 1, 0},
  {0x81, (uint8_t[]){0x00}, 1, 0},
  {0x82, (uint8_t[]){0x06}, 1, 0},
  {0x83, (uint8_t[]){0x02}, 1, 0},
  {0x84, (uint8_t[]){0xD6}, 1, 0},
  {0x85, (uint8_t[]){0x04}, 1, 0},
  {0x86, (uint8_t[]){0x00}, 1, 0},
  {0x87, (uint8_t[]){0x00}, 1, 0},
  {0x88, (uint8_t[]){0x48}, 1, 0},
  {0x89, (uint8_t[]){0x00}, 1, 0},
  {0x8A, (uint8_t[]){0x08}, 1, 0},
  {0x8B, (uint8_t[]){0x02}, 1, 0},
  {0x8C, (uint8_t[]){0xD8}, 1, 0},
  {0x8D, (uint8_t[]){0x04}, 1, 0},
  {0x8E, (uint8_t[]){0x00}, 1, 0},
  {0x8F, (uint8_t[]){0x00}, 1, 0},
  {0x90, (uint8_t[]){0x48}, 1, 0},
  {0x91, (uint8_t[]){0x00}, 1, 0},
  {0x92, (uint8_t[]){0x0A}, 1, 0},
  {0x93, (uint8_t[]){0x02}, 1, 0},
  {0x94, (uint8_t[]){0xDA}, 1, 0},
  {0x95, (uint8_t[]){0x04}, 1, 0},
  {0x96, (uint8_t[]){0x00}, 1, 0},
  {0x97, (uint8_t[]){0x00}, 1, 0},
  {0x98, (uint8_t[]){0x48}, 1, 0},
  {0x99, (uint8_t[]){0x00}, 1, 0},
  {0x9A, (uint8_t[]){0x0C}, 1, 0},
  {0x9B, (uint8_t[]){0x02}, 1, 0},
  {0x9C, (uint8_t[]){0xDC}, 1, 0},
  {0x9D, (uint8_t[]){0x04}, 1, 0},
  {0x9E, (uint8_t[]){0x00}, 1, 0},
  {0x9F, (uint8_t[]){0x00}, 1, 0},
  {0xA0, (uint8_t[]){0x48}, 1, 0},
  {0xA1, (uint8_t[]){0x00}, 1, 0},
  {0xA2, (uint8_t[]){0x05}, 1, 0},
  {0xA3, (uint8_t[]){0x02}, 1, 0},
  {0xA4, (uint8_t[]){0xD5}, 1, 0},
  {0xA5, (uint8_t[]){0x04}, 1, 0},
  {0xA6, (uint8_t[]){0x00}, 1, 0},
  {0xA7, (uint8_t[]){0x00}, 1, 0},
  {0xA8, (uint8_t[]){0x48}, 1, 0},
  {0xA9, (uint8_t[]){0x00}, 1, 0},
  {0xAA, (uint8_t[]){0x07}, 1, 0},
  {0xAB, (uint8_t[]){0x02}, 1, 0},
  {0xAC, (uint8_t[]){0xD7}, 1, 0},
  {0xAD, (uint8_t[]){0x04}, 1, 0},
  {0xAE, (uint8_t[]){0x00}, 1, 0},
  {0xAF, (uint8_t[]){0x00}, 1, 0},
  {0xB0, (uint8_t[]){0x48}, 1, 0},
  {0xB1, (uint8_t[]){0x00}, 1, 0},
  {0xB2, (uint8_t[]){0x09}, 1, 0},
  {0xB3, (uint8_t[]){0x02}, 1, 0},
  {0xB4, (uint8_t[]){0xD9}, 1, 0},
  {0xB5, (uint8_t[]){0x04}, 1, 0},
  {0xB6, (uint8_t[]){0x00}, 1, 0},
  {0xB7, (uint8_t[]){0x00}, 1, 0},
  {0xB8, (uint8_t[]){0x48}, 1, 0},
  {0xB9, (uint8_t[]){0x00}, 1, 0},
  {0xBA, (uint8_t[]){0x0B}, 1, 0},
  {0xBB, (uint8_t[]){0x02}, 1, 0},
  {0xBC, (uint8_t[]){0xDB}, 1, 0},
  {0xBD, (uint8_t[]){0x04}, 1, 0},
  {0xBE, (uint8_t[]){0x00}, 1, 0},
  {0xBF, (uint8_t[]){0x00}, 1, 0},
  {0xC0, (uint8_t[]){0x10}, 1, 0},
  {0xC1, (uint8_t[]){0x47}, 1, 0},
  {0xC2, (uint8_t[]){0x56}, 1, 0},
  {0xC3, (uint8_t[]){0x65}, 1, 0},
  {0xC4, (uint8_t[]){0x74}, 1, 0},
  {0xC5, (uint8_t[]){0x88}, 1, 0},
  {0xC6, (uint8_t[]){0x99}, 1, 0},
  {0xC7, (uint8_t[]){0x01}, 1, 0},
  {0xC8, (uint8_t[]){0xBB}, 1, 0},
  {0xC9, (uint8_t[]){0xAA}, 1, 0},
  {0xD0, (uint8_t[]){0x10}, 1, 0},
  {0xD1, (uint8_t[]){0x47}, 1, 0},
  {0xD2, (uint8_t[]){0x56}, 1, 0},
  {0xD3, (uint8_t[]){0x65}, 1, 0},
  {0xD4, (uint8_t[]){0x74}, 1, 0},
  {0xD5, (uint8_t[]){0x88}, 1, 0},
  {0xD6, (uint8_t[]){0x99}, 1, 0},
  {0xD7, (uint8_t[]){0x01}, 1, 0},
  {0xD8, (uint8_t[]){0xBB}, 1, 0},
  {0xD9, (uint8_t[]){0xAA}, 1, 0},
  {0xF3, (uint8_t[]){0x01}, 1, 0},
  {0xF0, (uint8_t[]){0x00}, 1, 0},
  {0x21, (uint8_t[]){0x00}, 1, 0},
  {0x11, (uint8_t[]){0x00}, 1, 120},
  {0x29, (uint8_t[]){0x00}, 1, 0},
#ifdef Rotate_90
  {0x36, (uint8_t[]){0x60}, 1, 0},
#else
  {0x36, (uint8_t[]){0x00}, 1, 0},
#endif
};

void lcd_init(void) {
    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_drv_t disp_drv;

    const spi_bus_config_t buscfg = SH8601_PANEL_BUS_QSPI_CONFIG(
        PIN_NUM_LCD_PCLK,
        PIN_NUM_LCD_DATA0,
        PIN_NUM_LCD_DATA1,
        PIN_NUM_LCD_DATA2,
        PIN_NUM_LCD_DATA3,
        LCD_H_RES * LCD_V_RES * LCD_BIT_PER_PIXEL / 8
    );

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;

    const esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(
        PIN_NUM_LCD_CS,
        notify_lvgl_flush_ready,
        &disp_drv
    );

    sh8601_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags = {
            .use_qspi_interface = 1,
        },
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    amoled_panel_io_handle = io_handle;
    esp_lcd_panel_handle_t panel_handle = NULL;

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .color_space = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_init(panel_handle));
    //ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_disp_on_off(panel_handle, true));

    lv_init();
    
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * LVGL_BUF_HEIGHT);
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.rounder_cb = lvgl_rounder_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    lvgl_mux = xSemaphoreCreateMutex(); //mutex semaphores
    assert(lvgl_mux);
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);
    if (lvgl_lock(-1)) {
        // drawRedRectangle();
        drawJPG();
        lvgl_unlock();
    }   

}

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    const int offsetx1 = area->x1;
    const int offsetx2 = area->x2;
    const int offsety1 = area->y1;
    const int offsety2 = area->y2;

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

void lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area) {
  uint16_t x1 = area->x1;
  uint16_t x2 = area->x2;

  uint16_t y1 = area->y1;
  uint16_t y2 = area->y2;

  // round the start of coordinate down to the nearest 2M number
  area->x1 = (x1 >> 1) << 1;
  area->y1 = (y1 >> 1) << 1;
  // round the end of coordinate up to the nearest 2N+1 number
  area->x2 = ((x2 >> 1) << 1) + 1;
  area->y2 = ((y2 >> 1) << 1) + 1;
}

static void lvgl_port_task(void *arg) {
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    for(;;) {
        if (lvgl_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            lvgl_unlock();
        }

        if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
        }

        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static bool lvgl_lock(int timeout_ms) {
    assert(lvgl_mux && "bsp_display_start must be called first");
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

static void lvgl_unlock(void) {
    assert(lvgl_mux && "bsp_display_start must be called first");
    xSemaphoreGive(lvgl_mux);
}

static void lvgl_increase_tick(void *arg) {
  lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    uint16_t tp_x, tp_y;
    uint8_t win = getTouch(&tp_x, &tp_y);

    if (win) {
        #ifdef Rotate_90
            data->point.x = tp_y;
            data->point.y = (LCD_V_RES - tp_x);
        #else
            data->point.x = tp_x;
            data->point.y = tp_y;
        #endif

        if(data->point.x > LCD_H_RES)
            data->point.x = LCD_H_RES;
    
        if(data->point.y > LCD_V_RES)
            data->point.y = LCD_V_RES;

        data->state = LV_INDEV_STATE_PRESSED;
        //ESP_LOGE("TP","(%d,%d)",data->point.x,data->point.y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void lcd_bl_pwm_bsp_init(uint16_t duty) {
    ledc_timer_config_t timer_conf = {
        .speed_mode =  LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT, //256
        .timer_num =  LEDC_TIMER_3,
        .freq_hz = 50 * 1000,
        .clk_cfg = LEDC_SLOW_CLK_APB,
    };

    ledc_channel_config_t ledc_conf = {
        .gpio_num = PIN_NUM_BK_LIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel =  LEDC_CHANNEL_1,
        .intr_type =  LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_3,
        .duty = duty,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_timer_config(&timer_conf));
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&ledc_conf));
}

static void setUpduty(uint16_t duty)
{
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}

void setUpdutySubdivide(uint16_t duty) {
    setUpduty(duty);
}

static void increase_lvgl_tick(void *arg) {
  lv_tick_inc(LVGL_TICK_PERIOD_MS);
}
