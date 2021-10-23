#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include <assert.h>
#include <errno.h>

#define I2C_DEV  "/dev/i2c-1"
#define I2C_BUS  1

#define ADDRESS 0x41

static unsigned char reg_addr[] = {0x15};

int ioctlWrite(int fd, size_t bytesToWrite, u_int8_t *buffer, uint address) {
  unsigned char retBuf[1];
  // populate the ioctl struct for i2c
  struct i2c_msg msgs[2];
  // for read
  // for write, write is default, 0
  //  msg.flags = I2C_M_RD;
  msgs[0].flags = 0;
  msgs[0].addr = address;
  msgs[0].buf = buffer;
  msgs[0].len = bytesToWrite;

  /* msgs[1].addr = address; */
  /* msgs[1].flags = I2C_M_RD; */
  /* msgs[1].buf = retBuf; */
  /* msgs[1].len = 1; */

  struct i2c_rdwr_ioctl_data i2cIoct;
  i2cIoct.msgs = &msgs[0];
  i2cIoct.nmsgs = 1;

  int res = ioctl(fd, I2C_RDWR, &i2cIoct);
  if (res < 0) {
      fprintf(stderr, "Write returned: %d\n", res);
      return -1;
  }

  return 0;
}

int smbusWrite(int fd, size_t bytesToWrite, u_int8_t *buffer, uint address) { 
  struct i2c_smbus_ioctl_data smbData;
  smbData.read_write = I2C_SMBUS_WRITE;
  smbData.size = I2C_SMBUS_WORD_DATA;
  smbData.command = address;
  smbData.data = (void*)buffer;


  return ioctl(fd, I2C_SMBUS, &smbData);
}

int writeBytes(int fd, size_t bytesToWrite, u_int8_t *buffer, uint address) {
  // maybe need to do ioctl, but I think it just tells us what we can or can't do
  // on this bus.
  unsigned long funcs;
  if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
    fprintf(stderr,"Unable to get capabilities\n");
  }

  if (funcs & I2C_FUNC_I2C) {
    fprintf(stderr, "Writing ioctl\n");
    return ioctlWrite(fd, bytesToWrite, buffer, address);
  } else   if (funcs & I2C_FUNC_SMBUS_WORD_DATA) {
    fprintf(stderr, "Writing smbus\n");
    return smbusWrite(fd, bytesToWrite, buffer, address);
  } 

  fprintf(stderr, "No supported I2C write\n");
  return -1;
}
  

int main(int argc, char **argv) {

  int fd = open(I2C_DEV, O_RDWR|O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "Unable to open device\n");
    exit(1);
  }

  /* if (ioctl(fd, I2C_SLAVE, ADDRESS) < 0) { */
  /*   fprintf(stderr, "Failed to set slave ioctl\n"); */
  /*   exit(1); */
  /* } */
  u_int8_t buf[16];
  snprintf(buf,16,"sixteenbytestring");
  /* for (int i = 0 ; i < 16 ; ++i) { */
  /*   buf[i] = i*2; */
  /* } */
  int res = 0;
  int tries = 0;
  const int maxTries = 8;
  do {
    res = writeBytes(fd, 6, &buf[6], ADDRESS);
    tries++;
  } while ((res < 0) && (tries < maxTries));
  if (res < 0) {
    perror("readBytes ioctl error");
  } else {
    fprintf(stderr, "Wrote %d bytes\n", res);
  }
  close(fd);
}
