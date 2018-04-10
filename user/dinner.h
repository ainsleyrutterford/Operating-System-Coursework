#ifndef __DINNER_H
#define __DINNER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "libc.h"

#include "PL011.h"

typedef struct {
  int id;
  bool dirty;
} fork_t;

#endif
