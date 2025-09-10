#include "lcd_bsp.h"
#include "cst816.h"
#include "lcd_bl_pwm_bsp.h"
#include "lcd_config.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char * ssid = "MahnaMahna";
const char * password = "doodoodeedoodoo";

uint8_t *jpgData = nullptr;
size_t jpgSize = 0;

void wifi_init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  printf("Connecting to WiFi\n");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  Touch_Init();
  wifi_init();
  lcd_lvgl_Init();
  lcd_bl_pwm_bsp_init(LCD_PWM_MODE_255);

  if (downloadImage("http://localhost:3000/test", &jpgData, &jpgSize)) {
    Serial.println("Image downloaded, ready to draw.");
    // drawJPG(jpgData, jpgSize);
    // free(jpgData); // Free after drawing
  } else {
    Serial.println("Failed to download image.");
  }
}

void loop() {
}

// http://localhost:3000/test
bool downloadImage(const char* imageUrl, uint8_t** imageData, size_t* imageSize) {
  WiFiClient client;
  HTTPClient http;

  Serial.print("HTTP Begin...\n");
  if (http.begin(client, imageUrl)) {
    Serial.print("HTTP GET...\n");
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        *imageSize = http.getSize();

        if (*imageSize > 0) {
          *imageData = (uint8_t*)malloc(*imageSize);

          if (*imageData) {
            WiFiClient* stream = http.getStreamPtr();
            size_t bytesRead = stream->readBytes(*imageData, *imageSize);

            if (bytesRead == *imageSize) {
              Serial.println("Image downloaded successfully!");
              http.end();
              return true;
            } else {
              Serial.printf("Failed to read all the image data. Expected %d, Read %d\n", *imageSize, bytesRead);
              free(*imageData);
              *imageData = nullptr;
            }
          } else {
            Serial.println("Failed to allocate memory for image.");
          }
        } else {
          Serial.println("Downloaded image size is 0 or unknown.");
        }
      }
    } else {
      Serial.printf("HTTP GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("HTTP Unable to connect\n");
  }
  return false;
}

