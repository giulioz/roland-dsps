#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "../emulator/emulator.h"

static int open_serial(const char *path) {
  int fd = open(path, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    perror("open serial");
    return -1;
  }

  struct termios tty;
  if (tcgetattr(fd, &tty) != 0) {
    perror("tcgetattr");
    close(fd);
    return -1;
  }

  // configure raw mode
  cfmakeraw(&tty);

  // set baud rate (match your arduino sketch)
  // cfsetspeed(&tty, B115200);
  cfsetspeed(&tty, 921600);

  tty.c_cflag |=
      (CLOCAL | CREAD); // ignore modem control lines, enable receiver
  tty.c_cc[VMIN] = 1;   // wait for at least 1 byte
  tty.c_cc[VTIME] = 5;  // timeout: 0.5s

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
    close(fd);
    return -1;
  }

  // flush the read buffer
  tcflush(fd, TCIFLUSH);

  return fd;
}

static uint8_t read_byte(int fd) {
  uint8_t b;
  int n = read(fd, &b, 1);
  if (n != 1) {
    perror("read");
    return 0xFF; // or throw
  }
  return b;
}

static void write_bytes(int fd, const uint8_t *data, size_t len) {
  size_t total = 0;
  while (total < len) {
    ssize_t n = write(fd, data + total, len - total);
    if (n < 0) {
      perror("write");
      break;
    }
    total += n;
  }
}

int serial_fd = -1;

uint8_t dsp_r(uint32_t ofs) {
  uint8_t buf[4] = {0};
  buf[0] = 'r';
  buf[1] = (ofs >> 8) & 0xff;
  buf[2] = ofs & 0xff;
  buf[3] = 0x00;
  write_bytes(serial_fd, buf, 4);
  uint8_t r = read_byte(serial_fd);

  return r;
}

void dsp_w(uint32_t ofs, uint8_t data) {
  uint8_t buf[4] = {0};
  buf[0] = 'w';
  buf[1] = (ofs >> 8) & 0xff;
  buf[2] = ofs & 0xff;
  buf[3] = data;
  write_bytes(serial_fd, buf, 4);
  read_byte(serial_fd);
}

void dsp_pgm_w(uint32_t pos, uint32_t instr, uint16_t coef) {
  dsp_w(0x1000 + pos * 4 + 0, instr & 0xff);
  dsp_w(0x1000 + pos * 4 + 1, (instr >> 8) & 0xff);
  dsp_w(0x1000 + pos * 4 + 2, (instr >> 16) & 0xff);

  dsp_w(0x0000 + pos * 2 + 0, coef & 0xff);
  dsp_w(0x0000 + pos * 2 + 1, (coef >> 8) & 0xff);
}

Emulator *emulator;

