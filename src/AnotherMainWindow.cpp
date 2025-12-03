#include "AnotherMainWindow.h"
#include "OccWidget.h"
#include "STEPLoader.h"

#include <QtConcurrent>
#include <QThreadPool>

// OCCT STL export headers
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Message_ProgressRange.hxx>

AnotherMainWindow::AnotherMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_occWidget(nullptr)
    , m_stepLoader(nullptr)
{
    setupUI();
    setupConnections();
}

AnotherMainWindow::~AnotherMainWindow()
{
    delete m_stepLoader;
}

void AnotherMainWindow::setupUI()
{
    setWindowTitle("QCCT STEP reader");
    setGeometry(100, 100, 1200, 800);

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);

    m_controlLayout = new QHBoxLayout();

    // Remove buttons, keep only info label and progress bar
    m_infoLabel = new QLabel("open STEP", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);

    m_controlLayout->addWidget(m_infoLabel);
    m_controlLayout->addWidget(m_progressBar);
    m_controlLayout->addStretch();

    m_mainLayout->addLayout(m_controlLayout);

    // Create menu bar and menus
    m_menuBar = new QMenuBar(this);
    setMenuBar(m_menuBar);
    
    // File menu
    m_fileMenu = m_menuBar->addMenu("File");
    m_openAction = m_fileMenu->addAction("Open STEP File");
    m_openAction->setShortcut(tr("Ctrl+O"));
    
    // Add export STL action
    m_exportSTLAction = m_fileMenu->addAction("Export STL");
    m_exportSTLAction->setShortcut(tr("Ctrl+E"));
    
    // View menu
    m_viewMenu = m_menuBar->addMenu("View");
    m_zoomAllAction = m_viewMenu->addAction("Zoom All");
    m_zoomAllAction->setShortcut(tr("Ctrl+A"));

    m_occWidget = new OccWidget(this);
    m_mainLayout->addWidget(m_occWidget);

    m_stepLoader = new STEPLoader(this);
    m_mainLayout->setStretch(1, 1);
}

void AnotherMainWindow::setupConnections()
{
    // Connect menu actions instead of buttons
    connect(m_openAction, &QAction::triggered, this, &AnotherMainWindow::openSTEPFile);
    connect(m_exportSTLAction, &QAction::triggered, this, &AnotherMainWindow::exportSTLFile);
    connect(m_zoomAllAction, &QAction::triggered, this, &AnotherMainWindow::zoomAll);
    connect(m_stepLoader, &STEPLoader::fileLoaded, this, &AnotherMainWindow::onFileLoaded);
}

void AnotherMainWindow::openSTEPFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "SELECT STEP FILE",
        "",
        "STEP Files (*.step *.stp)"
    );

    if (!filePath.isEmpty()) {
        m_progressBar->setVisible(true);
        m_infoLabel->setText("loading STEPfile...");

		// SINGLE THREAD BLOCKING LOAD
        /*QtConcurrent::run([this, filePath]() {
            bool success = m_stepLoader->loadSTEPFile(filePath);
            QString message = success ? "load success" : "load failed";
            QMetaObject::invokeMethod(this, [this, success, message]() {
                onFileLoaded(success, message);
                }, Qt::QueuedConnection);
            });*/

        Q_UNUSED(QtConcurrent::run([this, filePath]() {
            bool success = m_stepLoader->loadSTEPFile(filePath);
            QString message = success ? "load success" : "load failed";
            QMetaObject::invokeMethod(this, [this, success, message]() {
                onFileLoaded(success, message);
                }, Qt::QueuedConnection);
            }));
    }
}

void AnotherMainWindow::zoomAll()
{
    if (m_occWidget) {
        m_occWidget->fitAll();
    }
}

void AnotherMainWindow::onFileLoaded(bool success, const QString& message)
{
    m_progressBar->setVisible(false);
    m_infoLabel->setText(message);

    if (success) {
		// LOAD SHAPES INTO OCC WIDGET
        const auto& shapes = m_stepLoader->getShapes();
        for (const auto& shape : shapes) {
            m_occWidget->displayShape(shape);
        }
        m_occWidget->fitAll();
    }
    else {
        QMessageBox::critical(this, "ERROE", "CAN NOT LOAD STEP FILE");
    }
}

void AnotherMainWindow::exportSTLFile()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Export STL", "No shapes loaded to export.");
        return;
    }
    
    // Show file save dialog
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export STL File",
        "",
        "STL Files (*.stl)"
    );
    
    if (!filePath.isEmpty()) {
        m_progressBar->setVisible(true);
        m_infoLabel->setText("Exporting STL file...");
        
        // Export in a separate thread to avoid UI blocking
        QThreadPool::globalInstance()->start([this, filePath, shapes]() {
            bool success = true;
            QString message = "Export successful";
            
            try {
                // Create STL writer
                StlAPI_Writer writer;
                writer.ASCIIMode() = false; // Use binary format
                
                // For each shape, create mesh and export
                for (size_t i = 0; i < shapes.size(); ++i) {
                    const TopoDS_Shape& shape = shapes[i];
                    
                    // Create mesh for the shape
                    BRepMesh_IncrementalMesh meshBuilder(shape, 0.00001); // 0.1mm mesh tolerance
                    meshBuilder.Perform();
                    
                    if (!meshBuilder.IsDone()) {
                        success = false;
                        message = "Mesh generation failed";
                        break;
                    }
                    
                    // Export the shape
                    if (shapes.size() == 1) {
                        // Single shape, export directly
                        writer.Write(shape, filePath.toStdString().c_str());
                    } else {
                        // Multiple shapes, export each to separate file
                        QString shapeFilePath = filePath;
                        shapeFilePath.replace(".stl", QString("_shape%1.stl").arg(i + 1));
                        writer.Write(shape, shapeFilePath.toStdString().c_str());
                    }
                }
            } catch (const Standard_Failure& e) {
                success = false;
                message = QString("Export failed: %1").arg(e.GetMessageString());
            } catch (...) {
                success = false;
                message = "Export failed: Unknown error";
            }
            
            // Update UI in main thread
            QMetaObject::invokeMethod(this, [this, success, message]() {
                m_progressBar->setVisible(false);
                m_infoLabel->setText(message);
                
                if (success) {
                    QMessageBox::information(this, "Export STL", "STL file exported successfully.");
                } else {
                    QMessageBox::critical(this, "Export STL", message);
                }
            }, Qt::QueuedConnection);
        });
    }
}