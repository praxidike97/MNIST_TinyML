#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "WiFi.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sensor.h"

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

#define DL_IMAGE_MIN(A, B) ((A) < (B) ? (A) : (B))
#define DL_IMAGE_MAX(A, B) ((A) < (B) ? (B) : (A))

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


void image_zoom_in_twice(uint8_t *dimage,
                         int dw,
                         int dh,
                         int dc,
                         uint8_t *simage,
                         int sw,
                         int sc)
{
    for (int dyi = 0; dyi < dh; dyi++)
    {
        int _di = dyi * dw;

        int _si0 = dyi * 2 * sw;
        int _si1 = _si0 + sw;

        for (int dxi = 0; dxi < dw; dxi++)
        {
            int di = (_di + dxi) * dc;
            int si0 = (_si0 + dxi * 2) * sc;
            int si1 = (_si1 + dxi * 2) * sc;

            if (1 == dc)
            {
                dimage[di] = (uint8_t)((simage[si0] + simage[si0 + 1] + simage[si1] + simage[si1 + 1]) >> 2);
            }
            else if (3 == dc)
            {
                dimage[di] = (uint8_t)((simage[si0] + simage[si0 + 3] + simage[si1] + simage[si1 + 3]) >> 2);
                dimage[di + 1] = (uint8_t)((simage[si0 + 1] + simage[si0 + 4] + simage[si1 + 1] + simage[si1 + 4]) >> 2);
                dimage[di + 2] = (uint8_t)((simage[si0 + 2] + simage[si0 + 5] + simage[si1 + 2] + simage[si1 + 5]) >> 2);
            }
            else
            {
                for (int dci = 0; dci < dc; dci++)
                {
                    dimage[di + dci] = (uint8_t)((simage[si0 + dci] + simage[si0 + 3 + dci] + simage[si1 + dci] + simage[si1 + 3 + dci] + 2) >> 2);
                }
            }
        }
    }
    return;
}

void image_resize_linear(uint8_t *dst_image, uint8_t *src_image, int dst_w, int dst_h, int dst_c, int src_w, int src_h)
{ /*{{{*/
    float scale_x = (float)src_w / dst_w;
    float scale_y = (float)src_h / dst_h;

    int dst_stride = dst_c * dst_w;
    int src_stride = dst_c * src_w;

    if (fabs(scale_x - 2) <= 1e-6 && fabs(scale_y - 2) <= 1e-6)
    {
        image_zoom_in_twice(
            dst_image,
            dst_w,
            dst_h,
            dst_c,
            src_image,
            src_w,
            dst_c);
    }
    else
    {
        for (int y = 0; y < dst_h; y++)
        {
            float fy[2];
            fy[0] = (float)((y + 0.5) * scale_y - 0.5); // y
            int src_y = (int)fy[0];                     // y1
            fy[0] -= src_y;                             // y - y1
            fy[1] = 1 - fy[0];                          // y2 - y
            src_y = DL_IMAGE_MAX(0, src_y);
            src_y = DL_IMAGE_MIN(src_y, src_h - 2);

            for (int x = 0; x < dst_w; x++)
            {
                float fx[2];
                fx[0] = (float)((x + 0.5) * scale_x - 0.5); // x
                int src_x = (int)fx[0];                     // x1
                fx[0] -= src_x;                             // x - x1
                if (src_x < 0)
                {
                    fx[0] = 0;
                    src_x = 0;
                }
                if (src_x > src_w - 2)
                {
                    fx[0] = 0;
                    src_x = src_w - 2;
                }
                fx[1] = 1 - fx[0]; // x2 - x

                for (int c = 0; c < dst_c; c++)
                {
                    dst_image[y * dst_stride + x * dst_c + c] = round(src_image[src_y * src_stride + src_x * dst_c + c] * fx[1] * fy[1] + src_image[src_y * src_stride + (src_x + 1) * dst_c + c] * fx[0] * fy[1] + src_image[(src_y + 1) * src_stride + src_x * dst_c + c] * fx[1] * fy[0] + src_image[(src_y + 1) * src_stride + (src_x + 1) * dst_c + c] * fx[0] * fy[0]);
                }
            }
        }
    }
} /*}}}*/

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
  config.pixel_format = PIXFORMAT_GRAYSCALE;

  config.frame_size = FRAMESIZE_QVGA;
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

      // Crop the frame
      uint8_t* new_frame = NULL;
      image_resize_linear(new_frame, fb->buf, 100, 100, 1, 320, 240);
      /*for (int y = 0; y < 50; y++)
        for (int x = 0; x < 50; x++)
            current_frame[y][x] = 0;*/
      const uint8_t * fb_array = (const uint8_t *)fb->buf;

      Serial.print("fb length: ");
      Serial.println(*fb_array);

      sensor_t * s = esp_camera_sensor_get();
      Serial.println(s->id.PID);

      request->send_P(200, "image/jpeg", (const uint8_t *)fb->buf, fb->len);
      //request->send_P(200, "image/jpeg", current_frame, sizeof(current_frame));
  });


  server.begin();
}

void loop() {
  //Serial.println("Test");
  //delay(1000);
}