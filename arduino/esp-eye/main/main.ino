#include "esp_camera.h"
#include "esp_http_server.h"

#include <WiFi.h>


//ESP-Eye PIN Map
#define CAM_PIN_PWDN    -1 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    4
#define CAM_PIN_SIOD    18
#define CAM_PIN_SIOC    23

#define CAM_PIN_D7      36
#define CAM_PIN_D6      37
#define CAM_PIN_D5      38
#define CAM_PIN_D4      39
#define CAM_PIN_D3      35
#define CAM_PIN_D2      14
#define CAM_PIN_D1      13
#define CAM_PIN_D0      34
#define CAM_PIN_VSYNC   5
#define CAM_PIN_HREF    27
#define CAM_PIN_PCLK    25

// Wifi credentials
const char* ssid = "FRITZ!BoxAA2";
const char* password = "45374170342399146189";

// Not ecaxtly sure what this is?
httpd_handle_t camera_httpd = NULL;


static esp_err_t test_handler(httpd_req_t *req){
   httpd_resp_set_type(req, "text/html");
   //httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

   char* test_string = "<b> Test! </b>";
   return httpd_resp_send(req, test_string, strlen(test_string));
}

static esp_err_t capture_handler(httpd_req_t *req){
  camera_fb_t* fb = NULL;
   fb = esp_camera_fb_get();
   
   if(!fb){
      Serial.println("Error capturing image!");
      httpd_resp_send_500(req);
      return ESP_FAIL;
   }

   httpd_resp_set_type(req, "image/jpeg");
   httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
   esp_err_t res = ESP_OK;
   size_t fb_len = 0;
   
   fb_len = fb->len;
   res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

   esp_camera_fb_return(fb);

   Serial.println("Image captured successfully!");

   return res;
}

void startCamServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  // For each site we want to display we have to create
  // a httpd_uri_t containing a handler
  httpd_uri_t test_uri = {
    .uri      = "/test",
    .method   = HTTP_GET,
    .handler  = test_handler,
    .user_ctx = NULL
  };

  httpd_uri_t capture_uri = {
    .uri      = "/capture",
    .method   = HTTP_GET,
    .handler  = capture_handler,
    .user_ctx = NULL
  };

  Serial.printf("Starting web server!");
  // Start the webserver
  if (httpd_start(&camera_httpd, &config) == ESP_OK){
     // Register all of the URIs
     httpd_register_uri_handler(camera_httpd, &test_uri);
     httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
  
}

void setup(){
  // Set up debuging
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // This is also specific for the ESP-Eye
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  // Initialize the camera with the parameters above
  // (and hope it does not throw an error!)
  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK){
    Serial.printf("Camera initialization failed with error 0x%x", err);
    return;  
  }

  // According to the example, the default settings are not good:
  // - the sensor has to be flipped
  // - Raise the brightness
  // - Lower the saturation
  sensor_t * s = esp_camera_sensor_get();
  if(s->id.PID = OV3660_PID){
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  // Set the framesize (QVGA = 320 * 240)
  s->set_framesize(s, FRAMESIZE_QVGA);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Waiting for Wifi connection...");
  }

  Serial.printf("Camera initialization successful and Wifi connected!!");
  Serial.print("Connect on http://");
  Serial.println(WiFi.localIP());

  startCamServer();
  
}

void loop(){

}
