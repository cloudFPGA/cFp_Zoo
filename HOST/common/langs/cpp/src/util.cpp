
/**
 * @file   util.cpp
 * @author DID
 * @date   Nov 2021
 *
 * @brief  Common utility functions, i.e. timing, printing wrappers.
 */

#include "util.hpp"



void Dfprintf(uint8_t level, FILE *stream, const char* format, ... ) {
#ifndef __SYNTHESIS__
    if (level<=DEBUG_LEVEL) {
        va_list arglist;
        va_start( arglist, format );
        vfprintf(stream, format, arglist );
        va_end( arglist );
    }
#endif
}

void bprintf( const char* format, ... ) {
#ifndef __SYNTHESIS__
    va_list arglist;
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );
#endif
}


#ifdef DEBUG
std::ostream &dout = cout;
#else
std::ofstream dev_null ("/dev/null");
std::ostream &dout = dev_null;
#endif
