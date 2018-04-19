#ifndef __GUI_H
#define __GUI_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"
#include "PL050.h"
#include "PL111.h"
#include   "SYS.h"

// Include functionality relating to the   kernel.

#include "lolevel.h"
#include     "int.h"

#include "libc.h"
#include "font8x8_basic.h"

#define WIDTH  800
#define HEIGHT 600

#define WHITE 0x7FFF
#define BLACK 0x0000
#define RED   0x001F
#define GREEN 0x03E0
#define BLUE  0x7C00
#define GREY  0x3DEF

char cursor[12] = { 0x01, 0x03, 0x07, 0x0F,
                    0x1F, 0x3F, 0x7F, 0xFF,
                    0x1F, 0x1B, 0x31, 0x30 };

char gui_fork[48][2] = {{0x86, 0x61},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xFF, 0xFF},
                        {0xFE, 0x7F},
                        {0xFC, 0x3F},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0x80, 0x01}};

char gui_fork_inv[48][2] = {{0x80, 0x01},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xC0, 0x03},
                        {0xFC, 0x3F},
                        {0xFE, 0x7F},
                        {0xFF, 0xFF},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0xCF, 0xF3},
                        {0x86, 0x61}};

#endif
