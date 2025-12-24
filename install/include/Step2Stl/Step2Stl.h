#pragma once

#include "Step2StlGlobal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes returned by Step2Stl functions
 */
typedef enum {
    STEP2STL_SUCCESS = 0,
    STEP2STL_ERROR_FILE_NOT_FOUND = 1,
    STEP2STL_ERROR_INVALID_STEP_FILE = 2,
    STEP2STL_ERROR_STL_WRITE_FAILED = 3,
    STEP2STL_ERROR_MEMORY_ALLOCATION = 4,
    STEP2STL_ERROR_INVALID_PARAMETER = 5,
    STEP2STL_ERROR_INTERNAL = 6
} Step2Stl_ErrorCode;

/**
 * @brief 3D point structure
 */
typedef struct {
    double x;
    double y;
    double z;
} Step2Stl_Point;

/**
 * @brief Curve points structure
 *        Contains all points for a single curve
 */
typedef struct {
    /**
     * @brief Array of points representing the curve
     *        Memory is allocated by Step2Stl_ReadStepCurves and must be freed by Step2Stl_FreeCurveData
     */
    Step2Stl_Point* points;
    
    /**
     * @brief Number of points in the curve
     */
    size_t numPoints;
} Step2Stl_CurvePoints;

/**
 * @brief Curve collection structure
 *        Contains all curves read from a STEP file
 */
typedef struct {
    /**
     * @brief Array of curves
     *        Memory is allocated by Step2Stl_ReadStepCurves and must be freed by Step2Stl_FreeCurveData
     */
    Step2Stl_CurvePoints* curves;
    
    /**
     * @brief Number of curves in the collection
     */
    size_t numCurves;
} Step2Stl_CurveCollection;

/**
 * @brief Configuration options for STEP processing
 */
typedef struct {
    /**
     * @brief Whether to perform STL conversion
     *        Default: true
     */
    int doStlConversion;
    
    /**
     * @brief Whether to extract curves as point sets
     *        Default: false
     */
    int doCurveExtraction;
    
    /**
     * @brief Mesh tolerance in millimeters for STL conversion
     *        Lower values create more detailed meshes but take longer to generate
     *        Default: 0.1 mm
     */
    double meshTolerance;
    
    /**
     * @brief Tolerance for curve discretization in millimeters
     *        Lower values create more detailed curves but take longer to generate
     *        Default: 0.1 mm
     */
    double curveTolerance;
    
    /**
     * @brief Whether to use ASCII STL format
     *        If false, binary STL format will be used
     *        Default: false (binary format)
     */
    int useAsciiFormat;
    
    /**
     * @brief Progress callback function
     *        Called periodically during processing with progress percentage (0-100)
     *        Can be NULL if progress tracking is not needed
     */
    void (*progressCallback)(int progress, void* userData);
    
    /**
     * @brief User data to pass to the progress callback
     */
    void* userData;
} Step2Stl_Config;

/**
 * @brief Result structure for STEP processing
 */
typedef struct {
    /**
     * @brief Whether STL conversion was successful
     *        Only valid if doStlConversion was true in the config
     */
    int stlSuccess;
    
    /**
     * @brief Curve collection extracted from the STEP file
     *        Only valid if doCurveExtraction was true in the config
     *        Memory is allocated by Step2Stl_ProcessStepFile and must be freed by Step2Stl_FreeResult
     */
    Step2Stl_CurveCollection curveCollection;
} Step2Stl_Result;

/**
 * @brief Initialize the Step2Stl library
 *        This function must be called before any other Step2Stl functions
 * @return STEP2STL_SUCCESS on success, error code otherwise
 */
STEP2STL_API Step2Stl_ErrorCode Step2Stl_Initialize();

/**
 * @brief Cleanup the Step2Stl library
 *        This function must be called when done using the library
 */
STEP2STL_API void Step2Stl_Cleanup();

/**
 * @brief Convert a STEP file to STL format
 * @param stepFilePath Path to the input STEP file (.step or .stp)
 * @param stlFilePath Path to the output STL file
 * @param config Conversion configuration (can be NULL for default settings)
 * @return STEP2STL_SUCCESS on success, error code otherwise
 */
STEP2STL_API Step2Stl_ErrorCode Step2Stl_Convert(const char* stepFilePath, const char* stlFilePath, const Step2Stl_Config* config);

/**
 * @brief Get a human-readable error message for an error code
 * @param errorCode The error code returned by a Step2Stl function
 * @return Pointer to a static error message string
 */
STEP2STL_API const char* Step2Stl_GetErrorMessage(Step2Stl_ErrorCode errorCode);

/**
 * @brief Read curves from a STEP file and return them as point sets
 * @param stepFilePath Path to the input STEP file (.step or .stp)
 * @param curveCollection Output parameter to store the curves read from the file
 * @param tolerance Tolerance for curve discretization in millimeters
 *                  Lower values create more detailed curves but take longer to generate
 *                  Default: 0.1 mm
 * @return STEP2STL_SUCCESS on success, error code otherwise
 * @note The caller must free the returned curve data using Step2Stl_FreeCurveData
 */
STEP2STL_API Step2Stl_ErrorCode Step2Stl_ReadStepCurves(const char* stepFilePath, Step2Stl_CurveCollection* curveCollection, double tolerance);

/**
 * @brief Free the memory allocated for curve data
 * @param curveCollection Curve data to free
 * @return STEP2STL_SUCCESS on success, error code otherwise
 */
STEP2STL_API Step2Stl_ErrorCode Step2Stl_FreeCurveData(Step2Stl_CurveCollection* curveCollection);

/**
 * @brief Process a STEP file with configurable options
 *        Can perform STL conversion, curve extraction, or both
 * @param stepFilePath Path to the input STEP file (.step or .stp)
 * @param stlFilePath Path to the output STL file (only used if doStlConversion is true)
 * @param config Configuration options for processing
 * @param result Output parameter to store the processing result
 * @return STEP2STL_SUCCESS on success, error code otherwise
 * @note If doCurveExtraction is true, the caller must free the curve data using Step2Stl_FreeResult
 */
STEP2STL_API Step2Stl_ErrorCode Step2Stl_ProcessStepFile(const char* stepFilePath, const char* stlFilePath, const Step2Stl_Config* config, Step2Stl_Result* result);

/**
 * @brief Free the memory allocated for processing result
 * @param result Result data to free
 * @return STEP2STL_SUCCESS on success, error code otherwise
 */
STEP2STL_API Step2Stl_ErrorCode Step2Stl_FreeResult(Step2Stl_Result* result);

#ifdef __cplusplus
}
#endif
