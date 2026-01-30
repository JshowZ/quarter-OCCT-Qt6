#include "TextDemoWidget.h"

// Qt headers
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>

// Coin3D and Quarter headers
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/SoPickedPoint.h>
#include <Quarter/QuarterWidget.h>
#include <Quarter/eventhandlers/DragDropHandler.h>

#include <Standard.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Point.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <IMeshTools_Parameters.hxx>
#include <Precision.hxx>
#include <gp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>

#include "ViewTool.h"
#include "base.h"
#include "TextShape.h"
#include "ShapeUtil.h"

using namespace SIM::Coin3D::Quarter;

TextDemoWidget::TextDemoWidget(QWidget *parent)
    : QMainWindow(parent),
      m_centralWidget(nullptr),
      m_mainLayout(nullptr),
      m_controlPanel(nullptr),
      m_quarterWidget(nullptr),
      m_root(nullptr),
      m_modelRoot(nullptr),
      m_camera(nullptr),
      m_textInput(nullptr),
      m_fontComboBox(nullptr),
      m_textHeight(nullptr),
      m_createTextBtn(nullptr),
      m_clearBtn(nullptr),
      m_zoomAllBtn(nullptr)
{
    setupUI();
}

TextDemoWidget::~TextDemoWidget()
{
    // Cleanup Coin3D nodes
    if (m_root) {
        m_root->unref();
    }
}

void TextDemoWidget::setupUI()
{
    // Create central widget and main layout
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create Quarter widget for 3D rendering
    m_quarterWidget = new SIM::Coin3D::Quarter::QuarterWidget(m_centralWidget);
    m_mainLayout->addWidget(m_quarterWidget);

    this->m_quarterWidget->installEventFilter(new SIM::Coin3D::Quarter::DragDropHandler(this->m_quarterWidget));
    //set default navigation mode file
    this->m_quarterWidget->setNavigationModeFile();
    
    // Create control panel
    m_controlPanel = new QWidget(m_centralWidget);
    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlPanel);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    
    // Text input
    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->addWidget(new QLabel("Text:", m_controlPanel));
    m_textInput = new QLineEdit(m_controlPanel);
    m_textInput->setText("HELLO WORLD");
    textLayout->addWidget(m_textInput);
    controlLayout->addLayout(textLayout);
    
    // Font selection
    QHBoxLayout* fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel("Font:", m_controlPanel));
    m_fontComboBox = new QComboBox(m_controlPanel);
    
    // Initialize and load system fonts
    const std::vector<std::string> &systemFonts = Base::TextShape::InitOcctFonts();
    for (const std::string& fontName : systemFonts) {
        m_fontComboBox->addItem(fontName.c_str());
    }
    
    // Add default fonts as fallback
    if (m_fontComboBox->count() == 0) {
        m_fontComboBox->addItem("Arial");
        m_fontComboBox->addItem("Times New Roman");
        m_fontComboBox->addItem("Courier New");
        m_fontComboBox->addItem("Verdana");
        m_fontComboBox->addItem("Georgia");
    }
    
    fontLayout->addWidget(m_fontComboBox);
    controlLayout->addLayout(fontLayout);
    
    // Text parameters
    QHBoxLayout* textParamLayout = new QHBoxLayout();
    textParamLayout->addWidget(new QLabel("Text Height:", m_controlPanel));
    m_textHeight = new QDoubleSpinBox(m_controlPanel);
    m_textHeight->setRange(0.1, 50.0);
    m_textHeight->setValue(2.0);
    m_textHeight->setSingleStep(0.1);
    textParamLayout->addWidget(m_textHeight);
    controlLayout->addLayout(textParamLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_createTextBtn = new QPushButton("Create Text", m_controlPanel);
    connect(m_createTextBtn, &QPushButton::clicked, this, &TextDemoWidget::onCreateText);
    buttonLayout->addWidget(m_createTextBtn);
    
    m_clearBtn = new QPushButton("Clear", m_controlPanel);
    connect(m_clearBtn, &QPushButton::clicked, this, &TextDemoWidget::onClear);
    buttonLayout->addWidget(m_clearBtn);
    
    m_zoomAllBtn = new QPushButton("Zoom All", m_controlPanel);
    connect(m_zoomAllBtn, &QPushButton::clicked, this, &TextDemoWidget::onZoomAll);
    buttonLayout->addWidget(m_zoomAllBtn);
    
    controlLayout->addLayout(buttonLayout);
    
    m_mainLayout->addWidget(m_controlPanel);
    m_mainLayout->setStretch(0, 1); // Make quarter widget expand
    
    // Create menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("File");
    
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("Help");
    
    QAction* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About Text Demo Widget", 
            "Text Demo Widget v1.0\n" 
            "A Qt application for rendering text in different fonts using OCCT\n" 
            "and displaying it in Coin3D Quarter\n" 
            "\n" 
            "Features:\n" 
            "- Create text in different fonts\n" 
            "- Adjust text height\n" 
            "- 3D visualization\n" 
            "\n" 
            "Uses:\n" 
            "- Qt Framework\n" 
            "- Coin3D\n" 
            "- Quarter\n" 
            "- Open CASCADE Technology (OCCT)");
    });
    
    // Initialize Coin3D interaction system (for mouse rotation, pan, zoom)
    SoInteraction::init();
    
    // Initialize Coin3D scene
    m_root = new SoSeparator;
    m_root->ref();
    
    // Add camera
    m_camera = new SoPerspectiveCamera;
    m_camera->position.setValue(0, 0, 30);
    m_camera->heightAngle.setValue(M_PI / 4.0);
    m_camera->nearDistance.setValue(0.1);
    m_camera->farDistance.setValue(1000.0);
    m_root->addChild(m_camera);
    
    // Add light
    SoDirectionalLight* light = new SoDirectionalLight;
    light->direction.setValue(-0.5, -0.5, -1);
    m_root->addChild(light);
    
    // Add shape hints for better rendering
    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering.setValue(SoShapeHints::COUNTERCLOCKWISE);
    m_root->addChild(shapeHints);
    
    // Add draw style node for wireframe control
    SoDrawStyle* drawStyle = new SoDrawStyle;
    drawStyle->style.setValue(SoDrawStyle::FILLED);
    m_root->addChild(drawStyle);
    
    // Add material
    SoMaterial* material = new SoMaterial;
    material->diffuseColor.setValue(0.8, 0.8, 0.8);
    m_root->addChild(material);
    
    // Add model root separator
    m_modelRoot = new SoSeparator;
    m_root->addChild(m_modelRoot);
    
    // Set the scene graph to Quarter widget
    m_quarterWidget->setSceneGraph(m_root);
    
    // Set default background color to white
    m_quarterWidget->setBackgroundColor(QColor::fromRgbF(1.0, 1.0, 1.0));

    // Set window properties
    setWindowTitle("Text Demo Widget");
    resize(800, 600);
}

