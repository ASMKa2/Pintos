#include <stdio.h>
#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <list.h>

typedef int32_t float;

int32_t float_add_float(int32_t, int32_t);
int32_t float_sub_float(int32_t, int32_t);
int32_t float_mul_float(int32_t, int32_t);
int32_t float_div_float(int32_t, int32_t);

int32_t float_add_int(int32_t, int32_t);
int32_t float_sub_int(int32_t, int32_t);
int32_t float_mul_int(int32_t, int32_t);
int32_t float_div_int(int32_t, int32_t);

int32_t int_add_float(int32_t, int32_t);
int32_t int_sub_float(int32_t, int32_t);
int32_t int_mul_float(int32_t, int32_t);
int32_t int_div_float(int32_t, int32_t);
