/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2025 Garmin International, Inc.
// Licensed under the Flexible and Interoperable Data Transfer (FIT) Protocol License; you
// may not use this file except in compliance with the Flexible and Interoperable Data
// Transfer (FIT) Protocol License.
/////////////////////////////////////////////////////////////////////////////////////////////


#if !defined(FIT_CONFIG_HPP)
#define FIT_CONFIG_HPP


#if defined(__cplusplus)
   extern "C" {
#endif

#define FIT_USE_STDINT_H            // Define to use stdint.h types. By default size in bytes of integer types assumed to be char=1, short=2, long=4.

#define FIT_WIDE_CHAR_SIZE       2     // Define the size of a wide character (1, 2 or 4 bytes).
                                       // Character encoding also defined by size.   1: UTF-8   2: UTF-16   4: UTF-32
//#define FIT_CPP_INCLUDE_C // Define to include C definitions in C++ header file.  Allows C and C++ code to be compiled together.

#if defined(__cplusplus)
   }
#endif

#endif // !defined(FIT_CONFIG_HPP)
