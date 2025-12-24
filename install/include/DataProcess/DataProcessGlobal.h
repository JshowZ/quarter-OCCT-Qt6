#pragma once

// Dynamic library export macros for DataProcess
#ifdef DATAPROCESS_EXPORTS
    #define DATAPROCESS_API __declspec(dllexport)
#else
    #define DATAPROCESS_API __declspec(dllimport)
#endif
