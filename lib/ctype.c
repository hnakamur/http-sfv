#include "hsfv.h"

bool hsfv_is_ascii_string(const char *input, const char *input_end)
{
    for (; input < input_end; ++input) {
        if (!HSFV_IS_ASCII(*input)) {
            return false;
        }
    }
    return true;
}
