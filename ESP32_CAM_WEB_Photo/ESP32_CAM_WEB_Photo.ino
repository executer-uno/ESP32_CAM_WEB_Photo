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
//#include <FS.h>                // SD Card ESP32
//#include <SD_MMC.h>            // SD Card ESP32


// Import BMP header constructor by https://github.com/bitluni/ESP32CameraI2S
#include "BMP.h"
unsigned char bmpHeader[BMP::headerSize];

camera_fb_t * fb = NULL; // pointer to camera image data



//File: favicon.ico.gz, Size: 726
#define favicon_ico_gz_len 726
const uint8_t favicon_ico_gz[] PROGMEM = {
 0x1F, 0x8B, 0x08, 0x08, 0x0B, 0x87, 0x90, 0x57, 0x00, 0x03, 0x66, 0x61, 0x76, 0x69, 0x63, 0x6F,
 0x6E, 0x2E, 0x69, 0x63, 0x6F, 0x00, 0xCD, 0x53, 0x5F, 0x48, 0x9A, 0x51, 0x14, 0xBF, 0x62, 0x6D,
 0x86, 0x96, 0xA9, 0x64, 0xD3, 0xFE, 0xA8, 0x99, 0x65, 0x1A, 0xB4, 0x8A, 0xA8, 0x51, 0x54, 0x23,
 0xA8, 0x11, 0x49, 0x51, 0x8A, 0x34, 0x62, 0x93, 0x85, 0x31, 0x58, 0x44, 0x12, 0x45, 0x2D, 0x58,
 0xF5, 0x52, 0x41, 0x10, 0x23, 0x82, 0xA0, 0x20, 0x98, 0x2F, 0xC1, 0x26, 0xED, 0xA1, 0x20, 0x89,
 0x04, 0xD7, 0x83, 0x58, 0x20, 0x28, 0x04, 0xAB, 0xD1, 0x9B, 0x8C, 0xE5, 0xC3, 0x60, 0x32, 0x64,
 0x0E, 0x56, 0xBF, 0x9D, 0xEF, 0xF6, 0x30, 0x82, 0xED, 0xAD, 0x87, 0xDD, 0x8F, 0xF3, 0xDD, 0x8F,
 0x73, 0xCF, 0xEF, 0x9C, 0xDF, 0x39, 0xBF, 0xFB, 0x31, 0x26, 0xA2, 0x27, 0x37, 0x97, 0xD1, 0x5B,
 0xCF, 0x9E, 0x67, 0x30, 0xA6, 0x66, 0x8C, 0x99, 0xC9, 0xC8, 0x45, 0x9E, 0x6B, 0x3F, 0x5F, 0x74,
 0xA6, 0x94, 0x5E, 0xDB, 0xFF, 0xB2, 0xE6, 0xE7, 0xE7, 0xF9, 0xDE, 0xD6, 0xD6, 0x96, 0xDB, 0xD8,
 0xD8, 0x78, 0xBF, 0xA1, 0xA1, 0xC1, 0xDA, 0xDC, 0xDC, 0x2C, 0xEB, 0xED, 0xED, 0x15, 0x9B, 0xCD,
 0xE6, 0x4A, 0x83, 0xC1, 0xE0, 0x2E, 0x29, 0x29, 0x99, 0xD6, 0x6A, 0xB5, 0x4F, 0x75, 0x3A, 0x9D,
 0x61, 0x75, 0x75, 0x95, 0xB5, 0xB7, 0xB7, 0xDF, 0xC8, 0xD1, 0xD4, 0xD4, 0xF4, 0xB0, 0xBA, 0xBA,
 0xFA, 0x83, 0xD5, 0x6A, 0xFD, 0x5A, 0x5E, 0x5E, 0x9E, 0x28, 0x2D, 0x2D, 0x0D, 0x10, 0xC6, 0x4B,
 0x98, 0x78, 0x5E, 0x5E, 0xDE, 0x95, 0x42, 0xA1, 0x40, 0x4E, 0x4E, 0xCE, 0x65, 0x76, 0x76, 0xF6,
 0x47, 0xB5, 0x5A, 0x6D, 0x4F, 0x26, 0x93, 0xA2, 0xD6, 0xD6, 0x56, 0x8E, 0x6D, 0x69, 0x69, 0xD1,
 0x11, 0x36, 0x62, 0xB1, 0x58, 0x60, 0x32, 0x99, 0xA0, 0xD7, 0xEB, 0x51, 0x58, 0x58, 0x88, 0xFC,
 0xFC, 0x7C, 0x10, 0x16, 0x02, 0x56, 0x2E, 0x97, 0x43, 0x2A, 0x95, 0x42, 0x2C, 0x16, 0x23, 0x33,
 0x33, 0x33, 0xAE, 0x52, 0xA9, 0x1E, 0x64, 0x65, 0x65, 0x71, 0x7C, 0x7D, 0x7D, 0xBD, 0x93, 0xEA,
 0xFE, 0x30, 0x1A, 0x8D, 0xE8, 0xEC, 0xEC, 0xC4, 0xE2, 0xE2, 0x22, 0x6A, 0x6A, 0x6A, 0x40, 0x39,
 0x41, 0xB5, 0x38, 0x4E, 0xC8, 0x33, 0x3C, 0x3C, 0x0C, 0x87, 0xC3, 0xC1, 0x6B, 0x54, 0x54, 0x54,
 0xBC, 0xE9, 0xEB, 0xEB, 0x93, 0x5F, 0x5C, 0x5C, 0x30, 0x8A, 0x9D, 0x2E, 0x2B, 0x2B, 0xBB, 0xA2,
 0x3E, 0x41, 0xBD, 0x21, 0x1E, 0x8F, 0x63, 0x6A, 0x6A, 0x0A, 0x81, 0x40, 0x00, 0x94, 0x1B, 0x3D,
 0x3D, 0x3D, 0x42, 0x3C, 0x96, 0x96, 0x96, 0x70, 0x7E, 0x7E, 0x8E, 0xE3, 0xE3, 0x63, 0xF8, 0xFD,
 0xFE, 0xB4, 0xD7, 0xEB, 0xF5, 0x8F, 0x8F, 0x8F, 0x5B, 0x68, 0x5E, 0x6F, 0x05, 0xCE, 0xB4, 0xE3,
 0xE8, 0xE8, 0x08, 0x27, 0x27, 0x27, 0xD8, 0xDF, 0xDF, 0xC7, 0xD9, 0xD9, 0x19, 0x6C, 0x36, 0x1B,
 0x36, 0x36, 0x36, 0x38, 0x9F, 0x85, 0x85, 0x05, 0xAC, 0xAF, 0xAF, 0x23, 0x1A, 0x8D, 0x22, 0x91,
 0x48, 0x20, 0x16, 0x8B, 0xFD, 0xDA, 0xDA, 0xDA, 0x7A, 0x41, 0x33, 0x7E, 0x57, 0x50, 0x50, 0x80,
 0x89, 0x89, 0x09, 0x84, 0xC3, 0x61, 0x6C, 0x6F, 0x6F, 0x23, 0x12, 0x89, 0xE0, 0xE0, 0xE0, 0x00,
 0x43, 0x43, 0x43, 0x58, 0x5E, 0x5E, 0xE6, 0x9C, 0x7D, 0x3E, 0x1F, 0x46, 0x47, 0x47, 0x79, 0xBE,
 0xBD, 0xBD, 0x3D, 0xE1, 0x3C, 0x1D, 0x0C, 0x06, 0x9F, 0x10, 0xB7, 0xC7, 0x84, 0x4F, 0xF6, 0xF7,
 0xF7, 0x63, 0x60, 0x60, 0x00, 0x83, 0x83, 0x83, 0x18, 0x19, 0x19, 0xC1, 0xDC, 0xDC, 0x1C, 0x8F,
 0x17, 0x7C, 0xA4, 0x27, 0xE7, 0x34, 0x39, 0x39, 0x89, 0x9D, 0x9D, 0x1D, 0x6E, 0x54, 0xE3, 0x13,
 0xE5, 0x34, 0x11, 0x37, 0x49, 0x51, 0x51, 0xD1, 0x4B, 0xA5, 0x52, 0xF9, 0x45, 0x26, 0x93, 0x5D,
 0x0A, 0xF3, 0x92, 0x48, 0x24, 0xA0, 0x6F, 0x14, 0x17, 0x17, 0xA3, 0xB6, 0xB6, 0x16, 0x5D, 0x5D,
 0x5D, 0x7C, 0x1E, 0xBB, 0xBB, 0xBB, 0x9C, 0xD7, 0xE1, 0xE1, 0x21, 0x42, 0xA1, 0xD0, 0x6B, 0xD2,
 0x45, 0x4C, 0x33, 0x12, 0x34, 0xCC, 0xA0, 0x19, 0x54, 0x92, 0x56, 0x0E, 0xD2, 0xD9, 0x43, 0xF8,
 0xCF, 0x82, 0x56, 0xC2, 0xDC, 0xEB, 0xEA, 0xEA, 0x38, 0x7E, 0x6C, 0x6C, 0x4C, 0xE0, 0xFE, 0x9D,
 0xB8, 0xBF, 0xA7, 0xFA, 0xAF, 0x56, 0x56, 0x56, 0xEE, 0x6D, 0x6E, 0x6E, 0xDE, 0xB8, 0x47, 0x55,
 0x55, 0x55, 0x6C, 0x66, 0x66, 0x46, 0x44, 0xDA, 0x3B, 0x34, 0x1A, 0x4D, 0x94, 0xB0, 0x3F, 0x09,
 0x7B, 0x45, 0xBD, 0xA5, 0x5D, 0x2E, 0x57, 0x8C, 0x7A, 0x73, 0xD9, 0xED, 0xF6, 0x3B, 0x84, 0xFF,
 0xE7, 0x7D, 0xA6, 0x3A, 0x2C, 0x95, 0x4A, 0xB1, 0x8E, 0x8E, 0x0E, 0x6D, 0x77, 0x77, 0xB7, 0xCD,
 0xE9, 0x74, 0x3E, 0x73, 0xBB, 0xDD, 0x8F, 0x3C, 0x1E, 0x8F, 0xE6, 0xF4, 0xF4, 0x94, 0xAD, 0xAD,
 0xAD, 0xDD, 0xDE, 0xCF, 0x73, 0x0B, 0x0B, 0xB8, 0xB6, 0xE0, 0x5D, 0xC6, 0x66, 0xC5, 0xE4, 0x10,
 0x4C, 0xF4, 0xF7, 0xD8, 0x59, 0xF2, 0x7F, 0xA3, 0xB8, 0xB4, 0xFC, 0x0F, 0xEE, 0x37, 0x70, 0xEC,
 0x16, 0x4A, 0x7E, 0x04, 0x00, 0x00
};




