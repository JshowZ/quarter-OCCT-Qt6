#include "DataProcess.h"

// OCCT headers for STEP import
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>
#include <Message_ProgressRange.hxx>

// OCCT headers for curve extraction
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <gp_Pnt.hxx>

DataProcess::DataProcess()
{
    // Initialize OCCT if needed
}

DataProcess::~DataProcess()
{
    // Cleanup if needed
}

bool DataProcess::convertSTEPToSTL(const std::string& stepFilePath, 
                                 const std::string& stlFilePath, 
                                 double deflection, 
                                 bool asciiMode)
{
    try
    {
        // Create STEP reader
        STEPControl_Reader reader;
        Message_ProgressRange emptyProgress;
        
        // Read STEP file
        IFSelect_ReturnStatus status = reader.ReadFile(stepFilePath.c_str());
        if (status != IFSelect_RetDone)
        {
            setLastError("Failed to read STEP file: " + stepFilePath);
            return false;
        }
        
        // Transfer roots
        int nbRoots = reader.TransferRoots(emptyProgress);
        if (nbRoots == 0)
        {
            setLastError("No shapes found in STEP file: " + stepFilePath);
            return false;
        }
        
        // Get the shape
        TopoDS_Shape shape = reader.OneShape();
        if (shape.IsNull())
        {
            setLastError("Failed to get shape from STEP file: " + stepFilePath);
            return false;
        }
        
        // Create mesh
        BRepMesh_IncrementalMesh meshBuilder(shape, deflection);
        meshBuilder.Perform();
        if (!meshBuilder.IsDone())
        {
            setLastError("Failed to mesh shape from STEP file: " + stepFilePath);
            return false;
        }
        
        // Create STL writer
        StlAPI_Writer writer;
        writer.ASCIIMode() = asciiMode;
        
        // Write STL file
        if (!writer.Write(shape, stlFilePath.c_str()))
        {
            setLastError("Failed to write STL file: " + stlFilePath);
            return false;
        }
        
        return true;
    }
    catch (const Standard_Failure& e)
    {
        setLastError("OCCT exception: " + std::string(e.GetMessageString()));
        return false;
    }
    catch (const std::exception& e)
    {
        setLastError("Exception: " + std::string(e.what()));
        return false;
    }
    catch (...)
    {
        setLastError("Unknown error occurred during STEP to STL conversion");
        return false;
    }
}

bool DataProcess::convertIGESToSTL(const std::string& igesFilePath, 
                                 const std::string& stlFilePath, 
                                 double deflection, 
                                 bool asciiMode)
{
    try
    {
        // Create IGES reader
        IGESControl_Reader reader;
        Message_ProgressRange emptyProgress;
        
        // Read IGES file
        IFSelect_ReturnStatus status = reader.ReadFile(igesFilePath.c_str());
        if (status != IFSelect_RetDone)
        {
            setLastError("Failed to read IGES file: " + igesFilePath);
            return false;
        }
        
        // Transfer roots
        int nbRoots = reader.TransferRoots(emptyProgress);
        if (nbRoots == 0)
        {
            setLastError("No shapes found in IGES file: " + igesFilePath);
            return false;
        }
        
        // Get the shape
        TopoDS_Shape shape = reader.OneShape();
        if (shape.IsNull())
        {
            setLastError("Failed to get shape from IGES file: " + igesFilePath);
            return false;
        }
        
        // Create mesh
        BRepMesh_IncrementalMesh meshBuilder(shape, deflection);
        meshBuilder.Perform();
        if (!meshBuilder.IsDone())
        {
            setLastError("Failed to mesh shape from IGES file: " + igesFilePath);
            return false;
        }
        
        // Create STL writer
        StlAPI_Writer writer;
        writer.ASCIIMode() = asciiMode;
        
        // Write STL file
        if (!writer.Write(shape, stlFilePath.c_str()))
        {
            setLastError("Failed to write STL file: " + stlFilePath);
            return false;
        }
        
        return true;
    }
    catch (const Standard_Failure& e)
    {
        setLastError("OCCT exception: " + std::string(e.GetMessageString()));
        return false;
    }
    catch (const std::exception& e)
    {
        setLastError("Exception: " + std::string(e.what()));
        return false;
    }
    catch (...)
    {
        setLastError("Unknown error occurred during IGES to STL conversion");
        return false;
    }
}

std::string DataProcess::getLastError() const
{
    return m_lastError;
}

void DataProcess::setLastError(const std::string& errorMessage)
{
    m_lastError = errorMessage;
}

