#include <Arduino.h>

#define PIN_WR   2
#define PIN_RD   3
#define PIN_CS   4

static inline void _addr(uint8_t a) { PORTK = a; }

static inline void _wr_1() { PORTE |= (1 << PE4); }
static inline void _wr_0() { PORTE &= ~(1 << PE4); }
static inline void _rd_1() { PORTE |= (1 << PE5); }
static inline void _rd_0() { PORTE &= ~(1 << PE5); }

static inline void bus_begin() {
  pinMode(PIN_RD, OUTPUT);
  pinMode(PIN_WR, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_RD, HIGH);
  digitalWrite(PIN_WR, HIGH);
  digitalWrite(PIN_CS, HIGH);

  DDRK  = 0xFF;
  DDRF  = 0x00;
}

static inline void bus_write(uint8_t addr, uint8_t data) {
  DDRF = 0xFF;
  _addr(addr);
  PORTF = data;

  asm volatile("nop\n nop");
  _wr_0();
  asm volatile("nop\n nop");
  _wr_1();
}

static uint8_t read_3;
static uint8_t read_2;
static uint8_t read_1;
static uint8_t read_0;

static void read_mem(uint16_t addr) {
  wait_lsp();
  bus_write(9, addr >> 8);
  bus_write(8, addr & 0xff);
  wait_lsp();

  DDRF = 0x00;

  _rd_0();
  asm volatile("nop\n nop");
  _addr(3);
  asm volatile("nop\n nop");
  read_3 = PINF;
  asm volatile("nop\n nop");
  _addr(2);
  asm volatile("nop\n nop");
  read_2 = PINF;
  asm volatile("nop\n nop");
  _addr(1);
  asm volatile("nop\n nop");
  read_1 = PINF;
  asm volatile("nop\n nop");
  _addr(0);
  asm volatile("nop\n nop");
  read_0 = PINF;
  asm volatile("nop\n nop");
  _rd_1();
}

static inline void wait_lsp() {
  DDRF = 0x00;
  _addr(3);
  _rd_0();
  asm volatile("nop");
  while (PINF != 0);
  _rd_1();
}

static inline void halt_lsp() {
  wait_lsp();
  bus_write(3, 0x10);
  bus_write(2, 0x01);
  // bus_write(2, 0x21);
  bus_write(6, 0x00);
}

static inline void start_lsp() {
  wait_lsp();
  bus_write(3, 0x00);
  bus_write(2, 0x01);
  bus_write(6, 0x00);
}

static void clear_all_mem() {
  for (int i = 0; i < 0x1ff; i++) {
    wait_lsp();
    bus_write(4, 0x00);
    bus_write(3, 0x00);
    bus_write(2, 0x00);
    bus_write(1, i >> 8);
    bus_write(0, i & 0xff);
  }
}

static void check_program_zero() {
  for (int i = 0; i < 384; i++) {
    read_mem(i + 0x80);
    if (read_3 != 0 || read_2 != 0 || read_1 != 0 || read_0 != 0) {
      Serial.print("error in ");
      Serial.println(i);
    }
  }
}

static void send_program(uint8_t program[]) {
  for (int i = 383; i >= 0; i--) {
    wait_lsp();
    uint32_t program_ptr = i * 4;
    bus_write(4, program[program_ptr + 0]);
    bus_write(3, program[program_ptr + 1]);
    bus_write(2, program[program_ptr + 2]);
    bus_write(1, (i + 0x80) >> 8);
    bus_write(0, (i + 0x80) & 0xff);
  }
}

static void dump_state() {
  char tmp[16];
  for (int i = 0; i < 0x80; i++) {
    read_mem(i);

    sprintf(tmp, "%02X: ", i);
    Serial.print(tmp);
    
    sprintf(tmp, "%02X", read_3);
    Serial.print(tmp);
    sprintf(tmp, "%02X", read_2);
    Serial.print(tmp);
    sprintf(tmp, "%02X", read_1);
    Serial.print(tmp);
    sprintf(tmp, "%02X", read_0);
    Serial.print(tmp);
    Serial.print("\n");
  }
}

void setup() {
  Serial.begin(115200);

  bus_begin();
  digitalWrite(PIN_CS, LOW);

  halt_lsp();
  clear_all_mem();
}

uint8_t tmp_pgm[384*4];
int readt = 0;

#define N_CYCLES 1

void loop() {
  while (Serial.available() > 0) {
    uint8_t incoming = Serial.read();
    tmp_pgm[readt] = incoming;
    readt++;
  }

  // when entire program is received
  if (readt == 384*4) {
    // upload new program
    halt_lsp();
    clear_all_mem();
    send_program(tmp_pgm);
    
    // run N cycles for debugging, then log the internal state
    start_lsp();
    delayMicroseconds(20 * N_CYCLES);
    halt_lsp();
    dump_state();

    // start again
    start_lsp();
  
    readt = 0;
  }
}
