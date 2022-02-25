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


bool isNumeric(const std::string str) {        // loop Through each character in the string
    for(char x:  str)
        if(!isdigit(x)) // Check if a single character "x" its a digit
            return false;  // if its not return false 
      return true; // else return true
}