CurveCollection DataProcess::readStepCurves(const std::string& stepFilePath, double tolerance)
{
    CurveCollection result;
    
    try
    {
        // Create STEP reader
        STEPControl_Reader reader;
        Message_ProgressRange emptyProgress;
        
        // Read STEP file
        IFSelect_ReturnStatus status = reader.ReadFile(stepFilePath.c_str());
        if (status != IFSelect_RetDone)
        {
            setLastError("Failed to read STEP file: " + stepFilePath);
            return result;
        }
        
        // Transfer roots
        int nbRoots = reader.TransferRoots(emptyProgress);
        if (nbRoots == 0)
        {
            setLastError("No shapes found in STEP file: " + stepFilePath);
            return result;
        }
        
        // Get the shape
        TopoDS_Shape shape = reader.OneShape();
        if (shape.IsNull())
        {
            setLastError("Failed to get shape from STEP file: " + stepFilePath);
            return result;
        }
        
        // Extract all edges from the shape
        TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
        while (edgeExplorer.More())
        {
            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
            CurvePoints curvePoints;
            
            // Get the curve from the edge
            Standard_Real firstParam, lastParam;
            Handle(Geom_Curve) geomCurve = BRep_Tool::Curve(edge, firstParam, lastParam);
            if (!geomCurve.IsNull())
            {
                // Create curve adaptor
                GeomAdaptor_Curve adaptorCurve(geomCurve, firstParam, lastParam);
                
                // Discretize the curve using GCPnts_TangentialDeflection
                GCPnts_TangentialDeflection discretizer;
                Standard_Real angular = 0.1; // Angular deflection in radians
                
                // Initialize discretizer
                discretizer.Initialize(adaptorCurve, tolerance, angular, firstParam, lastParam);
                
                // Get number of points
                Standard_Integer nbPoints = discretizer.NbPoints();
                
                if (nbPoints > 0)
                {
                    // Get the points
                    for (Standard_Integer j = 1; j <= nbPoints; ++j)
                    {
                        gp_Pnt pnt = discretizer.Value(j);
                        Point3D point;
                        point.x = pnt.X();
                        point.y = pnt.Y();
                        point.z = pnt.Z();
                        curvePoints.points.push_back(point);
                    }
                    
                    // Add the curve to the collection
                    if (!curvePoints.points.empty())
                    {
                        result.curves.push_back(curvePoints);
                    }
                }
            }
            
            edgeExplorer.Next();
        }
        
        return result;
    }
    catch (const Standard_Failure& e)
    {
        setLastError("OCCT exception: " + std::string(e.GetMessageString()));
        return result;
    }
    catch (const std::exception& e)
    {
        setLastError("Exception: " + std::string(e.what()));
        return result;
    }
    catch (...)
    {
        setLastError("Unknown error occurred during STEP curve extraction");
        return result;
    }
}

CurveCollection DataProcess::readIgesCurves(const std::string& igesFilePath, double tolerance)
{
    CurveCollection result;
    
    try
    {
        // Create IGES reader
        IGESControl_Reader reader;
        Message_ProgressRange emptyProgress;
        
        // Read IGES file
        IFSelect_ReturnStatus status = reader.ReadFile(igesFilePath.c_str());
        if (status != IFSelect_RetDone)
        {
            setLastError("Failed to read IGES file: " + igesFilePath);
            return result;
        }
        
        // Transfer roots
        int nbRoots = reader.TransferRoots(emptyProgress);
        if (nbRoots == 0)
        {
            setLastError("No shapes found in IGES file: " + igesFilePath);
            return result;
        }
        
        // Get the shape
        TopoDS_Shape shape = reader.OneShape();
        if (shape.IsNull())
        {
            setLastError("Failed to get shape from IGES file: " + igesFilePath);
            return result;
        }
        
        // Extract all edges from the shape
        TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
        while (edgeExplorer.More())
        {
            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
            CurvePoints curvePoints;
            
            // Get the curve from the edge
            Standard_Real firstParam, lastParam;
            Handle(Geom_Curve) geomCurve = BRep_Tool::Curve(edge, firstParam, lastParam);
            if (!geomCurve.IsNull())
            {
                // Create curve adaptor
                GeomAdaptor_Curve adaptorCurve(geomCurve, firstParam, lastParam);
                
                // Discretize the curve using GCPnts_TangentialDeflection
                GCPnts_TangentialDeflection discretizer;
                Standard_Real angular = 0.1; // Angular deflection in radians
                
                // Initialize discretizer
                discretizer.Initialize(adaptorCurve, tolerance, angular, firstParam, lastParam);
                
                // Get number of points
                Standard_Integer nbPoints = discretizer.NbPoints();
                
                if (nbPoints > 0)
                {
                    // Get the points
                    for (Standard_Integer j = 1; j <= nbPoints; ++j)
                    {
                        gp_Pnt pnt = discretizer.Value(j);
                        Point3D point;
                        point.x = pnt.X();
                        point.y = pnt.Y();
                        point.z = pnt.Z();
                        curvePoints.points.push_back(point);
                    }
                    
                    // Add the curve to the collection
                    if (!curvePoints.points.empty())
                    {
                        result.curves.push_back(curvePoints);
                    }
                }
            }
            
            edgeExplorer.Next();
        }
        
        return result;
    }
    catch (const Standard_Failure& e)
    {
        setLastError("OCCT exception: " + std::string(e.GetMessageString()));
        return result;
    }
    catch (const std::exception& e)
    {
        setLastError("Exception: " + std::string(e.what()));
        return result;
    }
    catch (...)
    {
        setLastError("Unknown error occurred during IGES curve extraction");
        return result;
    }
}
