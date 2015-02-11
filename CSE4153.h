#include <sys/errno.h>
#include <limits.h>
#include <string.h>

uint16_t validate_port(char const *svalue, uint16_t def)
{
    //convert the given port to the correct type
    errno = 0;
    char *end;
    long value = strtol(svalue, &end, 10);
    if (end == svalue)
    {
        fprintf(stderr, "%s: not a decimal number\n", svalue);
    }
    else if ('\0' != *end)
    {
        fprintf(stderr, "%s: extra characters at end of input: %s\n", svalue, end);
    }
    else if ((LONG_MIN == value || LONG_MAX == value) && ERANGE == errno)
    {
        fprintf(stderr, "%s out of range of type long\n", svalue);
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


void printints(char *buffer)
{
    //debug function
    printf("\n\n");
    for (int i = 0; i <= strlen(buffer); i++)
    {
        printf("%c: %d, ", buffer[i], buffer[i]);
    }

    printf("\n\n");
}