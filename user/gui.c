#include "gui.h"

#define BUTTONS 9

extern uint16_t fb[ HEIGHT ][ WIDTH ];
int mouse_buffer[3]; int byte_number = 0;
button_t buttons[BUTTONS];
uint16_t mx = 0;
uint16_t my = 0;
int dinner_x = 20;
int dinner_y = 20;
bool P5_running = false;
bool dinner = false;

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
      if (id < 8) {
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
    y_centre = dinner_y + r + ((1 - i) * spacing);
    y_fork_1 = dinner_y + ((1 - i) * spacing);
    y_fork_2 = dinner_y + ((1 - i) * spacing);
    for (int j = 0; j < 8; j++) {
      if (i == 1) offset = (7 - j);
      else offset = j;
      x_centre = dinner_x + 22 + r + (offset * spacing);
      x_fork_1 = dinner_x + 5 + (offset * spacing);
      x_fork_2 = dinner_x + (spacing - 21) + (offset * spacing);
      fill_circle(x_centre, y_centre, r, custom_colour(0, 31, 0));
      draw_fork(x_fork_1, y_fork_1, (i * 8) + j, custom_colour(1, 1, 1));
      draw_fork(x_fork_2, y_fork_2, (i * 8) + j, custom_colour(1, 1, 1));
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
    y_centre = dinner_y + r + ((1 - i) * spacing);
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
    y_centre = dinner_y + r + ((1 - i) * spacing);
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
    y_fork = dinner_y + ((1 - i) * spacing);
    for (int j = 0; j < 8; j++) {
      if ((i * 8 + j) == id) {
        if (i == 1) {
          offset = (7 - j);
          true_left = false;
        } else {
          offset = j;
          true_left = true;
        }
        if (0 == strcmp(side, "left")) {
          if (true_left) {
            x_fork = dinner_x + 5              + (offset * spacing);
          } else {
            x_fork = dinner_x + (spacing - 21) + (offset * spacing);
          }
        } else {
          if (true_left) {
            x_fork = dinner_x + (spacing - 21) + (offset * spacing);
          } else {
            x_fork = dinner_x + 5              + (offset * spacing);
          }
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
    y_fork = dinner_y + ((1 - i) * spacing);
    for (int j = 0; j < 8; j++) {
      if ((i * 8 + j) == id) {
        if (i == 1) {
          offset = (7 - j);
          true_left = false;
        } else {
          offset = j;
          true_left = true;
        }
        if (0 == strcmp(side, "left")) {
          if (true_left) x_fork = dinner_x + 5              + (offset * spacing);
          else           x_fork = dinner_x + (spacing - 21) + (offset * spacing);
        } else {
          if (true_left) x_fork = dinner_x + (spacing - 21) + (offset * spacing);
          else           x_fork = dinner_x + 5              + (offset * spacing);
        }
        draw_fork(x_fork, y_fork, (i * 8) + j, custom_colour(1, 1, 1));
      }
    }
  }
}

int ctoi(char c) {
  switch (c) {
    case '0':
      return 0x30;
    case '1':
      return 0x31;
    case '2':
      return 0x32;
    case '3':
      return 0x33;
    case '4':
      return 0x34;
    case '5':
      return 0x35;
    case '6':
      return 0x36;
    case '7':
      return 0x37;
    case '8':
      return 0x38;
    case '9':
      return 0x39;
    case 'A':
      return 0x41;
    case 'B':
      return 0x42;
    case 'C':
      return 0x43;
    case 'D':
      return 0x44;
    case 'E':
      return 0x45;
    case 'F':
      return 0x46;
    case 'G':
      return 0x47;
    case 'H':
      return 0x48;
    case 'I':
      return 0x49;
    case 'J':
      return 0x4A;
    case 'K':
      return 0x4B;
    case 'L':
      return 0x4C;
    case 'M':
      return 0x4D;
    case 'N':
      return 0x4E;
    case 'O':
      return 0x4F;
    case 'P':
      return 0x50;
    case 'Q':
      return 0x51;
    case 'R':
      return 0x52;
    case 'S':
      return 0x53;
    case 'T':
      return 0x54;
    case 'U':
      return 0x55;
    case 'V':
      return 0x56;
    case 'W':
      return 0x57;
    case 'X':
      return 0x58;
    case 'Y':
      return 0x59;
    case 'Z':
      return 0x5A;
    case 'a':
      return 0x61;
    case 'b':
      return 0x62;
    case 'c':
      return 0x63;
    case 'd':
      return 0x64;
    case 'e':
      return 0x65;
    case 'f':
      return 0x66;
    case 'g':
      return 0x67;
    case 'h':
      return 0x68;
    case 'i':
      return 0x69;
    case 'j':
      return 0x6A;
    case 'k':
      return 0x6B;
    case 'l':
      return 0x6C;
    case 'm':
      return 0x6D;
    case 'n':
      return 0x6E;
    case 'o':
      return 0x6F;
    case 'p':
      return 0x70;
    case 'q':
      return 0x71;
    case 'r':
      return 0x72;
    case 's':
      return 0x73;
    case 't':
      return 0x74;
    case 'u':
      return 0x75;
    case 'v':
      return 0x76;
    case 'w':
      return 0x77;
    case 'x':
      return 0x78;
    case 'y':
      return 0x79;
    case 'z':
      return 0x7A;
  }
}

void draw_string(int x, int y, int button_width, int button_height, char* label, int length, int font_size, uint16_t colour){
  int char_size;
  if (font_size == 1) char_size = 18;
  else char_size = 9;
  int x_offset = (button_width - (length * char_size)) / 2;
  int y_offset = (button_height - char_size) / 2;
  for (int i = 0; i < length; i++) {
    if (font_size == 1) draw_big_char(ctoi(label[i]), (x + x_offset) + ((18) * i), (y + y_offset), colour, -1);
    else draw_char(ctoi(label[i]), (x + x_offset) + ((8) * i), (y + y_offset), colour, -1);
  }
}

void draw_buttons() {
  int x = 20;
  int y = 20;
  for (int i = 0; i < BUTTONS; i++) {
    if (buttons[i].visible) {
      fill_rect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, buttons[i].colour);
      draw_string(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, buttons[i].label, buttons[i].label_length, buttons[i].label_size, buttons[i].label_colour);
    }
  }
}

void draw_gui() {
  fill_background(BLACK);
  draw_buttons();
  if (dinner) {
    draw_dinner_gui();
  } else {
    draw_buttons();
  }
}

extern void main_P3();
extern void main_P4();
extern void main_P5();
extern void main_dinner();

void button_pressed(int id) {
  PL011_putc( UART0, '1', true );
  switch (id) {
    case 0: {
      break;
    }
    case 1: {
      break;
    }
    case 2: {
      if (!P5_running) {
        pid_t pid = fork();
        if (pid == 0) {
          exec( &main_P5 );
        } else {
          nice(pid, 10);
        }
        P5_running = true;
      }
      break;
    }
    case 3: {
      fill_background(BLACK);
      draw_dinner_gui();
      dinner = true;
      break;
    }
  }
}

void check_button_press(int mx, int my) {
  for (int i = 0; i < BUTTONS; i++) {
    if ((mx > buttons[i].x) && (mx < (buttons[i].x + buttons[i].width))) {
      if ((my > buttons[i].y) && (my < (buttons[i].y + buttons[i].height))) {
        button_pressed(i);
      }
    }
  }
}

void create_buttons() {
  for (int i = 0; i < BUTTONS; i++) {
    memset( &buttons[i], 0, sizeof( button_t ));
    buttons[i].visible = false;
  }

  for (int i = 0; i < 4; i++) {
    buttons[i].x = 100 + (i * 150);
    buttons[i].y = 100;
    buttons[i].width = 140;
    buttons[i].height = 80;
    buttons[i].colour = custom_colour(20,31,20);
    buttons[i].label_size = 1;
    buttons[i].label_colour = BLACK;
    buttons[i].visible = true;
  }

  for (int i = 4; i < 8; i++) {
    buttons[i].x = 100 + ((i % 4) * 150);
    buttons[i].y = 190;
    buttons[i].width = 140;
    buttons[i].height = 40;
    buttons[i].colour = custom_colour(31,25,25);
    buttons[i].label_size = 0;
    buttons[i].label_colour = BLACK;
    buttons[i].visible = true;
  }

  buttons[8].x = 100;
  buttons[8].y = 420;
  buttons[8].width = 140;
  buttons[8].height = 80;
  buttons[8].colour = custom_colour(20,20,31);
  buttons[8].label_size = 1;
  buttons[8].label_colour = BLACK;
  buttons[8].visible = true;

  buttons[0].label = "P3";
  buttons[0].label_length = 2;
  buttons[1].label = "P4";
  buttons[1].label_length = 2;
  buttons[2].label = "P5";
  buttons[2].label_length = 2;
  buttons[3].label = "Dinner";
  buttons[3].label_length = 6;
  buttons[4].label = "terminate";
  buttons[4].label_length = 9;
  buttons[5].label = "terminate";
  buttons[5].label_length = 9;
  buttons[6].label = "terminate";
  buttons[6].label_length = 9;
  buttons[7].label = "terminate";
  buttons[7].label_length = 9;
  buttons[8].label = "killall";
  buttons[8].label_length = 7;

}

void process_mouse_buffer() {
  uint8_t y_overflow = (mouse_buffer[0] >> 7) & 0x01;
  uint8_t x_overflow = (mouse_buffer[0] >> 6) & 0x01;
  uint8_t m1_pressed = mouse_buffer[0] & 0x01;
  uint8_t m2_pressed = (mouse_buffer[0] >> 1) & 0x01;
  // fill_background(BLACK);
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
  if (m1_pressed == 1) {
    check_button_press(mx, my);
  }
  if (!dinner) {
    draw_gui();
    draw_cursor(mx, my);
  }
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

  create_buttons();

  draw_gui();

  exit(EXIT_SUCCESS);
}