void fuzz() {
  while (true) {
    emulator->clear();

    uint16_t possible_mems[] = {0x70, 0x01, 0x02};
    uint8_t possible_accs[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                               0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    uint8_t possible_mulmodes[] = {
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    };

    // uint8_t rnd_opcode = rand() % 16;
    uint8_t rnd_opcode = possible_accs[rand() % sizeof(possible_accs)];
    uint8_t rnd_mulshift = rand() % 4;
    uint16_t rnd_memslot = possible_mems[rand() % (sizeof(possible_mems) / 2)];
    uint16_t rnd_coef = rand() & 0xffff;
    uint8_t rnd_mulmode = possible_mulmodes[rand() % sizeof(possible_mulmodes)];

    uint8_t rnd_mulshift_mem = rand() % 4;
    uint8_t rnd_immshift_mem = (rand() % 2) + 1;
    uint16_t rnd_memval = rand() & 0xffff;

    uint8_t rnd_mulshift_accA = rand() % 4;
    uint8_t rnd_immshift_accA = (rand() % 2) + 1;
    uint16_t rnd_accA = rand() & 0xffff;

    uint8_t rnd_mulshift_accB = rand() % 4;
    uint8_t rnd_immshift_accB = (rand() % 2) + 1;
    uint16_t rnd_accB = rand() & 0xffff;

    uint8_t rnd_mulshift_mul186 = rand() % 4;
    uint8_t rnd_immshift_mul186 = (rand() % 2) + 1;
    uint16_t rnd_mul186 = rand() & 0xffff;

    uint8_t rnd_mulshift_mul187 = rand() % 4;
    uint8_t rnd_immshift_mul187 = (rand() % 2) + 1;
    uint16_t rnd_mul187 = rand() & 0xffff;

    uint8_t rnd_dest_187 = (rand() & 1) != 0 ? 0x87 : 0x85;
    uint8_t rnd_accSel = (rand() & 1) != 0;
    uint8_t rnd_saturate =
        (rand() & 1) != 0 ? (rnd_accSel ? 0x8 : 0xa) : (rnd_accSel ? 0xc : 0xe);

    rnd_opcode = 0x5;
    // rnd_mulmode = 2;
    // rnd_mulshift = 0x01;
    // rnd_memslot = 0x70;
    // rnd_coef = 0x1200;

    // rnd_mulshift_mem = 0x00;
    // rnd_immshift_mem = 0x01;
    // rnd_memval = 0x1234;

    // rnd_mulshift_accA = 0x00;
    // rnd_immshift_accA = 0x01;
    rnd_accA = 0x8000;

    // rnd_mulshift_accB = 0x00;
    // rnd_immshift_accB = 0x01;
    // rnd_accB = 0x0000;

    // rnd_mulshift_mul186 = 0x02;
    // rnd_immshift_mul186 = 0x02;
    // rnd_mul186 = 0x48ad;

    // rnd_mulshift_mul187 = 0x03;
    // rnd_immshift_mul187 = 0x02;
    // rnd_mul187 = 0xb819;

    // rnd_dest_187 = 0x87;
    rnd_saturate = 0x0c;

    int pos = 0;
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x20, 0x01, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x30, 0x01, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

    // save to mem
    emulator->writePgm(pos++, rnd_mulshift_mem, 0x20, rnd_immshift_mem,
                       rnd_memval);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x08, 0x70, 0x0000);

    // s186
    emulator->writePgm(pos++, rnd_mulshift_mul186, 0x20, rnd_immshift_mul186,
                       rnd_mul186);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x05, 0x86, 0x0000);

    // s187
    emulator->writePgm(pos++, rnd_mulshift_mul187, 0x20, rnd_immshift_mul187,
                       rnd_mul187);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x05, rnd_dest_187, 0x0000);

    // accA/accB
    emulator->writePgm(pos++, rnd_mulshift_accA, 0x20, rnd_immshift_accA,
                       rnd_accA);
    emulator->writePgm(pos++, rnd_mulshift_accB, 0x30, rnd_immshift_accB,
                       rnd_accB);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

    // test
    if (rnd_opcode >= 0xc) {
      emulator->writePgm(pos++, rnd_mulshift,
                         ((rnd_memslot >> 8) & 1) | (rnd_opcode << 4),
                         rnd_memslot, rnd_mulmode);
    } else {
      emulator->writePgm(pos++, rnd_mulshift,
                         ((rnd_memslot >> 8) & 1) | (rnd_opcode << 4),
                         rnd_memslot, rnd_coef);
    }
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, rnd_saturate, 0x70, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

    emulator->writePgm(pos++, 0x01, 0x20, 0x70, 0x4000); // readback
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
    emulator->writePgm(pos++, 0x00, 0x07, 0x82, 0x0000);

    emulator->runProgram();
    int32_t result_emu = emulator->hostReadback;

    for (size_t i = 0; i < pos + 20; i++) {
      dsp_pgm_w(i,
                (emulator->instr0[i] << 16) | (emulator->instr1[i] << 8) |
                    (emulator->instr2[i] << 0),
                emulator->coefs[i]);
    }

    dsp_w(0x2000 | 0x802, 0x00);
    int32_t result =
        sign_extend<24>((dsp_r(0x02) << 16) | (dsp_r(0x01) << 8) | dsp_r(0x00));

    printf("opcode: %x   hw: %06x    emu: %06x\n", rnd_opcode,
           result & 0xffffff, result_emu & 0xffffff);

    if ((result & 0xffffff) != (result_emu & 0xffffff)) {
      printf("Error detected!\n");
      printf("opcode: %02x\n", rnd_opcode);
      printf("mulshift: %02x\n", rnd_mulshift);
      printf("memslot: %02x\n", rnd_memslot);
      printf("coef: %04x\n", rnd_coef);
      printf("mulshift_mem: %02x\n", rnd_mulshift_mem);
      printf("immshift_mem: %02x\n", rnd_immshift_mem);
      printf("memval: %04x\n", rnd_memval);
      printf("mulshift_accA: %02x\n", rnd_mulshift_accA);
      printf("immshift_accA: %02x\n", rnd_immshift_accA);
      printf("accA: %04x\n", rnd_accA);
      printf("mulshift_accB: %02x\n", rnd_mulshift_accB);
      printf("immshift_accB: %02x\n", rnd_immshift_accB);
      printf("accB: %04x\n", rnd_accB);
      printf("mulshift_mul186: %02x\n", rnd_mulshift_mul186);
      printf("immshift_mul186: %02x\n", rnd_immshift_mul186);
      printf("mul186: %04x\n", rnd_mul186);
      printf("mulshift_mul187: %02x\n", rnd_mulshift_mul187);
      printf("immshift_mul187: %02x\n", rnd_immshift_mul187);
      printf("mul187: %04x\n", rnd_mul187);
      printf("mulmode: %04x\n", rnd_mulmode);
      printf("dest_187: %02x\n", rnd_dest_187);
      printf("saturate: %02x\n", rnd_saturate);

      // break;
    }
  }
}

