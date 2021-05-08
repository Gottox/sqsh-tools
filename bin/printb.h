/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : printb
 * @created     : Monday May 03, 2021 09:47:06 CEST
 */

#ifndef PRINTB_H

#define PRINTB_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define for_endian(size) for (int i = 0; i < size; ++i)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define for_endian(size) for (int i = size - 1; i >= 0; --i)
#else
#error "Endianness not detected"
#endif

#define printb(value, out)                              \
({                                                      \
        const typeof(value) _v = value;                       \
        __printb((typeof(_v) *) &_v, sizeof(_v), out);  \
})

#define MSB_MASK 1 << (CHAR_BIT - 1)

void __printb(const void *value, size_t size, FILE *out)
{
        unsigned char uc;
        unsigned char bits[CHAR_BIT + 1];

        bits[CHAR_BIT] = '\0';
        for_endian(size) {
                uc = ((unsigned char *) value)[i];
                memset(bits, '0', CHAR_BIT);
                for (int j = 0; uc && j < CHAR_BIT; ++j) {
                        if (uc & MSB_MASK)
                                bits[j] = '1';
                        uc <<= 1;
                }
                fprintf(out, "%s ", bits);
        }
        fputs("\n", out);
}

#endif /* end of include guard PRINTB_H */

