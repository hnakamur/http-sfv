#include "hsfv.h"

bool hsfv_is_parsable_boolean(const char *input, const char *input_end, const char **out_rest)
{
    if (input_end < input + 2 || input[0] != '?' || (input[1] != '1' && input[1] != '0')) {
        return false;
    }
    *out_rest = input + 2;
    return true;
}