void test_mem() {
  emulator->clear();

  int pos = 0;

  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  // ram write
  emulator->writePgm(pos++, 0x48, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x01, 0x20, 0x05, 0x4000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x01, 0x0001);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x08, 0x04, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x05, 0x83, 0x0000); // eram write latch
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000); // (eram write high)
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  // ram read
  emulator->writePgm(pos++, 0x08, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x40, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x10, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x01, 0x27, 0xf0, 0x4000); // read to iram
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  emulator->writePgm(pos++, 0x01, 0x20, 0x04, 0x4000);
  emulator->writePgm(pos++, 0x01, 0x01, 0xf0, 0xc000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x07, 0x82, 0x0000);

  pos += 60;

  for (size_t i = 0; i < 0x20000; i++) {
    emulator->runProgram();
  }
  int32_t result_emu = emulator->hostReadback;

  for (size_t i = 0; i < pos; i++) {
    dsp_pgm_w(i,
              (emulator->instr0[i] << 16) | (emulator->instr1[i] << 8) |
                  (emulator->instr2[i] << 0),
              emulator->coefs[i]);
  }

  while (true) {
    dsp_w(0x2000 | 0x802, 0x00);
    int32_t result =
        sign_extend<24>((dsp_r(0x02) << 16) | (dsp_r(0x01) << 8) | dsp_r(0x00));
    printf("hw: %06x    emu: %06x\n", result & 0xffffff, result_emu & 0xffffff);
  }
}

