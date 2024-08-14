/*
    Fake header for DCon SDK for the purposes of GUSI 2
 */
#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void dfprintf(const char *log, const char *format, ...);
    void vdfprintf(const char *log, const char *format, va_list args);
#ifdef __cplusplus
}
#endif