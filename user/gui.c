#include "gui.h"

uint16_t fb[ HEIGHT ][ WIDTH ];
uint8_t mouse_buffer[3]; int byte_number = 0;
uint8_t mx = 0;
uint8_t my = 0;

void configure_LCD() {
  // Configure the LCD display into 800x600 SVGA @ 36MHz resolution.

  SYSCONF->CLCD      = 0x2CAC;     // per per Table 4.3 of datasheet
  LCD->LCDTiming0    = 0x1313A4C4; // per per Table 4.3 of datasheet
  LCD->LCDTiming1    = 0x0505F657; // per per Table 4.3 of datasheet
  LCD->LCDTiming2    = 0x071F1800; // per per Table 4.3 of datasheet

  LCD->LCDUPBASE     = ( uint32_t )( &fb );

  LCD->LCDControl    = 0x00000020; // select TFT   display type
  LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
  LCD->LCDControl   |= 0x00000800; // power-on LCD controller
  LCD->LCDControl   |= 0x00000001; // enable   LCD controller
}

void enable_ps2_interrupts() {
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring then enabling PS/2 controllers st. an interrupt is
   *   raised every time a byte is subsequently received,
   * - configuring GIC st. the selected interrupts are forwarded to the
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  PS20->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS20->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)
  PS21->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS21->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)

  uint8_t ack;

        PL050_putc( PS20, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS20       );  // receive  PS/2 acknowledgement
        PL050_putc( PS21, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS21       );  // receive  PS/2 acknowledgement

  GICC0->PMR         = 0x000000F0; // unmask all          interrupts
  GICD0->ISENABLER1 |= 0x00300000; // enable PS2          interrupts
  GICC0->CTLR        = 0x00000001; // enable GIC interface
  GICD0->CTLR        = 0x00000001; // enable GIC distributor
}

uint16_t custom_colour(uint8_t r, uint8_t g, uint8_t b) {
  return (b << 10) | (g << 5) | r;
}

void draw_char(int c, int x, int y, int fcolour, int bcolour) {
  int cx, cy;
  unsigned char mask;

  for (cy = 0; cy < 8; cy++) {
    for (cx = 0; cx < 8; cx++) {
      mask = 0x01 << cx;
      if ((font8x8_basic[c][cy] & mask) == mask) {
        fb[y + cy][x + cx] = fcolour;
      } else {
        if (bcolour != -1) {
          fb[y + cy][x + cx] = bcolour;
        }
      }
    }
  }

}

void draw_big_char(int c, int x, int y, int fcolour, int bcolour) {
  int cx, cy;
  unsigned char mask;

  for (cy = 0; cy < 16; cy++) {
    for (cx = 0; cx < 16; cx++) {
      mask = 0x01 << (cx / 2);
      if ((font8x8_basic[c][cy / 2] & mask) == mask) {
        fb[y + cy][x + cx] = fcolour;
      } else {
        if (bcolour != -1) {
          fb[y + cy][x + cx] = bcolour;
        }
      }
    }
  }
}

void fill_rect(int x, int y, int width, int height, int colour) {
  for (int ry = 0; ry < height; ry++) {
    for (int rx = 0; rx < width; rx++) {
      fb[y + ry][x + rx] = colour;
    }
  }
}

void fill_background(uint16_t colour) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      fb[y][x] = colour;
    }
  }
}

void draw_gui() {
  fill_background(BLACK);
}

void process_mouse_buffer() {
  uint8_t y_sign = (mouse_buffer[0] >> 5) & 0x01;
  uint8_t x_sign = (mouse_buffer[0] >> 4) & 0x01;
  uint8_t m1_pressed = mouse_buffer[0] & 0x01;
  uint8_t m2_pressed = (mouse_buffer[0] >> 1) & 0x01;
  uint8_t x_amount = mouse_buffer[1];
  uint8_t y_amount = mouse_buffer[2];
  fill_rect(0, 0, 100, 100, BLACK);
  if (y_sign == 0) {
    if (y_amount > 0) {
      my = (my - 1) % HEIGHT;
    }
    draw_char(0x31, 0, 0, WHITE, -1);
  } else {
    if (y_amount > 0) {
      my = (my + 1) % HEIGHT;
    }
    draw_char(0x30, 0, 0, WHITE, -1);
  }
  if (x_sign == 1) {
    if (x_amount > 0) {
      mx = (mx - 1) % WIDTH;
    }
    draw_char(0x31, 16, 0, WHITE, -1);
  } else {
    if (x_amount > 0) {
      mx = (mx + 1) % WIDTH;
    }
    draw_char(0x30, 16, 0, WHITE, -1);
  }
  if (m1_pressed == 1) {
    draw_char(0x31, 32, 0, WHITE, -1);
  } else {
    draw_char(0x30, 32, 0, WHITE, -1);
  }
  if (m2_pressed == 1) {
    draw_char(0x31, 48, 0, WHITE, -1);
  } else {
    draw_char(0x30, 48, 0, WHITE, -1);
  }
  fill_rect(mx, my, 8, 8, RED);
}

void add_to_mouse_buffer(uint8_t x) {
  mouse_buffer[byte_number] = x;
  byte_number++;
  if (byte_number > 2) {
    process_mouse_buffer();
    byte_number = 0;
  }
}

void mouse_handler(uint8_t x) {
  PL011_putc( UART0, '1',                      true );
  PL011_putc( UART0, '<',                      true );
  PL011_putc( UART0, itox( ( x >> 4 ) & 0xF ), true );
  PL011_putc( UART0, itox( ( x >> 0 ) & 0xF ), true );
  PL011_putc( UART0, '>',                      true );
  add_to_mouse_buffer(x);
}

void keyboard_handler(uint8_t x) {
  PL011_putc( UART0, '0',                      true );
  PL011_putc( UART0, '<',                      true );
  PL011_putc( UART0, itox( ( x >> 4 ) & 0xF ), true );
  PL011_putc( UART0, itox( ( x >> 0 ) & 0xF ), true );
  PL011_putc( UART0, '>',                      true );
  if (x == 0x12) {
    fill_background(BLACK);
    draw_char(0x42, 5, 5, 0x7FFF, -1);
    draw_big_char(0x61, 30, 30, 0x7FFF, -1);
  } else if (x == 0x11) {
    fill_background(BLACK);
    fill_rect(50, 10, 200, 100, 0x7FFF);
  }
}

void main_gui() {

  configure_LCD();

  enable_ps2_interrupts();

  draw_gui();

  exit(EXIT_SUCCESS);
}
