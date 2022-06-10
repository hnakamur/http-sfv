#include "hsfv.h"

int hsfv_string_eq(hsfv_string_t self, hsfv_string_t other) {
  return self.len == other.len && !memcmp(self.base, other.base, self.len);
}
