/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card

  IMPORTANT!!!
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#define CAMERA_MODEL_AI_THINKER
#define BUILD_IN_LED	4

#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>


// Import BMP header constructor by https://github.com/bitluni/ESP32CameraI2S
#include "BMP.h"
unsigned char bmpHeader[BMP::headerSize24];

camera_fb_t * fb = NULL; // pointer to camera image data

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


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
    <h2>ESP32-CAM Last Photo</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="rotatePhoto();">ROTATE</button>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
    </p>
  </div>
  <div><img src="saved-photo" id="photo"></div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.send();
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

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin();//(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // prepare flashlight
  pinMode(BUILD_IN_LED, OUTPUT);

  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Turn-off the 'brownout detector'
 // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
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


  // from Camera Counter OCR
  config.frame_size = FRAMESIZE_QVGA;//FRAMESIZE_QQVGA;//FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_GRAYSCALE;//PIXFORMAT_RGB565;//PIXFORMAT_GRAYSCALE;
  config.fb_count = 1;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.deepSleep(3600000000);
  }

  // Prepare bitmap header
  uint16_t w_size=0;
  uint16_t h_size=0;

  if(config.frame_size == FRAMESIZE_QVGA){
	  w_size = 320;
	  h_size = 240;
  }
  else if(config.frame_size == FRAMESIZE_QQVGA){
	  w_size = 160;
	  h_size = 120;
  }
  else{
	  w_size = 100;
	  h_size = 100;
	  Serial.println("Undefined image size!!");
  }
  if(config.pixel_format == PIXFORMAT_GRAYSCALE){
	  BMP::construct24BitHeader(bmpHeader, w_size, h_size);
  }
  else{
	  BMP::construct16BitHeader(bmpHeader, w_size, h_size);
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    takeNewPhoto = true;
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {

      // Chunked response, we calculate the chunks based on free heap (in multiples of 32)
      // This is necessary when a TLS connection is open since it sucks too much memory
	  // https://github.com/helderpe/espurna/blob/76ad9cde5a740822da9fe6e3f369629fa4b59ebc/code/espurna/web.ino - Thanks A LOT!
	  Serial.printf(PSTR("[MAIN] Free heap: %d bytes\n"), ESP.getFreeHeap());

	  AsyncWebServerResponse *response = request->beginChunkedResponse("image/x-windows-bmp",[](uint8_t *buffer, size_t maxLen, size_t index) -> size_t{
          return genBMPChunk((char *)buffer, (int)maxLen, index);
      });
	  response->addHeader("Content-Disposition", "inline; filename=capture.bmp");
	  request->send(response);

  });

  // Start server
  server.begin();

  // Take first photo immediately
  takeNewPhoto = true;

}

int genBMPChunk(char *buffer, int maxLen, size_t index)
{
	  size_t BMPSize = BMP::headerSize24 + (fb->len * 3);        //TODO
      size_t max 	 = (ESP.getFreeHeap() / 3) & 0xFFE0;

      // Get the chunk based on the index and maxLen
      size_t len = BMPSize - index;
      if (len > maxLen) len = maxLen;
      if (len > max) len = max;
      if (len > 0){
    	  if(0==index){
    		  memcpy_P(buffer, bmpHeader + index, BMP::headerSize24);
    		  len = BMP::headerSize24;
    		  Serial.printf(PSTR("[WEB] Sending BMP (max chunk size: %4d) "), max);
    	  }
    	  else{
    		  if(fb->format==PIXFORMAT_GRAYSCALE){
				for(uint32_t i = 0; i < len; i+=3){
					uint8_t Gray = *(fb->buf + (index - BMP::headerSize24 + i)/3);
					*(buffer + i + 0) = Gray;
					*(buffer + i + 1) = Gray;
					*(buffer + i + 2) = Gray;
				}
    		  }
    		  else{
    			  memcpy_P(buffer, fb->buf + index - BMP::headerSize24, len);
    		  }
    		  Serial.printf(PSTR("."));
    	  }
      }
      if (len == 0) Serial.printf(PSTR("\r\n"));
      // Return the actual length of the chunk (0 for end of file)
      return len;
}

void loop() {
  if (takeNewPhoto) {
    capturePhoto();
    takeNewPhoto = false;
  }
  delay(10);
}

// Capture Photo and Save it to SPIFFS
void capturePhoto() {

    // Take a photo with the camera
    Serial.println("Taking a photo...");

    esp_camera_fb_return(fb);

    // Turns on the ESP32-CAM white on-board LED (flash) connected to GPIO 4
    digitalWrite(BUILD_IN_LED, HIGH);
    //delay(10);

    fb = esp_camera_fb_get();

    digitalWrite(BUILD_IN_LED, LOW);

    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    Serial.printf("Camera capture done. W=%i, H=%i, Bytes=%i\r\n",fb->width, fb->height, fb->len);

	//https://www.arducam.com/rgb565-format-issues/
	//direct save to BMP is not possible - reverse bytes order
    //fixing procedure:

    if(fb->format == PIXFORMAT_RGB565){
		//Read 320x240x2 byte from buffer
		//Save as RGB565 bmp format
		for(uint32_t i = 0; i < fb->len; i+=2){
			// swap bytes
			uint8_t tmp = *(fb->buf + i);
			*(fb->buf + i) = *(fb->buf + i+1);
			*(fb->buf + i+1) = tmp;
		}
    }

}
