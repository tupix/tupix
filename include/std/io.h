#ifndef SYSTEM_IO_H
#define SYSTEM_IO_H

/*
 * Print output according to format string `format`.
 *
 * The format string can include conversion specifiers following the syntax:
 *
 *     %[<padding>]<conversion>
 *
 * The optional padding is a decimal digit string specifying a minimum field
 * width. The remaining length will be padded with whitespace except if
 * `padding` starts with a zero.
 *
 * Following conversion characters are supported:
 *
 *   c     Takes an int argument, casts it to an `unsigned char` and then prints
 *         the resulting character.
 *
 *   s     Takes a `const char*` argument and writes all characters up to a
 *         terminating null byte
 *
 *   x     Takes an `unsigned int` argument and prints it in hexadecimal
 *         notation.
 *
 *   d, i  Takes an int argument and prints it in signed decimal notation.
 *
 *   u     Takes an `unsigned int` argument and prints it in unsigned decimal
 *         notation.
 *
 *   p     Takes a `void*` and prints the address in hexadecimal notation with
 *         `0x` a prefix.
 *
 *   %     Takes no argument and prints a literal `%`.
 */
void kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif /* SYSTEM_IO_H */
