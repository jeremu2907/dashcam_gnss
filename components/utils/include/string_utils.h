#pragma once

size_t count_char(const char c, const char *string);

int split(const char delim,
          const char *string,
          char **tokens,
          const size_t tokens_size);