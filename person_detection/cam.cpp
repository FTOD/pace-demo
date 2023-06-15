#include "cam.hpp"

// uint8_t img_buf[IMG_BUF_SIZE];
ArduCAM myCAM(OV2640, CS);

void capture()
{
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  Serial.println("capture starts");
}

void read_data(uint8_t* buf, size_t size)
{
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println("waiting for capture...");
    delay(5);
  }
  if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    delay(50);
    read_fifo_burst(myCAM, buf, size);
    // Clear the capture done flag
    myCAM.clear_fifo_flag();
  }
}

void cam_setup()
{
  uint8_t vid, pid;
  Wire.begin();
  Serial.begin(921600);
  while (!Serial)
    ;
  Serial.println(F("ACK CMD ArduCAM Start! END"));
  // set the CS as an output:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  // initialize SPI:
  SPI.begin();
  // Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  uint8_t temp;
  while (1)
  {
    // Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println("ACK CMD SPI interface Error! END");
      delay(1000);
      continue;
    }
    else
    {
      Serial.println("ACK CMD SPI interface OK. END");
      break;
    }
  }
  while (1)
  {
    // Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) && ((pid != 0x41) || (pid != 0x42)))
    {
      Serial.println(F("ACK CMD Can't find OV2640 module! END"));
      delay(1000);
      continue;
    }
    else
    {
      Serial.println(F("ACK CMD OV2640 detected. END"));
      break;
    }
  }
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_160x120);
  // myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  delay(1000);
  myCAM.clear_fifo_flag();
}

// void loop()
// {
//   capture();
//   delay(1000);
//   read_data();
//   for (int i = 0; i < buf_SIZE; i++)
//     Serial.print(buf[i], DEC);
// }

uint8_t read_fifo_burst(ArduCAM myCAM, uint8_t* buf, size_t size)
{
  bool is_header = false;
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  length = myCAM.read_fifo_length();
  Serial.println(length, DEC);
  if (length >= size) // 512 kb
  {
    Serial.println(F("Data Over sized. Abort!"));
    return -1;
  }
  if (length == 0) // 0 kb
  {
    Serial.println(F("FIFO buffer empty. Ignoring"));
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst(); // Set fifo burst mode
  temp = SPI.transfer(0x00);
  length--;
  int i = 0;
  while (length--)
  {
    temp_last = temp;
    temp = SPI.transfer(0x00);
    if (is_header == true)
    {
      buf[i++] = temp;
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;
      Serial.println(F("Image Start Mark detected, transmission continues"));
    }
    if ((temp == 0xD9) && (temp_last == 0xFF))
    {
      Serial.println("Image End Mark detected, transimission completed");
      buf[i++] = temp_last;
      buf[i++] = temp;
      break;
    }
    delayMicroseconds(15);
  }
  myCAM.CS_HIGH();
  is_header = false;
  return 1;
}
