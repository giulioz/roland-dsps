#include <Arduino.h>

static inline void d_out(uint8_t a) { PORTA = a; }
static inline void a_out_0(uint8_t a) { PORTF = a; }
static inline void a_out_1(uint8_t a) { PORTK = a; }

static inline void _rd_1() { PORTC |= (1 << PC7); }   // 30
static inline void _rd_0() { PORTC &= ~(1 << PC7); }  // 30
static inline void _wr_1() { PORTC |= (1 << PC6); }   // 31
static inline void _wr_0() { PORTC &= ~(1 << PC6); }  // 31
static inline void _cs_1() { PORTC |= (1 << PC5); }   // 32
static inline void _cs_0() { PORTC &= ~(1 << PC5); }  // 32

static inline void wait() {
  asm volatile("nop\n nop\n nop");
  asm volatile("nop\n nop\n nop");
  asm volatile("nop\n nop\n nop");
  // asm volatile("nop\n nop\n nop");
  // asm volatile("nop\n nop\n nop");
}

static inline void bus_begin() {
  DDRA = 0x00;
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;

  _rd_1();
  _wr_1();
  _cs_1();
}

static inline uint8_t bus_read(uint16_t addr) {
  uint8_t result = 0;

  a_out_0(addr & 0xff);
  a_out_1(addr >> 8);
  wait();

  _cs_0();
  _rd_0();
  wait();

  result = PINA;

  _rd_1();
  _cs_1();

  return result;
}

static inline void bus_write(uint16_t addr, uint8_t data) {
  a_out_0(addr & 0xff);
  a_out_1(addr >> 8);
  wait();

  _cs_0();

  DDRA = 0xFF;
  d_out(data);
  wait();
  
  _wr_0();
  wait();

  _wr_1();
  _cs_1();

  DDRA = 0x00;
}


char tmp[64];

void setup() {
  // delay(5000);

  Serial.begin(921600);

  // Serial.print("\n");
  // Serial.println("REBOOT");
  
  bus_begin();
}

void loop() {
  char mode = 0x00;
  uint16_t addr = 0;
  uint8_t data = 0;

  if (Serial.available() == 4) {
    mode = Serial.read();
    addr = Serial.read() << 8;
    addr |= Serial.read();
    data = Serial.read();

    if (mode == 'r') {
      data = bus_read(addr);
      Serial.write(data);
    } else if (mode == 'w') {
      bus_write(addr, data);
      Serial.write(0x00);
    } else {
      Serial.write(0x00);
    }
  }

  // delay(500);
  // Serial.println(bus_read(0), HEX);
}
