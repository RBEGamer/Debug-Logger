#pragma once



#ifndef FRM_DEFINES
#define FRM_DEFINES

//OS DEFINES
#if defined(__WIN__) || defined(_WIN32)
#define FRM_OS_WIN
#endif
#if defined(_WIN32)
#define FRM_OS_WIN
#define FRM_OS_WIN_32
#endif
#if defined(_WIN64)
#define FRM_OS_WIN
#define FRM_OS_WIN_64
#endif
#if defined(_LINUX) || defined(__LINUX__) || defined(__APPL__) || defined(__APPLE__)
#defined FRM_OS_UNIX_BASED
#endif
#if defined(_LINUX) || defined(__LINUX__)
#defined FRM_OS_LINUX
#endif
#if defined(__APPL__) || defined(__APPLE__)
#defined FRM_OS_OSX
#endif
#if defined(DENOVO)
#define FRM_OS_XBOXONE
#define FRM_CONSOLE
#endif
//OTHER = UNKNOWN
#if not defined(FRM_OS_WIN) || not defined(FRM_OS_UNIX_BASED) || not defined(FRM_CONSOLE)
#define FRM_OS_UNKNOWN
#endif
//BUILD_SCHEME_DEFINES
#if defined(RELEASE) || defined(_RELEASE)
#define FRM_BUILD_RELEASE
#endif
#if defined(DEBUG) || defined(_DEBUG)
#define FRM_BUILD_DEBUG
#endif






#endif // !FRM_DEFINES
