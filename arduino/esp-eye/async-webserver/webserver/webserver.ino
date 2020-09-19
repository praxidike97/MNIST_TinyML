//#include <ESPAsyncWebServer.h>

#include "WiFi.h"
#include "esp_camera.h"

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

// The HTML-site to display
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align:center; }
    .vert { margin-bottom: 10%; }
    .hori{ margin-bottom: 0%; }
  </style>
</head>
<body>
  <div id="container">
    <h2>Smile!</h2>
    <p>ESP-Eye Camera</p>
    <p>
      <button onclick="rotatePhoto();">ROTATE</button>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
    </p>
  </div>
  <div><img id="photo" src="capture" width="30%"></div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
     var source = 'capture';
     timestamp = (new Date()).getTime();
     newUrl = source + '?_=' + timestamp;
     document.getElementById("photo").src = newUrl;
  }
  function rotatePhoto() {
     var img = document.getElementById("photo");
     deg += 90;
     if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
     else{ document.getElementById("container").className = "hori"; }
     img.style.transform = "rotate(" + deg + "deg)";
  }
  function isOdd(n) { return Math.abs(n % 2) == 1; }
</script>
</html>)rawliteral";

// Wifi credentials
const char* ssid = "FRITZ!BoxAA2";
const char* password = "45374170342399146189";

// Create AsyncWebserver object
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);

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

  config.frame_size = FRAMESIZE_VGA;
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
  while(WiFi.status() != WL_CONNECTED){
     Serial.println("Connecting to WiFi...");
     delay(1000);
  }

  Serial.print("IP address is http://");
  Serial.println(WiFi.localIP());

  // Set-up all the desired HTML pages
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/html", index_html);
  });

    // Set-up all the desired HTML pages
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
      camera_fb_t* fb = NULL;
      fb = esp_camera_fb_get();

      // Send Error 500 if there is an error capturing the image
      if(!fb){
         Serial.println("Error capturing image!");
         request->send(500);
      }

      esp_camera_fb_return(fb);

      request->send_P(200, "image/jpeg", (const uint8_t *)fb->buf, fb->len);
  });


  server.begin();
}

void loop() {
  //Serial.println("Test");
  //delay(1000);
}
