#ifndef STD_IO_H
#define STD_IO_H

/*
 * Print output according to formatstring `format`.
 */
void printf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif /* STD_IO_H */
