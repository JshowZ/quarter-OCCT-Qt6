#pragma once

#include <string>
#include <vector>
#include "DataProcessGlobal.h"

/**
 * @brief 3D point structure representing a point in 3D space
 */
struct DATAPROCESS_API Point3D {
    double x;
    double y;
    double z;
};

/**
 * @brief Curve points structure containing all points for a single curve
 */
struct DATAPROCESS_API CurvePoints {
    std::vector<Point3D> points; ///< Array of points representing the curve
};

/**
 * @brief Curve collection structure containing all curves read from a file
 */
struct DATAPROCESS_API CurveCollection {
    std::vector<CurvePoints> curves; ///< Array of curves
};

/**
 * @brief The DataProcess class provides functionality to convert CAD files to STL format
 * using Open CASCADE Technology (OCCT).
 */
class DATAPROCESS_API DataProcess {
public:
    /**
     * @brief Default constructor
     */
    DataProcess();
    
    /**
     * @brief Destructor
     */
    ~DataProcess();
    
    /**
     * @brief Convert a STEP file to STL format
     * @param stepFilePath Path to the input STEP file
     * @param stlFilePath Path to the output STL file
     * @param deflection Mesh deflection tolerance (default: 0.01)
     * @param asciiMode Whether to use ASCII format for STL (default: false, i.e., binary format)
     * @return true if conversion succeeded, false otherwise
     */
    bool convertSTEPToSTL(const std::string& stepFilePath, const std::string& stlFilePath, 
                         double deflection = 0.01, bool asciiMode = false);
    
    /**
     * @brief Convert an IGES file to STL format
     * @param igesFilePath Path to the input IGES file
     * @param stlFilePath Path to the output STL file
     * @param deflection Mesh deflection tolerance (default: 0.01)
     * @param asciiMode Whether to use ASCII format for STL (default: false, i.e., binary format)
     * @return true if conversion succeeded, false otherwise
     */
    bool convertIGESToSTL(const std::string& igesFilePath, const std::string& stlFilePath, 
                         double deflection = 0.01, bool asciiMode = false);
    
    /**
     * @brief Get the last error message
     * @return The last error message
     */
    std::string getLastError() const;
    
    /**
     * @brief Read curves from a STEP file and return them as point sets
     * @param stepFilePath Path to the input STEP file
     * @param tolerance Tolerance for curve discretization in millimeters (default: 0.1 mm)
     * @return Curve collection containing all curves from the file
     */
    CurveCollection readStepCurves(const std::string& stepFilePath, double tolerance = 0.1);
    
    /**
     * @brief Read curves from an IGES file and return them as point sets
     * @param igesFilePath Path to the input IGES file
     * @param tolerance Tolerance for curve discretization in millimeters (default: 0.1 mm)
     * @return Curve collection containing all curves from the file
     */
    CurveCollection readIgesCurves(const std::string& igesFilePath, double tolerance = 0.1);
    
private:
    /**
     * @brief Set the last error message
     * @param errorMessage The error message to set
     */
    void setLastError(const std::string& errorMessage);
    
    std::string m_lastError;
};
