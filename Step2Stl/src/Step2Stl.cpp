#include "Step2Stl.h"

// OCCT headers
#include <STEPControl_Reader.hxx>
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <TopExp_Explorer.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Message_ProgressRange.hxx>
#include <TCollection_AsciiString.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Mutex.hxx>
#include <GCPnts_TangentialDeflection.hxx>

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
    1,      // doStlConversion (true, perform STL conversion)
    0,      // doCurveExtraction (false, don't extract curves)
    0.1,    // meshTolerance (0.1 mm)
    0.1,    // curveTolerance (0.1 mm)
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
    
    // Create config for STL conversion only
    Step2Stl_Config actualConfig;
    if (config) {
        // Copy existing config
        actualConfig = *config;
    } else {
        // Use default config
        actualConfig = DEFAULT_CONFIG;
    }
    
    // Force STL conversion and disable curve extraction
    actualConfig.doStlConversion = 1;
    actualConfig.doCurveExtraction = 0;
    
    // Call the unified processing function
    Step2Stl_Result result;
    return Step2Stl_ProcessStepFile(stepFilePath, stlFilePath, &actualConfig, &result);
}

const char* Step2Stl_GetErrorMessage(Step2Stl_ErrorCode errorCode)
{
    if (errorCode >= 0 && errorCode < sizeof(ERROR_MESSAGES) / sizeof(ERROR_MESSAGES[0])) {
        return ERROR_MESSAGES[errorCode];
    }
    return "Unknown error";
}

Step2Stl_ErrorCode Step2Stl_ReadStepCurves(const char* stepFilePath, Step2Stl_CurveCollection* curveCollection, double tolerance)
{
    if (!stepFilePath || !curveCollection) {
        return STEP2STL_ERROR_INVALID_PARAMETER;
    }
    
    // Create config for curve extraction only
    Step2Stl_Config actualConfig = DEFAULT_CONFIG;
    actualConfig.doStlConversion = 0; // Disable STL conversion
    actualConfig.doCurveExtraction = 1; // Enable curve extraction
    
    // Set curve tolerance if specified
    if (tolerance > 0.0) {
        actualConfig.curveTolerance = tolerance;
    }
    
    // Call the unified processing function
    Step2Stl_Result result;
    Step2Stl_ErrorCode error = Step2Stl_ProcessStepFile(stepFilePath, NULL, &actualConfig, &result);
    
    if (error == STEP2STL_SUCCESS) {
        // Copy curve data to output
        *curveCollection = result.curveCollection;
        
        // Clear the curve collection in the result so it doesn't get freed
        result.curveCollection.curves = nullptr;
        result.curveCollection.numCurves = 0;
    }
    
    // Free the result (this won't free the curve data we just copied)
    Step2Stl_FreeResult(&result);
    
    return error;
}

Step2Stl_ErrorCode Step2Stl_FreeCurveData(Step2Stl_CurveCollection* curveCollection)
{
    if (!curveCollection) {
        return STEP2STL_ERROR_INVALID_PARAMETER;
    }
    
    // Free points for each curve
    if (curveCollection->curves) {
        for (size_t i = 0; i < curveCollection->numCurves; ++i) {
            if (curveCollection->curves[i].points) {
                delete[] curveCollection->curves[i].points;
                curveCollection->curves[i].points = nullptr;
            }
        }
        
        // Free the curves array
        delete[] curveCollection->curves;
        curveCollection->curves = nullptr;
    }
    
    // Reset the number of curves
    curveCollection->numCurves = 0;
    
    return STEP2STL_SUCCESS;
}

