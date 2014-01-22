#include <string.h>

size_t strlen(const char* s)
{
    size_t len = 0;
    while (*s != '\0')
    {
        ++len;
        ++s;
    }
    return len;
}

char* strrchr(const char* s, int c)
{
    char* save = NULL;
    for (;;)
    {
        if (*s == c)
            save = s;
        if (*s == '\0')
            break;
        ++s;
    }
    return save;
}

char* strchr(const char* s, int c)
{
    for (;;)
    {
        if (*s == c)
            return s;
        if (*s == '\0')
            break;
        ++s;
    }
    return NULL;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    if (n != 0)
    {
        do {
            unsigned char c1 = *s1;
            unsigned char c2 = *s2;
            if (c1 != c2)
                return (c1 - c2);
            if (c1 == 0)
                break;
            ++s1; ++s2;
        } while (--n != 0);
    }
    return (0);
}

char* strncpy(char* dst, const char* src, size_t n)
{
    if (n != 0)
    {
        char *d = dst;
        do
        {
            if ((*d++ = *src++) == '\0')
            {
                /* NUL pad the remaining n-1 bytes */
                while (--n)
                    *d++ = '\0';
                break;
            }
        } while (--n);
    }
    return (dst);
}

char* strcpy(char* dst, const char* src)
{
    char *d = dst;
    do
    /* nothing */;
    while ((*d++ = *src++) != '\0');
    return (dst);
}