void test_mem2() {
  emulator->clear();

  int pos = 0;

  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  emulator->writePgm(pos++, 0x00, 0x20, 0x01, 0xffff);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x05, 0x85, 0x0000);

  emulator->writePgm(pos++, 0x08, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  emulator->writePgm(pos++, 0xc8, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  pos += 60;

  for (size_t i = 0; i < pos; i++) {
    dsp_pgm_w(i,
              (emulator->instr0[i] << 16) | (emulator->instr1[i] << 8) |
                  (emulator->instr2[i] << 0),
              emulator->coefs[i]);
  }
}

void test_50() {
  emulator->clear();

  int pos = 0;

  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  // mem
  emulator->writePgm(pos++, 0x00, 0x20, 0x01, 0x1234);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x0c, 0x70, 0x0000);
  
  emulator->writePgm(pos++, 0x00, 0x20, 0x01, 0xa000);
  emulator->writePgm(pos++, 0x00, 0x30, 0x01, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  // test
  emulator->writePgm(pos++, 0x00, 0x50, 0x70, 0x1234);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x0c, 0x70, 0x0000);

  emulator->writePgm(pos++, 0x01, 0x20, 0x70, 0x4000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x07, 0x82, 0x0000);

  pos += 60;

  for (size_t i = 0; i < 40; i++) {
    emulator->runProgram();
  }
  int32_t result_emu = emulator->hostReadback;

  for (size_t i = 0; i < pos; i++) {
    dsp_pgm_w(i,
              (emulator->instr0[i] << 16) | (emulator->instr1[i] << 8) |
                  (emulator->instr2[i] << 0),
              emulator->coefs[i]);
  }

  dsp_w(0x2000 | 0x802, 0x00);
  int32_t result =
      sign_extend<24>((dsp_r(0x02) << 16) | (dsp_r(0x01) << 8) | dsp_r(0x00));
  printf("hw: %06x    emu: %06x\n", result & 0xffffff, result_emu & 0xffffff);
}

void test_02() {
  emulator->clear();

  int pos = 0;

  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x20, 0x01, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x30, 0x01, 0x0000);

  // mul186
  emulator->writePgm(pos++, 0x00, 0x20, 0x01, 0xfef0);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x07, 0x85, 0x0000);

  // mem
  emulator->writePgm(pos++, 0x00, 0x20, 0x02, 0x44e7);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x0c, 0x05, 0x0000);

  emulator->writePgm(pos++, 0x00, 0xc0, 0x05, 0x0003);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x07, 0x82, 0x0000);

  pos += 60;

  for (size_t i = 0; i < 40; i++) {
    emulator->runProgram();
  }
  int32_t result_emu = emulator->hostReadback;

  for (size_t i = 0; i < pos; i++) {
    dsp_pgm_w(i,
              (emulator->instr0[i] << 16) | (emulator->instr1[i] << 8) |
                  (emulator->instr2[i] << 0),
              emulator->coefs[i]);
  }

  dsp_w(0x2000 | 0x802, 0x00);
  int32_t result =
      sign_extend<24>((dsp_r(0x02) << 16) | (dsp_r(0x01) << 8) | dsp_r(0x00));
  printf("hw: %06x    emu: %06x\n", result & 0xffffff, result_emu & 0xffffff);
}

