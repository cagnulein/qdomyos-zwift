////////////////////////////////////////////////////////////////////////////////
// The following FIT Protocol software provided may be used with FIT protocol
// devices only and remains the copyrighted property of Garmin Canada Inc.
// The software is being provided on an "as-is" basis and as an accommodation,
// and therefore all warranties, representations, or guarantees of any kind
// (whether express, implied or statutory) including, without limitation,
// warranties of merchantability, non-infringement, or fitness for a particular
// purpose, are specifically disclaimed.
//
// Copyright 2020 Garmin Canada Inc.
////////////////////////////////////////////////////////////////////////////////


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
