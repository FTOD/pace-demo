#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

// set pin 7 as the slave select for the digital pot:
const static int CS = 7;
// const static int IMG_BUF_SIZE = 8194;
extern ArduCAM myCam;
void cam_setup();
void capture();
void read_data(uint8_t *buf, size_t size);
uint8_t read_fifo_burst(ArduCAM myCAM, uint8_t *buf, size_t size);
// extern uint8_t img_buf[IMG_BUF_SIZE];
