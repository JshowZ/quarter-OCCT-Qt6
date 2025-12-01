#include "Step2Stl.h"

// OCCT headers
#include <STEPControl_Reader.hxx>
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS_Shape.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopExp_Explorer.hxx>
#include <Message_ProgressRange.hxx>
#include <TCollection_AsciiString.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Mutex.hxx>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// Global variables for library state
static bool g_isInitialized = false;
static Standard_Mutex g_initializationMutex;

/**
 * @brief Default configuration values
 */
static const Step2Stl_Config DEFAULT_CONFIG = {
    0.1,    // meshTolerance (0.1 mm)
    0,      // useAsciiFormat (false, binary format)
    NULL,   // progressCallback (no callback)
    NULL    // userData (no user data)
};

/**
 * @brief Error messages corresponding to error codes
 */
static const char* ERROR_MESSAGES[] = {
    "Success",
    "File not found",
    "Invalid STEP file",
    "Failed to write STL file",
    "Memory allocation failed",
    "Invalid parameter",
    "Internal error"
};

/**
 * @brief Progress indicator implementation for OCCT
 *        Simplified implementation that doesn't use all OCCT progress indicator features
 */
class Step2Stl_ProgressIndicator
{
public:
    Step2Stl_ProgressIndicator(void (*callback)(int, void*), void* userData)
        : m_callback(callback), m_userData(userData), m_lastReportedProgress(-1)
    {}
    
    void reportProgress(double progress)
    {
        if (m_callback) {
            int intProgress = static_cast<int>(progress * 100.0);
            if (intProgress != m_lastReportedProgress) {
                m_callback(intProgress, m_userData);
                m_lastReportedProgress = intProgress;
            }
        }
    }
    
private:
    void (*m_callback)(int, void*);
    void* m_userData;
    int m_lastReportedProgress;
};

Step2Stl_ErrorCode Step2Stl_Initialize()
{
    Standard_Mutex::Sentry sentry(g_initializationMutex);
    
    if (g_isInitialized) {
        return STEP2STL_SUCCESS;
    }
    
    try {
        // OCCT initialization is handled automatically when using dynamic libraries
        g_isInitialized = true;
        return STEP2STL_SUCCESS;
    }
    catch (const Standard_Failure&) {
        return STEP2STL_ERROR_INTERNAL;
    }
    catch (...) {
        return STEP2STL_ERROR_INTERNAL;
    }
}

void Step2Stl_Cleanup()
{
    Standard_Mutex::Sentry sentry(g_initializationMutex);
    
    if (g_isInitialized) {
        // OCCT cleanup is handled automatically when using dynamic libraries
        g_isInitialized = false;
    }
}

Step2Stl_ErrorCode Step2Stl_Convert(const char* stepFilePath, const char* stlFilePath, const Step2Stl_Config* config)
{
    if (!stepFilePath || !stlFilePath) {
        return STEP2STL_ERROR_INVALID_PARAMETER;
    }
    
    Standard_Mutex::Sentry sentry(g_initializationMutex);
    
    if (!g_isInitialized) {
        // Auto-initialize if not already initialized
        Step2Stl_ErrorCode initResult = Step2Stl_Initialize();
        if (initResult != STEP2STL_SUCCESS) {
            return initResult;
        }
    }
    
    try {
        // Use default config if none provided
        const Step2Stl_Config& actualConfig = (config != NULL) ? *config : DEFAULT_CONFIG;
        
        // Check if STEP file exists
        FILE* file = fopen(stepFilePath, "r");
        if (!file) {
            return STEP2STL_ERROR_FILE_NOT_FOUND;
        }
        fclose(file);
        
        // Create STEP reader
        STEPControl_Reader reader;
        
        // Read STEP file
        IFSelect_ReturnStatus readStatus = reader.ReadFile(TCollection_AsciiString(stepFilePath).ToCString());
        if (readStatus != IFSelect_RetDone) {
            return STEP2STL_ERROR_INVALID_STEP_FILE;
        }
        
        // Transfer shapes from STEP file
        reader.TransferRoots();
        
        // Get number of shapes
        int numShapes = reader.NbRootsForTransfer();
        if (numShapes == 0) {
            return STEP2STL_ERROR_INVALID_STEP_FILE;
        }
        
        // Create progress indicator if callback is provided
        Step2Stl_ProgressIndicator* progressIndicator = nullptr;
        if (actualConfig.progressCallback) {
            progressIndicator = new Step2Stl_ProgressIndicator(actualConfig.progressCallback, actualConfig.userData);
        }
        
        // Process each shape
        std::vector<TopoDS_Shape> shapes;
        for (int i = 1; i <= numShapes; ++i) {
            // Transfer the root shape
            reader.TransferRoot(i);
            // Get the shape
            TopoDS_Shape shape = reader.Shape(i);
            if (!shape.IsNull()) {
                shapes.push_back(shape);
            }
        }
        
        // Delete progress indicator if it was created
        delete progressIndicator;
        progressIndicator = nullptr;
        
        if (shapes.empty()) {
            return STEP2STL_ERROR_INVALID_STEP_FILE;
        }
        
        // Create STL writer
        StlAPI_Writer writer;
        writer.ASCIIMode() = (actualConfig.useAsciiFormat != 0);
        
        // Process each shape for STL export
        for (size_t i = 0; i < shapes.size(); ++i) {
            const TopoDS_Shape& shape = shapes[i];
            
            // Create mesh for the shape
            BRepMesh_IncrementalMesh meshBuilder(shape, actualConfig.meshTolerance);
            meshBuilder.Perform();
            
            if (!meshBuilder.IsDone()) {
                return STEP2STL_ERROR_INTERNAL;
            }
            
            // Export the shape
            std::string outputPath;
            if (shapes.size() == 1) {
                // Single shape, export directly
                outputPath = stlFilePath;
            } else {
                // Multiple shapes, export each to separate file
                std::string basePath = stlFilePath;
                size_t dotPos = basePath.find_last_of('.');
                if (dotPos != std::string::npos) {
                    basePath = basePath.substr(0, dotPos);
                }
                char buffer[32];
                sprintf(buffer, "_shape%zu.stl", i + 1);
                outputPath = basePath + buffer;
            }
            
            // Write STL file
            if (!writer.Write(shape, outputPath.c_str())) {
                return STEP2STL_ERROR_STL_WRITE_FAILED;
            }
            
            // Update progress if callback is provided
            if (actualConfig.progressCallback) {
                int progress = static_cast<int>(((i + 1.0) / shapes.size()) * 100.0);
                actualConfig.progressCallback(progress, actualConfig.userData);
            }
        }
        
        return STEP2STL_SUCCESS;
    }
    catch (const Standard_Failure& e) {
        // Handle OCCT exceptions
        return STEP2STL_ERROR_INTERNAL;
    }
    catch (const std::bad_alloc&) {
        // Handle memory allocation exceptions
        return STEP2STL_ERROR_MEMORY_ALLOCATION;
    }
    catch (...) {
        // Handle other exceptions
        return STEP2STL_ERROR_INTERNAL;
    }
}

const char* Step2Stl_GetErrorMessage(Step2Stl_ErrorCode errorCode)
{
    if (errorCode >= 0 && errorCode < sizeof(ERROR_MESSAGES) / sizeof(ERROR_MESSAGES[0])) {
        return ERROR_MESSAGES[errorCode];
    }
    return "Unknown error";
}
