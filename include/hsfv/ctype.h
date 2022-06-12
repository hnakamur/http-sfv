#ifndef hsfv_ctype_h
#define hsfv_ctype_h

#ifdef __cplusplus
extern "C" {
#endif

#define hsfv_is_digit(c) ('0' <= (c) && (c) <= '9')
#define hsfv_is_lcalpha(c) (('a' <= (c) && (c) <= 'z'))
#define hsfv_is_alpha(c)                                                       \
  (('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z'))
#define hsfv_is_key_char(c)                                                    \
  (hsfv_is_lcalpha(c) || hsfv_is_digit(c) || (c) == '_' || (c) == '-' ||       \
   (c) == '.' || (c) == '*')
#define hsfv_is_ascii(c) ((c) <= '\x7f')

static inline bool hsfv_is_ascii_string(const char *input,
                                        const char *input_end) {
  for (; input < input_end; ++input) {
    if (!hsfv_is_ascii(*input)) {
      return false;
    }
  }
  return true;
}

#define hsfv_is_token_lead_char(c) (hsfv_is_alpha(c) || (c) == '*')

extern const char *hsfv_extended_tchar_map;
#define hsfv_is_extended_tchar(c) hsfv_extended_tchar_map[(unsigned char)(c)]

#ifdef __cplusplus
}
#endif

#endif
