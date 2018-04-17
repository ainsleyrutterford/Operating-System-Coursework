#ifndef __GUI_H
#define __GUI_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

#endif
