#ifndef __BLIN_H
#define __BLIN_H

#include <stdint.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef size_t usize;
typedef int32_t i32;
typedef uint32_t u32;
typedef u8* ptr;

ptr compile(u8 *line);
void run(ptr m, i32 result[4]);

#endif