void TextDemoWidget::createText(const std::string& text, double height, const std::string& font)
{
    try {
        clearScene();
        
        TopoDS_Shape resultShape;
        double textWidth;
        
        // Call MakeTextShape to create the text shape
        bool success = Base::TextShape::MakeTextShape(
            text.c_str(),             // text
            font.c_str(),             // font
            static_cast<float>(height), // text height
            static_cast<float>(height * 0.1), // thickness (10% of height)
            false,                    // isBold
            false,                    // isItalic
            resultShape,              // resultShape
            textWidth                 // textWidth
        );
        
        if (success && !resultShape.IsNull()) {
            displayShape(resultShape);
        } else {
            QMessageBox::warning(this, "Warning", "Failed to create text shape, using fallback");
            
            // Fallback to placeholder box if text creation fails
            BRepPrimAPI_MakeBox boxMaker(height * 0.5, height, height * 0.1);
            displayShape(boxMaker.Shape());
        }
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error creating text");
    }
}

bool TextDemoWidget::displayShape(const TopoDS_Shape& shape, bool clearExisting)
{
    // Clear existing model if requested
    if (clearExisting) {
        clearScene();
    }
    
    // Convert OCCT shape to Coin3D node
    SoNode* modelNode = ShapeUtil::convertShapeRecursive(shape);
    if (modelNode) {
        m_modelRoot->addChild(modelNode);
        
        // Zoom to fit
        zoomAll();
        
        return true;
    }
    
    return false;
}

void TextDemoWidget::clearScene()
{
    // Remove all children from model root
    while (m_modelRoot->getNumChildren() > 0) {
        m_modelRoot->removeChild(0);
    }
}

void TextDemoWidget::zoomAll()
{
    if (m_modelRoot->getNumChildren() > 0) {
        // Calculate bounding box and adjust camera position
        m_quarterWidget->viewAll();
    }
}

void TextDemoWidget::onCreateText()
{
    std::string text = m_textInput->text().toStdString();
    double height = m_textHeight->value();
    std::string font = m_fontComboBox->currentText().toStdString();
    
    try {
        createText(text, height, font);
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create text: %1").arg(e.GetMessageString()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Failed to create text: Unknown error");
    }
}

void TextDemoWidget::onClear()
{
    clearScene();
}

void TextDemoWidget::onZoomAll()
{
    zoomAll();
}
