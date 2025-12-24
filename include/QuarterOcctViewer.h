#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <string>

// Forward declarations for Coin3D and Quarter
class SoNode;
class SoSeparator;
class SoPerspectiveCamera;
namespace SIM {
namespace Coin3D {
namespace Quarter {
    class QuarterWidget;
}
}
}

class QSlider;
class QWheelEvent;

// Forward declarations for OCCT
class TopoDS_Shape;


/**
 * @brief The QuarterOcctViewer class provides a Qt window for displaying OCCT models using Quarter (Coin3D)
 * 
 * This class implements the conversion from OCCT TopoDS_Shape to Coin3D SoNode, and provides basic
 * view manipulation functionality like rotation, zoom, and pan.
 */
class QuarterOcctViewer : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit QuarterOcctViewer(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~QuarterOcctViewer() override;
    
    /**
     * @brief Load and display a STEP file
     * @param filePath Path to the STEP file
     * @return true if successful, false otherwise
     */
    bool loadSTEPFile(const std::string& filePath);
    
    /**
     * @brief Load and display an IGES file
     * @param filePath Path to the IGES file
     * @return true if successful, false otherwise
     */
    bool loadIGESFile(const std::string& filePath);
    
    /**
     * @brief Display an OCCT shape
     * @param shape The OCCT shape to display
     * @return true if successful, false otherwise
     */
    bool displayShape(const TopoDS_Shape& shape);
    
    /**
     * @brief Clear the current scene
     */
    void clearScene();
    
    /**
     * @brief Set the background color
     * @param color RGB color values (0.0 to 1.0)
     */
    void setBackgroundColor(float r, float g, float b);
    
    /**
     * @brief Enable/disable wireframe mode
     * @param wireframe true for wireframe, false for solid
     */
    void setWireframe(bool wireframe);
    
    /**
     * @brief Zoom to fit the entire scene
     */
    void zoomAll();
    
    /**
     * @brief Handle mouse wheel events for zooming
     * @param event Mouse wheel event
     */
    void wheelEvent(QWheelEvent* event) override;
    
protected:
    /**
     * @brief Setup the UI components
     */
    void setupUI();
    
    /**
     * @brief Convert OCCT TopoDS_Shape to Coin3D SoNode
     * @param shape The OCCT shape to convert
     * @return Converted Coin3D node, or nullptr if conversion failed
     */
    SoNode* convertOcctShapeToCoin3D(const TopoDS_Shape& shape);
    
    /**
     * @brief Read a STEP file and return the shape
     * @param filePath Path to the STEP file
     * @param shape Output shape
     * @return true if successful, false otherwise
     */
    bool readSTEPFile(const std::string& filePath, TopoDS_Shape& shape);
    
    /**
     * @brief Read an IGES file and return the shape
     * @param filePath Path to the IGES file
     * @param shape Output shape
     * @return true if successful, false otherwise
     */
    bool readIGESFile(const std::string& filePath, TopoDS_Shape& shape);
    
private slots:
    /**
     * @brief Slot for wireframe checkbox
     * @param checked Checkbox state
     */
    void onWireframeChanged(bool checked);
    
    /**
     * @brief Slot for zoom slider
     * @param value Slider value
     */
    void onZoomChanged(int value);
    
    /**
     * @brief Slot for reset view action
     */
    void onResetView();
    
    /**
     * @brief Slot for about dialog
     */
    void onAbout();
    
private:
    QWidget* m_centralWidget;            // Central widget
    QVBoxLayout* m_mainLayout;            // Main layout
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget; // Quarter widget for Coin3D rendering
    SoSeparator* m_root;                  // Root of the Coin3D scene graph
    SoSeparator* m_modelRoot;             // Root for the model geometry
    SoPerspectiveCamera* m_camera;        // Camera for the scene
    QSlider* m_zoomSlider;                // Zoom control slider
    bool m_wireframeMode;                 // Wireframe mode flag
};
