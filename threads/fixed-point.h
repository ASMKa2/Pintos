#include <stdio.h>
#include <debug.h>
#include <stddef.h>
#include <stdint.h>

int32_t int_to_float(int32_t);
int32_t float_round(int32_t);

int32_t float_add_float(int32_t, int32_t);
int32_t float_sub_float(int32_t, int32_t);
int32_t float_mul_float(int32_t, int32_t);
int32_t float_div_float(int32_t, int32_t);

int32_t float_add_int(int32_t, int32_t);
int32_t float_sub_int(int32_t, int32_t);
int32_t float_mul_int(int32_t, int32_t);
int32_t float_div_int(int32_t, int32_t);

int32_t int_sub_float(int32_t, int32_t);

#define FRACTION_BITS 14

int32_t int_to_float(int32_t i){
    return i << FRACTION_BITS;
}

int32_t float_round(int32_t f){
    if(f >= 0){
        return (f + (1 << (FRACTION_BITS - 1))) >> FRACTION_BITS;
    }
    else{
        return (f - (1 << (FRACTION_BITS - 1))) >> FRACTION_BITS;
    }
}

int32_t float_add_float(int32_t f1, int32_t f2){
    return f1 + f2;
}

int32_t float_sub_float(int32_t f1, int32_t f2){
    return f1 - f2;
}

int32_t float_mul_float(int32_t f1, int32_t f2){
    int64_t retval = ((int64_t)f1 * f2) >> FRACTION_BITS;

    return (int32_t)retval;
}

int32_t float_div_float(int32_t f1, int32_t f2){
    int64_t retval = ((int64_t)f1 << FRACTION_BITS) / f2;

    return (int32_t)retval;
}


int32_t float_add_int(int32_t f1, int32_t f2){
    return f1 + (f2 << FRACTION_BITS);
}

int32_t float_sub_int(int32_t f1, int32_t f2){
    return f1 - (f2 << FRACTION_BITS);
}

int32_t float_mul_int(int32_t f1, int32_t f2){
    return f1 * f2;
}

int32_t float_div_int(int32_t f1, int32_t f2){
    return f1 / f2;
}


int32_t int_sub_float(int32_t f1, int32_t f2){
    return (f1 << FRACTION_BITS) - f2;
}