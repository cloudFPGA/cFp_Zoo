/*******************************************************************************
 * Copyright 2016 -- 2022 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*******************************************************************************/

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
