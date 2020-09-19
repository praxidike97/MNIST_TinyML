#include <Arduino.h>
#include "esp_camera.h"

#include "camera_utils.h"

int esp_eye_camera_init(){
  // These pins are specific for the ESP-Eye !!
  camera_config_t config;
  
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CAM_PIN_D0;
  config.pin_d1 = CAM_PIN_D1;
  config.pin_d2 = CAM_PIN_D2;
  config.pin_d3 = CAM_PIN_D3;
  config.pin_d4 = CAM_PIN_D4;
  config.pin_d5 = CAM_PIN_D5;
  config.pin_d6 = CAM_PIN_D6;
  config.pin_d7 = CAM_PIN_D7;
  config.pin_xclk = CAM_PIN_XCLK;
  config.pin_pclk = CAM_PIN_PCLK;
  config.pin_vsync = CAM_PIN_VSYNC;
  config.pin_href = CAM_PIN_HREF;
  config.pin_sscb_sda = CAM_PIN_SIOD;
  config.pin_sscb_scl = CAM_PIN_SIOC;
  config.pin_pwdn = CAM_PIN_PWDN;
  config.pin_reset = CAM_PIN_RESET;
  
  config.xclk_freq_hz = 5000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;

  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // This is also specific for the ESP-Eye
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  // Initialize the camera with the parameters above
  // (and hope it does not throw an error!)
  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK){
    return -1;  
  }

  return 0;
}