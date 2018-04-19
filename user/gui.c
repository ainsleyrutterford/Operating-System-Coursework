#include "gui.h"

uint16_t fb[ HEIGHT ][ WIDTH ];
int mouse_buffer[3]; int byte_number = 0;
uint16_t mx = 0;
uint16_t my = 0;
int dinner_x = 20;
int dinner_y = 20;

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

void draw_cursor(int x, int y) {
  int cx, cy;
  unsigned char mask;

  for (cy = 0; cy < 12; cy++) {
    for (cx = 0; cx < 8; cx++) {
      mask = 0x01 << cx;
      if ((cursor[cy] & mask) == mask) {
        fb[y + cy][x + cx] = WHITE;
      }
    }
  }
}

void draw_fork(int x, int y, int id, int colour) {
  int cx, cy;
  unsigned char mask;

  for (cy = 0; cy < 48; cy++) {
    for (cx = 0; cx < 16; cx++) {
      mask = 0x01 << (cx % 8);
      if (id > 7) {
        if ((gui_fork[cy][cx / 8] & mask) == mask) {
          fb[y + cy][x + cx] = colour;
        }
      } else {
        if ((gui_fork_inv[cy][cx / 8] & mask) == mask) {
          fb[y + cy][x + cx] = colour;
        }
      }
    }
  }
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

void stroke_circle(int x_centre, int y_centre, int r, int colour) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if ((x - x_centre) * (x - x_centre) + (y - y_centre) * (y - y_centre) < (r * r + r) &&
          (x - x_centre) * (x - x_centre) + (y - y_centre) * (y - y_centre) > (r * r - r)) {
            fb[y][x] = colour;
          }
    }
  }
}

void fill_circle(int x_centre, int y_centre, int r, int colour) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if ((x - x_centre) * (x - x_centre) + (y - y_centre) * (y - y_centre) < (r * r)) {
            fb[y][x] = colour;
          }
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

void draw_dinner_gui() {
  int x_centre = 0;
  int y_centre = 0;
  int r = 25;
  int spacing = 95;
  int y_fork_1, y_fork_2, x_fork_1, x_fork_2, offset;
  for (int i = 0; i < 2; i++) {
    y_centre = dinner_y + r + (i * spacing);
    y_fork_1 = dinner_y + (i * spacing);
    y_fork_2 = dinner_y + (i * spacing);
    for (int j = 0; j < 8; j++) {
      if (i == 1) offset = (7 - j);
      else offset = j;
      x_centre = dinner_x + 22 + r + (offset * spacing);
      x_fork_1 = dinner_x + 5 + (offset * spacing);
      x_fork_2 = dinner_x + (spacing - 21) + (offset * spacing);
      fill_circle(x_centre, y_centre, r, custom_colour(0, 31, 0));
      draw_fork(x_fork_1, y_fork_1, (i * 8) + j, custom_colour(4, 4, 4));
      draw_fork(x_fork_2, y_fork_2, (i * 8) + j, custom_colour(4, 4, 4));
    }
  }
}

void add_eater_dinner_gui(int id) {
  int x_centre = 0;
  int y_centre = 0;
  int r = 25;
  int spacing = 95;
  int offset;
  for (int i = 0; i < 2; i++) {
    y_centre = dinner_y + r + (i * spacing);
    for (int j = 0; j < 8; j++) {
      if (i == 1) offset = (7 - j);
      else offset = j;
      x_centre = dinner_x + 22 + r + (offset * spacing);
      if (((i * 8) + j) == id) {
        fill_circle(x_centre, y_centre, r, custom_colour(0, 0, 31));
      }
    }
  }
}

void remove_eater_dinner_gui(int id) {
  int x_centre = 0;
  int y_centre = 0;
  int r = 25;
  int spacing = 95;
  int offset;
  for (int i = 0; i < 2; i++) {
    y_centre = dinner_y + r + (i * spacing);
    for (int j = 0; j < 8; j++) {
      if (i == 1) offset = (7 - j);
      else offset = j;
      x_centre = dinner_x + 22 + r + (offset * spacing);
      if (((i * 8) + j) == id) {
        fill_circle(x_centre, y_centre, r, custom_colour(0, 31, 0));
      }
    }
  }
}

void add_fork_dinner_gui(int id, char* side) {
  int x_fork, y_fork, offset;
  int spacing = 95;
  bool true_left = true;
  for (int i = 0; i < 2; i++) {
    y_fork = dinner_y + (i * spacing);
    for (int j = 0; j < 8; j++) {
      if ((i * 8 + j) == id) {
        if (i == 1) {
          offset = (7 - j);
          true_left = true;
        } else {
          offset = j;
          true_left = false;
        }
        if (0 == strcmp(side, "left")) {
          if (true_left) x_fork = dinner_x + 5              + (offset * spacing);
          else           x_fork = dinner_x + (spacing - 21) + (offset * spacing);
        } else {
          if (true_left) x_fork = dinner_x + (spacing - 21) + (offset * spacing);
          else           x_fork = dinner_x + 5              + (offset * spacing);
        }
        draw_fork(x_fork, y_fork, (i * 8) + j, WHITE);
      }
    }
  }
}

void remove_fork_dinner_gui(int id, char* side) {
  int x_fork, y_fork, offset;
  int spacing = 95;
  bool true_left = true;
  for (int i = 0; i < 2; i++) {
    y_fork = dinner_y + (i * spacing);
    for (int j = 0; j < 8; j++) {
      if ((i * 8 + j) == id) {
        if (i == 1) {
          offset = (7 - j);
          true_left = true;
        } else {
          offset = j;
          true_left = false;
        }
        if (0 == strcmp(side, "left")) {
          if (true_left) x_fork = dinner_x + 5              + (offset * spacing);
          else           x_fork = dinner_x + (spacing - 21) + (offset * spacing);
        } else {
          if (true_left) x_fork = dinner_x + (spacing - 21) + (offset * spacing);
          else           x_fork = dinner_x + 5              + (offset * spacing);
        }
        draw_fork(x_fork, y_fork, (i * 8) + j, custom_colour(4, 4, 4));
      }
    }
  }
}

void draw_gui() {
  fill_background(BLACK);
  draw_dinner_gui();
}

void process_mouse_buffer() {
  uint8_t y_overflow = (mouse_buffer[0] >> 7) & 0x01;
  uint8_t x_overflow = (mouse_buffer[0] >> 6) & 0x01;
  uint8_t m1_pressed = mouse_buffer[0] & 0x01;
  uint8_t m2_pressed = (mouse_buffer[0] >> 1) & 0x01;
  fill_background(BLACK);
  if (x_overflow != 0x01) {
    mx += mouse_buffer[1] - ((mouse_buffer[0] << 4) & 0x100);
    if (mx <= 20) mx = 20;
    if (mx >= WIDTH - 20) mx = WIDTH - 20;
  }
  if (y_overflow != 0x01) {
    my -= mouse_buffer[2] - ((mouse_buffer[0] << 3) & 0x100);
    if (my <= 20) my = 20;
    if (my >= HEIGHT- 20) my = HEIGHT - 20;
  }
  draw_cursor(mx, my);
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