void test_02_tbl() {
  emulator->clear();

  int pos = 0;

  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);

  // mul186
  emulator->writePgm(pos++, 0x00, 0x20, 0x02, 0x1234);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x07, 0x86, 0x0000);

  // mem
  emulator->writePgm(pos++, 0x00, 0x20, 0x02, 0x1234);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x0c, 0x05, 0x0000);

  emulator->writePgm(pos++, 0x00, 0xc0, 0x05, 0x0002);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x00, 0x00, 0x0000);
  emulator->writePgm(pos++, 0x00, 0x07, 0x82, 0x0000);

  for (size_t i = 0; i < pos; i++) {
    dsp_pgm_w(i,
              (emulator->instr0[i] << 16) | (emulator->instr1[i] << 8) |
                  (emulator->instr2[i] << 0),
              emulator->coefs[i]);
  }

  for (size_t i = 0; i < 32; i++) {
    for (size_t j = 0; j < 32; j++) {
      dsp_pgm_w(1,
                (emulator->instr0[1] << 16) | (emulator->instr1[1] << 8) | 0x01,
                sign_extend<16>(i * 0x739));
      dsp_pgm_w(5,
                (emulator->instr0[5] << 16) | (emulator->instr1[5] << 8) | 0x01,
                sign_extend<16>(j * 0x739));

      dsp_w(0x2000 | 0x802, 0x00);
      int32_t result = sign_extend<24>((dsp_r(0x02) << 16) |
                                       (dsp_r(0x01) << 8) | dsp_r(0x00));
      printf("mul:%08x  mem:%08x  hw: %06x\n", sign_extend<16>(i * 0x739) << 0,
             sign_extend<16>(j * 0x739) << 0, result & 0xffffff);
    }
  }
  for (size_t i = 0; i < 32; i++) {
    for (size_t j = 0; j < 32; j++) {
      dsp_pgm_w(1,
                (emulator->instr0[1] << 16) | (emulator->instr1[1] << 8) | 0x02,
                sign_extend<16>(i * 0x739));
      dsp_pgm_w(5,
                (emulator->instr0[5] << 16) | (emulator->instr1[5] << 8) | 0x01,
                sign_extend<16>(j * 0x739));

      dsp_w(0x2000 | 0x802, 0x00);
      int32_t result = sign_extend<24>((dsp_r(0x02) << 16) |
                                       (dsp_r(0x01) << 8) | dsp_r(0x00));
      printf("mul:%08x  mem:%08x  hw: %06x\n", sign_extend<16>(i * 0x739) << 7,
             sign_extend<16>(j * 0x739) << 0, result & 0xffffff);
    }
  }
  for (size_t i = 0; i < 32; i++) {
    for (size_t j = 0; j < 32; j++) {
      dsp_pgm_w(1,
                (emulator->instr0[1] << 16) | (emulator->instr1[1] << 8) | 0x01,
                sign_extend<16>(i * 0x739));
      dsp_pgm_w(5,
                (emulator->instr0[5] << 16) | (emulator->instr1[5] << 8) | 0x02,
                sign_extend<16>(j * 0x739));

      dsp_w(0x2000 | 0x802, 0x00);
      int32_t result = sign_extend<24>((dsp_r(0x02) << 16) |
                                       (dsp_r(0x01) << 8) | dsp_r(0x00));
      printf("mul:%08x  mem:%08x  hw: %06x\n", sign_extend<16>(i * 0x739) << 0,
             sign_extend<16>(j * 0x739) << 7, result & 0xffffff);
    }
  }
  for (size_t i = 0; i < 32; i++) {
    for (size_t j = 0; j < 32; j++) {
      dsp_pgm_w(1,
                (emulator->instr0[1] << 16) | (emulator->instr1[1] << 8) | 0x02,
                sign_extend<16>(i * 0x739));
      dsp_pgm_w(5,
                (emulator->instr0[5] << 16) | (emulator->instr1[5] << 8) | 0x02,
                sign_extend<16>(j * 0x739));

      dsp_w(0x2000 | 0x802, 0x00);
      int32_t result = sign_extend<24>((dsp_r(0x02) << 16) |
                                       (dsp_r(0x01) << 8) | dsp_r(0x00));
      printf("mul:%08x  mem:%08x  hw: %06x\n", sign_extend<16>(i * 0x739) << 7,
             sign_extend<16>(j * 0x739) << 7, result & 0xffffff);
    }
  }
}

int main() {
  srand(time(NULL));

  serial_fd = open_serial("/dev/cu.usbmodem1101");

  emulator = new Emulator();

  sleep(1);

  dsp_w(0x0804, 0x00);
  dsp_w(0x0805, 0x00);
  dsp_w(0x0806, 0x07);

  dsp_w(0x0808, 0xf6);
  dsp_w(0x0809, 0x00);
  dsp_w(0x080a, 0x00);

  // fuzz();
  // test_mem();
  test_mem2();
  // test_50();
  // test_02();
  // test_02_tbl();
}
