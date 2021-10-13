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

int writeBytes(int fd, size_t bytesToWrite, char *buffer, uint address) {
  // populate the ioctl struct for i2c
  struct i2c_msg msg;
  msg.addr = address;
  // for read
  // for write, write is default
  //  msg.flags = I2C_M_RD;
  msg.flags = 0;

  msg.len = bytesToWrite;
  msg.buf = buffer;

  struct i2c_rdwr_ioctl_data i2cIoct;
  i2cIoct.msgs = &msg;
  i2cIoct.nmsgs = 1;

  
  return ioctl(fd, I2C_RDWR, &i2cIoct);
}


int main(int argc, char **argv) {

  int fd = open(I2C_DEV, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Unable to open device\n");
    exit(1);
  }

  // maybe need to do ioctl, but I think it just tells us what we can or can't do
  // on this bus.
  unsigned long funcs;
  if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
    fprintf(stderr,"Unable to get capabilities\n");
  }

  assert(funcs & I2C_FUNC_I2C);
  

  if (ioctl(fd, I2C_SLAVE, ADDRESS) < 0) {
    fprintf(stderr, "Failed to set slave ioctl\n");
    exit(1);
  }
  char buf[16];
  snprintf(buf,2,"sixteenbytestring");
  int res = writeBytes(fd, 2, &buf[2], ADDRESS);
  if (res < 0) {
    perror("readBytes ioctl error");
  } else {
    fprintf(stderr, "Wrote %d bytes\n", res);
  }
  close(fd);
}
