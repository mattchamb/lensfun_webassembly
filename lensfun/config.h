#ifndef __CONFIG_H__
#define __CONFIG_H__

/* config file for Lensfun library
   generated by CMake
*/

#define CONF_PACKAGE "lensfun"
#define CONF_VERSION "0.3.2.0"

#if defined(__APPLE__)
    #define PLATFORM_OSX
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS
#elif defined(__unix__)
    #define PLATFORM_LINUX
#endif


#define CMAKE_COMPILER_IS_GNUCC
#ifdef CMAKE_COMPILER_IS_GNUCC
#define CONF_COMPILER_GCC 1
#endif


// add a macro to know we're compiling Lensfun, not a client library
#define CONF_LENSFUN_INTERNAL

#define VECTORIZATION_SSE
#define VECTORIZATION_SSE2

#define HAVE_ENDIAN_H

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#endif // __CONFIG_H__