#define favicon_ico_gz_p1_len 368
const uint8_t favicon_ico_gz_p1[] PROGMEM = {
 0x1F, 0x8B, 0x08, 0x08, 0x0B, 0x87, 0x90, 0x57, 0x00, 0x03, 0x66, 0x61, 0x76, 0x69, 0x63, 0x6F,
 0x6E, 0x2E, 0x69, 0x63, 0x6F, 0x00, 0xCD, 0x53, 0x5F, 0x48, 0x9A, 0x51, 0x14, 0xBF, 0x62, 0x6D,
 0x86, 0x96, 0xA9, 0x64, 0xD3, 0xFE, 0xA8, 0x99, 0x65, 0x1A, 0xB4, 0x8A, 0xA8, 0x51, 0x54, 0x23,
 0xA8, 0x11, 0x49, 0x51, 0x8A, 0x34, 0x62, 0x93, 0x85, 0x31, 0x58, 0x44, 0x12, 0x45, 0x2D, 0x58,
 0xF5, 0x52, 0x41, 0x10, 0x23, 0x82, 0xA0, 0x20, 0x98, 0x2F, 0xC1, 0x26, 0xED, 0xA1, 0x20, 0x89,
 0x04, 0xD7, 0x83, 0x58, 0x20, 0x28, 0x04, 0xAB, 0xD1, 0x9B, 0x8C, 0xE5, 0xC3, 0x60, 0x32, 0x64,
 0x0E, 0x56, 0xBF, 0x9D, 0xEF, 0xF6, 0x30, 0x82, 0xED, 0xAD, 0x87, 0xDD, 0x8F, 0xF3, 0xDD, 0x8F,
 0x73, 0xCF, 0xEF, 0x9C, 0xDF, 0x39, 0xBF, 0xFB, 0x31, 0x26, 0xA2, 0x27, 0x37, 0x97, 0xD1, 0x5B,
 0xCF, 0x9E, 0x67, 0x30, 0xA6, 0x66, 0x8C, 0x99, 0xC9, 0xC8, 0x45, 0x9E, 0x6B, 0x3F, 0x5F, 0x74,
 0xA6, 0x94, 0x5E, 0xDB, 0xFF, 0xB2, 0xE6, 0xE7, 0xE7, 0xF9, 0xDE, 0xD6, 0xD6, 0x96, 0xDB, 0xD8,
 0xD8, 0x78, 0xBF, 0xA1, 0xA1, 0xC1, 0xDA, 0xDC, 0xDC, 0x2C, 0xEB, 0xED, 0xED, 0x15, 0x9B, 0xCD,
 0xE6, 0x4A, 0x83, 0xC1, 0xE0, 0x2E, 0x29, 0x29, 0x99, 0xD6, 0x6A, 0xB5, 0x4F, 0x75, 0x3A, 0x9D,
 0x61, 0x75, 0x75, 0x95, 0xB5, 0xB7, 0xB7, 0xDF, 0xC8, 0xD1, 0xD4, 0xD4, 0xF4, 0xB0, 0xBA, 0xBA,
 0xFA, 0x83, 0xD5, 0x6A, 0xFD, 0x5A, 0x5E, 0x5E, 0x9E, 0x28, 0x2D, 0x2D, 0x0D, 0x10, 0xC6, 0x4B,
 0x98, 0x78, 0x5E, 0x5E, 0xDE, 0x95, 0x42, 0xA1, 0x40, 0x4E, 0x4E, 0xCE, 0x65, 0x76, 0x76, 0xF6,
 0x47, 0xB5, 0x5A, 0x6D, 0x4F, 0x26, 0x93, 0xA2, 0xD6, 0xD6, 0x56, 0x8E, 0x6D, 0x69, 0x69, 0xD1,
 0x11, 0x36, 0x62, 0xB1, 0x58, 0x60, 0x32, 0x99, 0xA0, 0xD7, 0xEB, 0x51, 0x58, 0x58, 0x88, 0xFC,
 0xFC, 0x7C, 0x10, 0x16, 0x02, 0x56, 0x2E, 0x97, 0x43, 0x2A, 0x95, 0x42, 0x2C, 0x16, 0x23, 0x33,
 0x33, 0x33, 0xAE, 0x52, 0xA9, 0x1E, 0x64, 0x65, 0x65, 0x71, 0x7C, 0x7D, 0x7D, 0xBD, 0x93, 0xEA,
 0xFE, 0x30, 0x1A, 0x8D, 0xE8, 0xEC, 0xEC, 0xC4, 0xE2, 0xE2, 0x22, 0x6A, 0x6A, 0x6A, 0x40, 0x39,
 0x41, 0xB5, 0x38, 0x4E, 0xC8, 0x33, 0x3C, 0x3C, 0x0C, 0x87, 0xC3, 0xC1, 0x6B, 0x54, 0x54, 0x54,
 0xBC, 0xE9, 0xEB, 0xEB, 0x93, 0x5F, 0x5C, 0x5C, 0x30, 0x8A, 0x9D, 0x2E, 0x2B, 0x2B, 0xBB, 0xA2,
 0x3E, 0x41, 0xBD, 0x21, 0x1E, 0x8F, 0x63, 0x6A, 0x6A, 0x0A, 0x81, 0x40, 0x00, 0x94, 0x1B, 0x3D
};