Step2Stl_ErrorCode Step2Stl_ProcessStepFile(const char* stepFilePath, const char* stlFilePath, const Step2Stl_Config* config, Step2Stl_Result* result)
{
    if (!stepFilePath || !result) {
        return STEP2STL_ERROR_INVALID_PARAMETER;
    }
    
    // If STL conversion is requested, stlFilePath must be provided
    const Step2Stl_Config& actualConfig = (config != NULL) ? *config : DEFAULT_CONFIG;
    if (actualConfig.doStlConversion && !stlFilePath) {
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
        // Initialize result
        memset(result, 0, sizeof(Step2Stl_Result));
        result->stlSuccess = 0;
        result->curveCollection.curves = nullptr;
        result->curveCollection.numCurves = 0;
        
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
        
        // Collect all shapes
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
        
        if (shapes.empty()) {
            return STEP2STL_ERROR_INVALID_STEP_FILE;
        }
        
        // Create progress indicator if callback is provided
        Step2Stl_ProgressIndicator* progressIndicator = nullptr;
        if (actualConfig.progressCallback) {
            progressIndicator = new Step2Stl_ProgressIndicator(actualConfig.progressCallback, actualConfig.userData);
        }
        
        // Process STL conversion if requested
        if (actualConfig.doStlConversion) {
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
                    delete progressIndicator;
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
                    delete progressIndicator;
                    return STEP2STL_ERROR_STL_WRITE_FAILED;
                }
                
                // Update progress if callback is provided
                if (actualConfig.progressCallback) {
                    int progress = static_cast<int>(((i + 1.0) / shapes.size()) * 50.0);
                    actualConfig.progressCallback(progress, actualConfig.userData);
                }
            }
            
            result->stlSuccess = 1;
        }
        
        // Extract curves if requested
        if (actualConfig.doCurveExtraction) {
            // Collect all edges from all shapes
            std::vector<TopoDS_Edge> allEdges;
            for (const auto& shape : shapes) {
                // Explore all edges in the shape
                TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
                while (edgeExplorer.More()) {
                    TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
                    allEdges.push_back(edge);
                    edgeExplorer.Next();
                }
            }
            
            if (!allEdges.empty()) {
                // Allocate memory for curves
                try {
                    result->curveCollection.curves = new Step2Stl_CurvePoints[allEdges.size()];
                    result->curveCollection.numCurves = allEdges.size();
                } catch (const std::bad_alloc&) {
                    delete progressIndicator;
                    return STEP2STL_ERROR_MEMORY_ALLOCATION;
                }
                
                // Process each edge
                for (size_t i = 0; i < allEdges.size(); ++i) {
                    const TopoDS_Edge& edge = allEdges[i];
                    Step2Stl_CurvePoints& curvePoints = result->curveCollection.curves[i];
                    curvePoints.points = nullptr;
                    curvePoints.numPoints = 0;
                    
                    // Get the curve from the edge
                    Standard_Real firstParam, lastParam;
                    Handle(Geom_Curve) geomCurve = BRep_Tool::Curve(edge, firstParam, lastParam);
                    if (geomCurve.IsNull()) {
                        continue;
                    }
                    
                    // Create curve adaptor
                    GeomAdaptor_Curve adaptorCurve(geomCurve, firstParam, lastParam);
                    
                    // Discretize the curve using GCPnts_TangentialDeflection
                    GCPnts_TangentialDeflection discretizer;
                    Standard_Real deflection = actualConfig.curveTolerance;
                    Standard_Real angular = 0.1; // Angular deflection in radians
                    
                    // Initialize discretizer (returns void)
                    discretizer.Initialize(adaptorCurve, deflection, angular, firstParam, lastParam);
                    
                    // Get number of points
                    curvePoints.numPoints = discretizer.NbPoints();
                    
                    if (curvePoints.numPoints > 0) {
                        // Allocate memory for points
                        try {
                            curvePoints.points = new Step2Stl_Point[curvePoints.numPoints];
                        } catch (const std::bad_alloc&) {
                            // Free already allocated memory before returning
                            Step2Stl_FreeResult(result);
                            delete progressIndicator;
                            return STEP2STL_ERROR_MEMORY_ALLOCATION;
                        }
                        
                        // Get the points
                        for (size_t j = 0; j < curvePoints.numPoints; ++j) {
                            gp_Pnt pnt = discretizer.Value(j + 1);
                            curvePoints.points[j].x = pnt.X();
                            curvePoints.points[j].y = pnt.Y();
                            curvePoints.points[j].z = pnt.Z();
                        }
                    }
                    
                    // Update progress if callback is provided
                    if (actualConfig.progressCallback && actualConfig.doStlConversion) {
                        // If both operations are performed, STL is 0-50%, curve extraction is 50-100%
                        int progress = 50 + static_cast<int>(((i + 1.0) / allEdges.size()) * 50.0);
                        actualConfig.progressCallback(progress, actualConfig.userData);
                    } else if (actualConfig.progressCallback) {
                        // If only curve extraction is performed, it's 0-100%
                        int progress = static_cast<int>(((i + 1.0) / allEdges.size()) * 100.0);
                        actualConfig.progressCallback(progress, actualConfig.userData);
                    }
                }
            }
        }
        
        // Delete progress indicator if it was created
        delete progressIndicator;
        
        return STEP2STL_SUCCESS;
    }
    catch (const Standard_Failure&) {
        // Handle OCCT exceptions
        Step2Stl_FreeResult(result);
        return STEP2STL_ERROR_INTERNAL;
    }
    catch (const std::bad_alloc&) {
        // Handle memory allocation exceptions
        Step2Stl_FreeResult(result);
        return STEP2STL_ERROR_MEMORY_ALLOCATION;
    }
    catch (...) {
        // Handle other exceptions
        Step2Stl_FreeResult(result);
        return STEP2STL_ERROR_INTERNAL;
    }
}

Step2Stl_ErrorCode Step2Stl_FreeResult(Step2Stl_Result* result)
{
    if (!result) {
        return STEP2STL_ERROR_INVALID_PARAMETER;
    }
    
    // Free curve data if it exists
    Step2Stl_FreeCurveData(&result->curveCollection);
    
    // Reset result structure
    memset(result, 0, sizeof(Step2Stl_Result));
    
    return STEP2STL_SUCCESS;
}
