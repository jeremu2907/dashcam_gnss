#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"

#include "string_utils.h"

#define TAG "utils"

size_t count_char(const char c, const char *string)
{
    size_t count = 0;
    size_t i = 0;

    while (string[i] != '\0')
    {
        if (string[i] == c)
        {
            count++;
        }

        i++;
    }

    return count;
}

int split(const char delim,
          const char *string,
          char **tokens,
          const size_t tokens_size)
{
    size_t l = 0;
    size_t r = 0;
    size_t len = 0;

    for (size_t i = 0; i < tokens_size; i++)
    {
        const char *next_delim = strchr(string + l, delim);
        if (next_delim != NULL)
        {
            // If delim is found, r is the idx of the delim
            r = next_delim - string;
        }
        else
        {
            // If delim is found, r is the idx of the nullterm
            r = strlen(string);
            if (string[r] != '\0')
            {
                ESP_LOGE(TAG, "String is not null terminated");
                return -1;
            }
        }

        len = r - l + 1;

        tokens[i] = (char *)calloc(len + 1, sizeof(char));
        strncpy(tokens[i], string + l, len - 1);

        l = r + 1;
    }

    return 0;
}