#define favicon_ico_gz_p2_len 358
const uint8_t favicon_ico_gz_p2[] PROGMEM = {
 0x3D, 0x3D, 0x42, 0x3C, 0x96, 0x96, 0x96, 0x70, 0x7E, 0x7E, 0x8E, 0xE3, 0xE3, 0x63, 0xF8, 0xFD,
 0xFE, 0xB4, 0xD7, 0xEB, 0xF5, 0x8F, 0x8F, 0x8F, 0x5B, 0x68, 0x5E, 0x6F, 0x05, 0xCE, 0xB4, 0xE3,
 0xE8, 0xE8, 0x08, 0x27, 0x27, 0x27, 0xD8, 0xDF, 0xDF, 0xC7, 0xD9, 0xD9, 0x19, 0x6C, 0x36, 0x1B,
 0x36, 0x36, 0x36, 0x38, 0x9F, 0x85, 0x85, 0x05, 0xAC, 0xAF, 0xAF, 0x23, 0x1A, 0x8D, 0x22, 0x91,
 0x48, 0x20, 0x16, 0x8B, 0xFD, 0xDA, 0xDA, 0xDA, 0x7A, 0x41, 0x33, 0x7E, 0x57, 0x50, 0x50, 0x80,
 0x89, 0x89, 0x09, 0x84, 0xC3, 0x61, 0x6C, 0x6F, 0x6F, 0x23, 0x12, 0x89, 0xE0, 0xE0, 0xE0, 0x00,
 0x43, 0x43, 0x43, 0x58, 0x5E, 0x5E, 0xE6, 0x9C, 0x7D, 0x3E, 0x1F, 0x46, 0x47, 0x47, 0x79, 0xBE,
 0xBD, 0xBD, 0x3D, 0xE1, 0x3C, 0x1D, 0x0C, 0x06, 0x9F, 0x10, 0xB7, 0xC7, 0x84, 0x4F, 0xF6, 0xF7,
 0xF7, 0x63, 0x60, 0x60, 0x00, 0x83, 0x83, 0x83, 0x18, 0x19, 0x19, 0xC1, 0xDC, 0xDC, 0x1C, 0x8F,
 0x17, 0x7C, 0xA4, 0x27, 0xE7, 0x34, 0x39, 0x39, 0x89, 0x9D, 0x9D, 0x1D, 0x6E, 0x54, 0xE3, 0x13,
 0xE5, 0x34, 0x11, 0x37, 0x49, 0x51, 0x51, 0xD1, 0x4B, 0xA5, 0x52, 0xF9, 0x45, 0x26, 0x93, 0x5D,
 0x0A, 0xF3, 0x92, 0x48, 0x24, 0xA0, 0x6F, 0x14, 0x17, 0x17, 0xA3, 0xB6, 0xB6, 0x16, 0x5D, 0x5D,
 0x5D, 0x7C, 0x1E, 0xBB, 0xBB, 0xBB, 0x9C, 0xD7, 0xE1, 0xE1, 0x21, 0x42, 0xA1, 0xD0, 0x6B, 0xD2,
 0x45, 0x4C, 0x33, 0x12, 0x34, 0xCC, 0xA0, 0x19, 0x54, 0x92, 0x56, 0x0E, 0xD2, 0xD9, 0x43, 0xF8,
 0xCF, 0x82, 0x56, 0xC2, 0xDC, 0xEB, 0xEA, 0xEA, 0x38, 0x7E, 0x6C, 0x6C, 0x4C, 0xE0, 0xFE, 0x9D,
 0xB8, 0xBF, 0xA7, 0xFA, 0xAF, 0x56, 0x56, 0x56, 0xEE, 0x6D, 0x6E, 0x6E, 0xDE, 0xB8, 0x47, 0x55,
 0x55, 0x55, 0x6C, 0x66, 0x66, 0x46, 0x44, 0xDA, 0x3B, 0x34, 0x1A, 0x4D, 0x94, 0xB0, 0x3F, 0x09,
 0x7B, 0x45, 0xBD, 0xA5, 0x5D, 0x2E, 0x57, 0x8C, 0x7A, 0x73, 0xD9, 0xED, 0xF6, 0x3B, 0x84, 0xFF,
 0xE7, 0x7D, 0xA6, 0x3A, 0x2C, 0x95, 0x4A, 0xB1, 0x8E, 0x8E, 0x0E, 0x6D, 0x77, 0x77, 0xB7, 0xCD,
 0xE9, 0x74, 0x3E, 0x73, 0xBB, 0xDD, 0x8F, 0x3C, 0x1E, 0x8F, 0xE6, 0xF4, 0xF4, 0x94, 0xAD, 0xAD,
 0xAD, 0xDD, 0xDE, 0xCF, 0x73, 0x0B, 0x0B, 0xB8, 0xB6, 0xE0, 0x5D, 0xC6, 0x66, 0xC5, 0xE4, 0x10,
 0x4C, 0xF4, 0xF7, 0xD8, 0x59, 0xF2, 0x7F, 0xA3, 0xB8, 0xB4, 0xFC, 0x0F, 0xEE, 0x37, 0x70, 0xEC,
 0x16, 0x4A, 0x7E, 0x04, 0x00, 0x00
};





// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.bmp"

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

  <div><img src="saved-photo" id="photo" width="10%"></div>


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

  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

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
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // from Camera Counter OCR
  config.frame_size = FRAMESIZE_QQVGA;//FRAMESIZE_QQVGA;//FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_RGB565;//PIXFORMAT_GRAYSCALE;
  config.fb_count = 1;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.deepSleep(3600000000);
  }

  // Prepare bitmap header
  if(config.frame_size == FRAMESIZE_QVGA){
	  BMP::construct16BitHeader(bmpHeader, 320, 240);	// ! constants
  }
  else if(config.frame_size == FRAMESIZE_QQVGA){
	  BMP::construct16BitHeader(bmpHeader, 160, 120);	// ! constants
  }
  else{
	  BMP::construct16BitHeader(bmpHeader, 100, 100);	// ! constants
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

	  //request->send(SD_MMC, FILE_PHOTO, "image/bmp", false);

	  //AsyncWebServerResponse *response = request->beginResponse_P(200, "image/bmp", favicon_ico_gz, favicon_ico_gz_len);
	  //response->addHeader("Content-Encoding", "gzip");

	  //AsyncWebServerResponse *response = request->beginChunkedResponse("image/x-icon", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
	    //Write up to "maxLen" bytes into "buffer" and return the amount written.
	    //index equals the amount of bytes that have been already sent
	    //You will be asked for more data until 0 is returned
	    //Keep in mind that you can not delay or yield waiting for more data!
	    //return mySource.read(buffer, maxLen);
	  //});
	  //response->addHeader("Content-Encoding", "gzip");
	  //request->send(response);



	  // that one works
	  //AsyncResponseStream *response = request->beginResponseStream("image/x-icon");
	  //response->addHeader("Content-Encoding", "gzip");
	  //response->write(favicon_ico_gz, favicon_ico_gz_len);

	  // that one works
	  //AsyncResponseStream *response = request->beginResponseStream("image/x-icon");
	  //response->addHeader("Content-Encoding", "gzip");
	  //response->write(favicon_ico_gz_p1, favicon_ico_gz_p1_len);
	  //response->write(favicon_ico_gz_p2, favicon_ico_gz_p2_len);


	  // test

	  Serial.printf("sizeof(fb->buf)=%i, sizeof(favicon_ico_gz)=%i\r\n",sizeof(fb->buf), sizeof(favicon_ico_gz));
	  //Serial.printf("Camera capture done. W=%i, H=%i, bytes=%i\r\n",fb->width, fb->height, fb->len);

	  //Serial.println("beginResponseStream...");
	  //AsyncResponseStream *response = request->beginResponseStream("image/x-windows-bmp");
	  //response->addHeader("Content-Disposition", "inline; filename=capture.bmp");
	  //response->write(bmpHeader, BMP::headerSize);


      // Chunked response, we calculate the chunks based on free heap (in multiples of 32)
      // This is necessary when a TLS connection is open since it sucks too much memory
	  // https://github.com/helderpe/espurna/blob/76ad9cde5a740822da9fe6e3f369629fa4b59ebc/code/espurna/web.ino
	  Serial.printf(PSTR("[MAIN] Free heap: %d bytes\n"), ESP.getFreeHeap());
      size_t max = (ESP.getFreeHeap() / 3) & 0xFFE0;

	  AsyncWebServerResponse *response = request->beginChunkedResponse("image/x-icon",[max](uint8_t *buffer, size_t maxLen, size_t index) -> size_t{

          // Get the chunk based on the index and maxLen
          size_t len = favicon_ico_gz_len - index;
          if (len > maxLen) len = maxLen;
          if (len > max) len = max;
          if (len > 0) memcpy_P(buffer, favicon_ico_gz + index, len);

          Serial.printf(PSTR("[WEB] Sending %d%%%% (max chunk size: %4d)\r"), int(100 * index / favicon_ico_gz_len), max);
          if (len == 0) Serial.printf(PSTR("\n"));

          // Return the actual length of the chunk (0 for end of file)
          return len;

      });

	  response->addHeader("Content-Encoding", "gzip");
	  request->send(response);

  });

  // Start server
  server.begin();
}


