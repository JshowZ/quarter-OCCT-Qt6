#include "mainwindow.h"
#include "geometrydrawer.h"
#include "occtgeometry.h"
#include "quarterviewer.h"
#include <QColorDialog>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>

// OCCT STEP reading related headers
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopExp_Explorer.hxx>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_currentMode(OCCT_Point),
    m_isDrawing(false)
{
    // Set window title
    this->setWindowTitle("Geometry Drawing Tool");
    
    // Initialize UI
    initUI();
    
    // Create and initialize OCCT geometry drawer
    m_occtGeometry = new OCCTGeometry(this);
    
    // Create and initialize QuarterViewer
    m_quarterViewer = new QuarterViewer(this);
    //m_quarterViewer->initialize();
    
    // Add QuarterViewer to main layout
    m_mainLayout->addWidget(m_quarterViewer);
    
    // Connect signals and slots
    connect(m_quarterViewer, &QuarterViewer::mousePressed, this, &MainWindow::onMousePressed);
    connect(m_quarterViewer, &QuarterViewer::mouseMoved, this, &MainWindow::onMouseMoved);
    connect(m_quarterViewer, &QuarterViewer::mouseReleased, this, &MainWindow::onMouseReleased);
    connect(m_occtGeometry, &OCCTGeometry::geometryChanged, this, &MainWindow::updateShapes);
    
    // Set initial drawing mode
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(static_cast<OCCTGeometryType>(m_currentMode));
    
    // Connect menu item signals and slots
    connect(m_actionPoint, &QAction::triggered, this, &MainWindow::on_actionPoint_triggered);
    connect(m_actionLine, &QAction::triggered, this, &MainWindow::on_actionLine_triggered);
    connect(m_actionCurve, &QAction::triggered, this, &MainWindow::on_actionCurve_triggered);
    connect(m_actionRectangle, &QAction::triggered, this, &MainWindow::on_actionRectangle_triggered);
    connect(m_actionCircle, &QAction::triggered, this, &MainWindow::on_actionCircle_triggered);
    connect(m_actionEllipse, &QAction::triggered, this, &MainWindow::on_actionEllipse_triggered);
    connect(m_actionSetLineStyle, &QAction::triggered, this, &MainWindow::on_actionSetLineStyle_triggered);
    connect(m_actionSetColor, &QAction::triggered, this, &MainWindow::on_actionSetColor_triggered);
    connect(m_actionSetWidth, &QAction::triggered, this, &MainWindow::on_actionSetWidth_triggered);
    connect(m_actionOpenSTEP, &QAction::triggered, this, &MainWindow::on_actionOpenSTEP_triggered);
}

MainWindow::~MainWindow()
{
    // Clean up UI widgets
    delete m_quarterViewer;
    delete m_occtGeometry;
    delete m_mainLayout;
    delete m_centralWidget;
    // Menus and actions are managed automatically by Qt, no need to delete manually
}

// Initialize UI interface
void MainWindow::initUI()
{
    // Create central widget and main layout
    m_centralWidget = new QWidget(this);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Set central widget
    setCentralWidget(m_centralWidget);
    
    // Create menu bar
    m_menuBar = menuBar();
    
    // Create geometry menu
    m_geometryMenu = m_menuBar->addMenu("Geometry");
    
    // Create geometry menu items
    m_actionPoint = new QAction("Point", this);
    m_actionLine = new QAction("Line", this);
    m_actionCurve = new QAction("Curve", this);
    m_actionRectangle = new QAction("Rectangle", this);
    m_actionCircle = new QAction("Circle", this);
    m_actionEllipse = new QAction("Ellipse", this);
    
    // Add geometry menu items
    m_geometryMenu->addAction(m_actionPoint);
    m_geometryMenu->addAction(m_actionLine);
    m_geometryMenu->addAction(m_actionCurve);
    m_geometryMenu->addAction(m_actionRectangle);
    m_geometryMenu->addAction(m_actionCircle);
    m_geometryMenu->addAction(m_actionEllipse);
    
    // Create style menu
    m_styleMenu = m_menuBar->addMenu("Style");
    
    // Create style menu items
    m_actionSetLineStyle = new QAction("Set Line Style", this);
    m_actionSetColor = new QAction("Set Color", this);
    m_actionSetWidth = new QAction("Set Line Width", this);
    
    // Add style menu items
    m_styleMenu->addAction(m_actionSetLineStyle);
    m_styleMenu->addAction(m_actionSetColor);
    m_styleMenu->addAction(m_actionSetWidth);
    
    // Create file menu
    QMenu *fileMenu = m_menuBar->addMenu("File");
    
    // Create open STEP file menu item
    m_actionOpenSTEP = new QAction("Open STEP File", this);
    fileMenu->addAction(m_actionOpenSTEP);
}

void MainWindow::on_actionPoint_triggered()
{
    m_currentMode = OCCT_Point;
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(OCCT_Point);
    qDebug() << "Drawing mode set to: Point";
}

void MainWindow::on_actionLine_triggered()
{
    m_currentMode = OCCT_Line;
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(OCCT_Line);
    qDebug() << "Drawing mode set to: Line";
}

void MainWindow::on_actionCurve_triggered()
{
    m_currentMode = OCCT_Curve;
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(OCCT_Curve);
    qDebug() << "Drawing mode set to: Curve";
}

