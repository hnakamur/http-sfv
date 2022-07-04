#include "hsfv.h"

bool hsfv_is_ascii_string(const char *input, const char *input_end)
{
    for (; input < input_end; ++input) {
        if (!HSFV_IS_ASCII(*(const unsigned char *)input)) {
            return false;
        }
    }
    return true;
}

int hsfv_strncasecmp(const char *s1, const char *s2, size_t n)
{
    const hsfv_byte_t *bytes1 = (const hsfv_byte_t *)s1;
    const hsfv_byte_t *bytes2 = (const hsfv_byte_t *)s2;
    while (n) {
        hsfv_byte_t c1 = *bytes1++;
        hsfv_byte_t c2 = *bytes2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if (c1 == c2) {

            if (c1) {
                n--;
                continue;
            }

            return 0;
        }

        return c1 < c2 ? -1 : 1;
    }

    return 0;
}
