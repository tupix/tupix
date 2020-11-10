#ifndef STD_UTIL_H

#define max(a, b)                                                              \
({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a > _b ? _a : _b;                                                     \
})

#define min(a, b)                                                              \
({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a < _b ? _a : _b;                                                     \
})

unsigned int calc_digits(unsigned int n, unsigned int base);
char* ultostr(unsigned long n, unsigned int base, char* str, unsigned int* len);
char* ltostr(long n, unsigned int base, char* str, unsigned int* len);

#define STD_UTIL_H
#endif

