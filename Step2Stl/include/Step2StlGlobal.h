#pragma once

// Windows DLL export/import macros
#ifdef _WIN32
    #ifdef STEP2STL_EXPORTS
        #define STEP2STL_API __declspec(dllexport)
    #else
        #define STEP2STL_API __declspec(dllimport)
    #endif
#else
    #define STEP2STL_API
#endif
