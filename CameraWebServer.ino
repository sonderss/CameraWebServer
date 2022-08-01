#include <WiFi.h>
#include "esp_camera.h"
#include <WebSocketsClient.h>

//#define PWDN_GPIO_NUM     32
//#define RESET_GPIO_NUM    -1
//#define XCLK_GPIO_NUM      0
//#define SIOD_GPIO_NUM     26
//#define SIOC_GPIO_NUM     27
//#define Y9_GPIO_NUM       35
//#define Y8_GPIO_NUM       34
//#define Y7_GPIO_NUM       39
//#define Y6_GPIO_NUM       36
//#define Y5_GPIO_NUM       21
//#define Y4_GPIO_NUM       19
//#define Y3_GPIO_NUM       18
//#define Y2_GPIO_NUM        5
//#define VSYNC_GPIO_NUM    25
//#define HREF_GPIO_NUM     23
//#define PCLK_GPIO_NUM     22




//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15 
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "SONDER";
const char* password = "sonderswifi";


const char* hostname = "ESP32CAM";
const char* socket_url = "ws://121.5.78.186";
int socket_port = 8090;
WebSocketsClient webSocket;


void startCameraServer();

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
 
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      webSocket.sendTXT("camlogin");
    }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      break;
    case WStype_BIN:
      break;
    case WStype_PING:
        Serial.printf("[WSc] get ping\n");
        break;
    case WStype_PONG:
        Serial.printf("[WSc] get pong\n");
        break;
    }
}
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
//  if(config.pixel_format == PIXFORMAT_JPEG){
//    if(psramFound()){
//      config.jpeg_quality = 10;
//      config.fb_count = 2;
//      config.grab_mode = CAMERA_GRAB_LATEST;
//    } else {
//      // Limit the frame size when PSRAM is not available
//      config.frame_size = FRAMESIZE_SVGA;
//      config.fb_location = CAMERA_FB_IN_DRAM;
//    }
//  } else {
//    // Best option for face detection/recognition
//    config.frame_size = FRAMESIZE_240X240;
//#if CONFIG_IDF_TARGET_ESP32S3
//    config.fb_count = 2;
//#endif
//  }

//#if defined(CAMERA_MODEL_ESP_EYE)
//  pinMode(13, INPUT_PULLUP);
//  pinMode(14, INPUT_PULLUP);
//#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

//  sensor_t * s = esp_camera_sensor_get();
//  // initial sensors are flipped vertically and colors are a bit saturated
//  if (s->id.PID == OV3660_PID) {
//    s->set_vflip(s, 1); // flip it back
//    s->set_brightness(s, 1); // up the brightness just a bit
//    s->set_saturation(s, -2); // lower the saturation
//  }
//  // drop down frame size for higher initial frame rate
//  if(config.pixel_format == PIXFORMAT_JPEG){
//    s->set_framesize(s, FRAMESIZE_QVGA);
//  }
//
//#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
//  s->set_vflip(s, 1);
//  s->set_hmirror(s, 1);
//#endif
//
//#if defined(CAMERA_MODEL_ESP32S3_EYE)
//  s->set_vflip(s, 1);
//#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  webSocket.begin(socket_url,socket_port);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(10000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}
unsigned long messageTimestamp = 0;
void loop() {
   webSocket.loop();
    uint64_t now = millis();
    if(now - messageTimestamp > 10) {
        messageTimestamp = now;
        camera_fb_t * fb = NULL;
        // Take Picture with Camera
        fb = esp_camera_fb_get();  
        if(!fb) {
          Serial.println("Camera capture failed");
          return;
        }
        webSocket.sendBIN(fb->buf,fb->len);
        Serial.println("Image sent");
        esp_camera_fb_return(fb); 
    }
 delay(1000);
}
