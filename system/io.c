#include <stdarg.h>

#include <driver/serial.h>

#include <std/types.h>
#include <std/util.h>
#include <std/strings.h>

#define MAX_NUM_LEN 20 // len(-9223372036854775808)

void kprint(const char* s)
{
        while (*s) {
                kputchar(*(s++));
        }
}

inline void print_with_padding(const char* num_str, unsigned int len,
                                                           unsigned int field_width, const char padding)
{
        unsigned int str_len = max(len, field_width);
        for (unsigned int i = 0; i < str_len - len; ++i) {
                kputchar(padding);
        }
        kprint(num_str);
}

void kprintf(const char* format, ...)
{
        const char* cur_char = format;
        unsigned int n_escape = 0;
        while (*cur_char) {
                if (*(cur_char++) != '%')
                        continue;

                n_escape++;

                bool repeat = false;
                do {
                        switch (*(cur_char++)) {
                        case '0' ... '9':
                                while (*cur_char >= '0' && *cur_char <= '9') {
                                        cur_char++;
                                }
                                repeat = true;
                                break;
                        case 'x':
                        case 'i':
                        case 'u':
                        case 'p':
                        case 's':
                        case 'c': // TODO: check if repeat == true and error?
                        case '%':
                                repeat = false;
                                break;
                        default:
                                // NOTE: Error!
                                return;
                        }
                } while (repeat);
        }
        // Reset cur_char
        cur_char = format;

        va_list args;
        va_start(args, format);

        char num_str[MAX_NUM_LEN + 1]; // \0
        unsigned int len;
        while (*cur_char) {
                if (*(cur_char++) != '%') {
                        kputchar(*(cur_char - 1));
                        continue;
                }

                char padding = ' ';
                unsigned int field_width = 0;

                // TODO(aurel): Pull out into function
                if (*cur_char >= '0' && *cur_char <= '9') {
                        if (*cur_char == '0') {
                                padding = '0';
                                cur_char++;
                        }

                        // Move pointer to the end of the number and walk through the number again in
                        // reverse.
                        const char* beg = cur_char;
                        while (*cur_char >= '0' && *cur_char <= '9') {
                                ++cur_char;
                        }
                        const char* end = cur_char;
                        for (unsigned int power = 1; --cur_char >= beg; power *= 10) {
                                unsigned int cur = *cur_char - '0';
                                field_width += power * cur;
                        }
                        cur_char = end;
                }

                switch (*cur_char) {
                case 'c':
                        kputchar((unsigned char)va_arg(args, int));
                        break;
                case 's': {
                        const char* str = va_arg(args, const char*);
                        print_with_padding(str, str_len(str), field_width, padding);
                        break;
                }
                case 'x':
                        ultostr(va_arg(args, unsigned int), 16, num_str, &len);
                        print_with_padding(num_str, len, field_width, padding);
                        break;
                case 'i':
                        ltostr(va_arg(args, int), 10, num_str, &len);
                        print_with_padding(num_str, len, field_width, padding);
                        break;
                case 'u':
                        ultostr(va_arg(args, unsigned int), 10, num_str, &len);
                        print_with_padding(num_str, len, field_width, padding);
                        break;
                case 'p':
                        kprint("0x");
                        ultostr((unsigned long)va_arg(args, void*), 16, num_str, &len);
                        print_with_padding(num_str, len, field_width, padding);
                        break;
                case '%':
                        kputchar('%');
                        break;
                }
                cur_char++;
        }

        va_end(args);
}
