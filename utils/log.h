#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _ERR_STRINGIFY2(x) #x
#define _ERR_STRINGIFY(x) _ERR_STRINGIFY2(x)

extern enum loglevel {
    LOGLEVEL_ALL = 0,
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARN,
    LOGLEVEL_ERROR,
} loglevel;
// enum loglevel loglevel = LOGLEVEL_INFO;

#define LOGLIMIT(LVL, msg, ...) ({          \
    static unsigned long _ntimes = 0;       \
    static unsigned long _limit = 4;        \
    if (_ntimes == 2 *_limit)  _limit *= 2; \
    if (_ntimes++ <= _limit)                \
        LVL("[%8lu] " msg, _ntimes, ## __VA_ARGS__); \
})

#define _ERR_MSG(severity, msg, ...) ({if (loglevel <= LOGLEVEL_ ## severity) { fprintf(stderr, "[%-5s] [%s:%d:%s] " msg "\n", _ERR_STRINGIFY(severity), __FILE__, __LINE__, __func__, ## __VA_ARGS__); } })

#define FAIL(...) ({ERROR(__VA_ARGS__); abort();})
#define ERROR(...) _ERR_MSG(ERROR, ## __VA_ARGS__)
#define WARN(...)  _ERR_MSG(WARN,  ## __VA_ARGS__)
#define INFO(...)  _ERR_MSG(INFO,  ## __VA_ARGS__)
#define DEBUG(...) _ERR_MSG(DEBUG, ## __VA_ARGS__)
#define MEMFAIL() PFAIL("Could not allocate memory")

#define PFAIL(...) ({PERROR(__VA_ARGS__); exit(EXIT_FAILURE);})
#define PERROR(msg, ...) _ERR_MSG(ERROR,"[%s] " msg, strerror(errno), ## __VA_ARGS__)

#define ASSERT(cond, ...) if (!(cond)) { FAIL("Assertion failed: " _ERR_STRINGIFY(cond) " " __VA_ARGS__ ); }

#define SET_ERRNO(e) ({ errno = e; BACKTRACE("errno = %s (%d)", strerror(e), e); })
#include <execinfo.h>
#define BACKTRACE(...) ({ \
    INFO("Backtrace: " __VA_ARGS__); \
    void * _buffer[100]; \
    int _nptrs = backtrace(_buffer, 100); \
    backtrace_symbols_fd(_buffer, _nptrs, fileno(stderr)); \
})
