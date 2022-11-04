#include <debug.h>
#include <stdio.h>
#include <stddef.h>
#include "threads/float.h"

#define FRACTION 14

int32_t float_add_float(int32_t f1, int32_t f2){
    return f1 + f2;
}

int32_t float_sub_float(int32_t f1, int32_t f2){
    return f1 - f2;
}

int32_t float_mul_float(int32_t f1, int32_t f2){
    int64_t retval = f1 * f2 >> FRACTION;

    return (int32_t)retval;
}

int32_t float_div_float(int32_t f1, int32_t f2){
    int64_t retval = f1 << FRACTION;

    retval /= f2;

    return (int32_t)retval;
}


int32_t float_add_int(int32_t f1, int32_t f2){
    return f1 + (f2 << FRACTION);
}

int32_t float_sub_int(int32_t f1, int32_t f2){
    return f1 - (f2 << FRACTION);
}

int32_t float_mul_int(int32_t f1, int32_t f2){
    int64_t retval = f1 * f2;

    return (int32_t)retval;
}

int32_t float_div_int(int32_t f1, int32_t f2){
    int64_t retval = (f1 << FRACTION) / (f2 << FRACTION);

    return (int32_t)retval;
}


int32_t int_add_float(int32_t f1, int32_t f2){
    return (f1 << FRACTION) + f2;
}

int32_t int_sub_float(int32_t f1, int32_t f2){
    return (f1 << FRACTION) - f2;
}

int32_t int_mul_float(int32_t f1, int32_t f2){
    int64_t retval = f1 * f2;

    return (int32_t)retval;
}

int32_t int_div_float(int32_t f1, int32_t f2){
    int64_t retval = (f1 << FRACTION) / f2;

    return (int32_t)retval;
}
