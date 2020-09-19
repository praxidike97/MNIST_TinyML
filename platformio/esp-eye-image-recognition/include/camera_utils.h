#include "esp_camera.h"
#include "esp_system.h"

// Set the camera color scheme, size and frequency
#define CAMERA_PIXEL_FORMAT PIXFORMAT_GRAYSCALE
#define CAMERA_FRAME_SIZE FRAMESIZE_96X96

#define XCLK_FRQ 20000000

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

// The methods to initialize the camera and take a picture
// are written in C so we also have to do this.
#ifdef __cplusplus
extern "C" {
#endif

int esp_eye_camera_init();

#ifdef __cplusplus
}
#endif