
/**
 * @file   util.hpp
 * @author DID
 * @date   Nov 2021
 *
 * @brief  Common utility functions headers, i.e. timing, printing wrappers.
 */

#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <ostream>
#include <fstream>
#include "config.h"

extern std::ostream &dout;

#define dbgLevelNone    0
#define dbgLevelError   1
#define dbgLevelWarning 2
#define dbgLevelInfo    3
#define dbgLevelDebug   4


template<uint8_t level>
void Dprintf(const char* format, ... ) {
#ifndef __SYNTHESIS__
    if (level<=DEBUG_LEVEL) {
        va_list arglist;
        va_start( arglist, format );
        vprintf( format, arglist );
        va_end( arglist );
    }
#endif
}

void Dfprintf(uint8_t level, FILE *stream, const char* format, ... );
void bprintf( const char* format, ... );


#endif
