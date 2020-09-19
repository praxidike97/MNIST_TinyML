#include <Arduino.h>

#include <ESPAsyncWebServer.h>

#include "WiFi.h"
#include "esp_camera.h"

#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "camera_utils.h"

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

  // Initialize ESP-Eye camera
  int camera_status = esp_eye_camera_init();
  if (camera_status != 0){
    Serial.println("Camera initialization failed!");
  }

  // According to the example, the default settings are not good:
  // - the sensor has to be flipped
  // - Raise the brightness
  // - Lower the saturation
  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_brightness(s, 1);
  s->set_saturation(s, -2);

  Serial.println("Camera initialization successful!");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
     Serial.println("Connecting to WiFi...");
     delay(1000);
  }

  Serial.print("IP address is http://");
  Serial.println(WiFi.localIP());

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
}