void MainWindow::on_actionRectangle_triggered()
{
    m_currentMode = OCCT_Rectangle;
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(OCCT_Rectangle);
    qDebug() << "Drawing mode set to: Rectangle";
}

void MainWindow::on_actionCircle_triggered()
{
    m_currentMode = OCCT_Circle;
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(OCCT_Circle);
    qDebug() << "Drawing mode set to: Circle";
}

void MainWindow::on_actionEllipse_triggered()
{
    m_currentMode = OCCT_Ellipse;
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(OCCT_Ellipse);
    qDebug() << "Drawing mode set to: Ellipse";
}

void MainWindow::on_actionSetLineStyle_triggered()
{
    // Open line style dialog
    QStringList styles = {"Solid", "Dashed", "Dotted"};
    QString style = QInputDialog::getItem(this, "Set Line Style", "Select line style:", styles, 0, false);
    
    if (!style.isEmpty()) {
        int styleIndex = styles.indexOf(style);
        m_occtGeometry->setLineStyle(styleIndex);
        qDebug() << "Line style set to:" << style;
    }
}

void MainWindow::on_actionSetColor_triggered()
{
    // Open color selection dialog
    QColor color = QColorDialog::getColor(Qt::black, this, "Select Color");
    
    if (color.isValid()) {
        m_occtGeometry->setLineColor(color);
        qDebug() << "Line color set to:" << color.name();
    }
}

void MainWindow::on_actionSetWidth_triggered()
{
    // Open line width dialog
    bool ok;
    int width = QInputDialog::getInt(this, "Set Line Width", "Line width (1-20):", 2, 1, 20, 1, &ok);
    
    if (ok) {
        m_occtGeometry->setLineWidth(width);
        qDebug() << "Line width set to:" << width;
    }
}

// Mouse press event handling
void MainWindow::onMousePressed(const gp_Pnt &point)
{
    m_isDrawing = true;
    m_occtGeometry->startDrawing(point);
    qDebug() << "Mouse pressed at:" << point.X() << "," << point.Y() << "," << point.Z();
}

// Mouse move event handling
void MainWindow::onMouseMoved(const gp_Pnt &point)
{
    if (m_isDrawing && m_currentMode == OCCT_Curve) {
        m_occtGeometry->continueDrawing(point);
        qDebug() << "Mouse moved at:" << point.X() << "," << point.Y() << "," << point.Z();
    }
}

// Mouse release event handling
void MainWindow::onMouseReleased(const gp_Pnt &point)
{
    if (m_isDrawing) {
        m_isDrawing = false;
        m_occtGeometry->finishDrawing(point);
        qDebug() << "Mouse released at:" << point.X() << "," << point.Y() << "," << point.Z();
    }
}

// Update shapes in view
void MainWindow::updateShapes()
{
    // Clear all current shapes
    m_quarterViewer->clearAllShapes();
    
    // Add all new shapes
    const auto &shapes = m_occtGeometry->getShapes();
    const auto &colors = m_occtGeometry->getColors();
    const auto &lineStyles = m_occtGeometry->getLineStyles();
    const auto &lineWidths = m_occtGeometry->getLineWidths();
    
    for (size_t i = 0; i < shapes.size(); ++i) {
        m_quarterViewer->addShape(shapes[i], colors[i], lineStyles[i], lineWidths[i]);
    }
    
    // Render view
    m_quarterViewer->render();
}

// Open STEP file slot function
void MainWindow::on_actionOpenSTEP_triggered()
{
    // Open file selection dialog
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        tr("Open STEP File"), 
        "", 
        tr("STEP Files (*.step *.stp)")
    );
    
    if (filePath.isEmpty()) {
        return; // User canceled selection
    }
    
    try {
        // Read STEP file using OCCT
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toStdString().c_str());
        
        if (status != IFSelect_RetDone) {
            qDebug() << "Failed to read STEP file:" << filePath;
            QMessageBox::warning(this, tr("Error"), tr("Failed to read STEP file"));
            return;
        }
        
        // Transfer all root shapes
        reader.TransferRoots();
        
        // Get number of shapes read
        int shapeCount = reader.NbRootsForTransfer();
        qDebug() << "Found" << shapeCount << "shapes in STEP file";
        
        // Clear all current shapes
        m_quarterViewer->clearAllShapes();
        
        // Iterate through all shapes and add to view
        for (int i = 1; i <= shapeCount; ++i) {
            TopoDS_Shape shape = reader.Shape(i);
            if (!shape.IsNull()) {
                // Use default color and style
                Quantity_Color color(Quantity_NOC_BLUE);
                m_quarterViewer->addShape(shape, color, 0, 1);
                qDebug() << "Added shape" << i << "to scene";
            }
        }
        
        // Render view
        m_quarterViewer->render();
        
        QMessageBox::information(this, tr("Success"), tr("STEP file read successfully"));
        
    } catch (const Standard_Failure &e) {
        qDebug() << "OCCT exception when reading STEP file:" << e.GetMessageString();
        QMessageBox::warning(this, tr("Error"), tr("OCCT exception occurred while reading STEP file"));
    } catch (...) {
        qDebug() << "Unknown exception when reading STEP file";
        QMessageBox::warning(this, tr("Error"), tr("Unknown exception occurred while reading STEP file"));
    }
}