int genJsonLog(uint8_t *buffer, size_t maxLen, size_t index)
{
	int sz = 0;

	if(0 == index){
		buffer = (uint8_t*)favicon_ico_gz_p1;
		sz = favicon_ico_gz_p1_len;
	}
	else{
		buffer = (uint8_t*)favicon_ico_gz_p2;
		sz = favicon_ico_gz_p2_len;
	}
	return (sz > 0) ? sz : 0;
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
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    // Turns on the ESP32-CAM white on-board LED (flash) connected to GPIO 4
    //pinMode(BUILD_IN_LED, OUTPUT);
    //digitalWrite(BUILD_IN_LED, HIGH);
    //delay(10);

    esp_camera_fb_return(fb);

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    Serial.printf("Camera capture done. W=%i, H=%i\r\n",fb->width, fb->height);

//    Image.SetSize(fb->width, fb->height);
//    Serial.println("Bitmap initialized");
//
//    Image.SetBitDepth(8);
//    Serial.println("Bitmap SetBitDepth done");
//
//    CreateGrayscaleColorTable(Image);
//    Serial.println("Bitmap template prepared");
//
//
//    // convert each pixel to greyscale
//    for( int i=0 ; i < Image.TellWidth() ; i++)
//    {
//		for( int j=0 ; j < Image.TellHeight() ; j++)
//		{
//
//			uint32_t l = (j * fb->width + i);
//
//			Image(i,j)->Red   = fb->buf[l];
//			Image(i,j)->Green = fb->buf[l];
//			Image(i,j)->Blue  = fb->buf[l];
//
//			//*Image(i,j) = fb->buf[l];
//		}
//    }
//
//    Serial.println("Bitmap data filled");



    // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
    //pinMode(BUILD_IN_LED, INPUT);
    //digitalWrite(BUILD_IN_LED, LOW);


    // check if file has been correctly saved in SPIFFS
    ok = 1;
  } while ( !ok );
}
