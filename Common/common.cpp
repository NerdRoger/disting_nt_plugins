#include <cstdarg>
#include <cstring>
#include "common.h"

void StringConcat(char* dest, int maxLen, ...) {
    if (dest == nullptr || maxLen <= 0) return;

    // 1. Initialize the buffer as an empty string
    dest[0] = '\0';
    int spaceLeft = maxLen - 1; // Reserve room for final null terminator

    va_list args;
    va_start(args, maxLen); // Start after the maxLen argument

    const char* src;
    // 2. Loop until we hit the NULL sentinel
    while ((src = va_arg(args, const char*)) != nullptr) {
        int currentLen = strlen(dest);
        int remaining = spaceLeft - currentLen;

        if (remaining > 0) {
            strncat(dest, src, remaining);
        }
    }

    va_end(args);
}