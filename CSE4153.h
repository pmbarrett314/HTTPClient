#include <sys/errno.h>
#include <limits.h>

uint16_t validate_port(char *svalue, uint16_t def)
{
    errno = 0;
    char *end;
    long value = strtol(svalue, &end, 10);
    if (end == svalue)
    {
        fprintf(stderr, "%s: not a decimal number\n", argv[i]);
    }
    else if ('\0' != *end)
    {
        fprintf(stderr, "%s: extra characters at end of input: %s\n", argv[i], end);
    }
    else if ((LONG_MIN == value || LONG_MAX == value) && ERANGE == errno)
    {
        fprintf(stderr, "%s out of range of type long\n", argv[i]);
    }
    else if (value > UINT16_MAX)
    {
        fprintf(stderr, "%ld is to big to be a port number\n", value);
    }
    else if (value <= 0)
    {
        fprintf(stderr, "%ld too small to be a port number\n", value);
    }
    else
    {
        return (unsigned short) value;
    }
    return def;
}