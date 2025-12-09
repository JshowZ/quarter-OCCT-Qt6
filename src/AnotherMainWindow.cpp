#include "AnotherMainWindow.h"
#include "OccWidget.h"
#include "STEPLoader.h"
#include "MeshabilitySeparator.h"
#include "CurveExtractor.h"
#include "ShapeStatistics.h"
#include "MeshRemover.h"
#include "STLExportDiagnoser.h"
#include "STLMultiLevelExporter.h"
#include "CompoundCurveExtractor.h"
#include "TopologyExplorer.h"
#include "importCurveToFile.h"

#include <QtConcurrent>
#include <QThreadPool>

// OCCT STL export headers
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Message_ProgressRange.hxx>

// OCCT shape analysis headers
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Solid.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>

AnotherMainWindow::AnotherMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_occWidget(nullptr)
    , m_stepLoader(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_infoLabel(nullptr)
    , m_progressBar(nullptr)
    , m_menuBar(nullptr)
    , m_fileMenu(nullptr)
    , m_viewMenu(nullptr)
    , m_analysisMenu(nullptr)
    , m_openAction(nullptr)
    , m_zoomAllAction(nullptr)
    , m_exportSTLAction(nullptr)
    , m_loadStepAction(nullptr)
    , m_meshAbilityAnalysisAction(nullptr)
    , m_curveAnalysisAction(nullptr)
    , m_shapeStatisticsAction(nullptr)
    , m_saveEdgesToBREPAction(nullptr)
    , m_meshRemovalAction(nullptr)
    , m_stlExportDiagnosisAction(nullptr)
    , m_stlMultiLevelExportAction(nullptr)
    , m_compoundCurveExtractAction(nullptr)
    , m_topologyExplorationAction(nullptr)
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
    
    // Add load STEP action
    m_loadStepAction = m_fileMenu->addAction("Load STEP with Statistics");
    m_loadStepAction->setShortcut(tr("Ctrl+L"));
    
    // Add export STL action
    m_exportSTLAction = m_fileMenu->addAction("Export STL");
    m_exportSTLAction->setShortcut(tr("Ctrl+E"));
    
    // Add Meshability Analysis action
    m_meshAbilityAnalysisAction = m_fileMenu->addAction("Meshability Analysis");
    m_meshAbilityAnalysisAction->setShortcut(tr("Ctrl+M"));
    
    // View menu
    m_viewMenu = m_menuBar->addMenu("View");
    m_zoomAllAction = m_viewMenu->addAction("Zoom All");
    m_zoomAllAction->setShortcut(tr("Ctrl+A"));
    
    // Analysis menu
    m_analysisMenu = m_menuBar->addMenu("Analysis");
    m_curveAnalysisAction = m_analysisMenu->addAction("Curve Analysis");
    m_curveAnalysisAction->setShortcut(tr("Ctrl+K"));
    m_shapeStatisticsAction = m_analysisMenu->addAction("Shape Statistics");
    m_shapeStatisticsAction->setShortcut(tr("Ctrl+S"));
    m_saveEdgesToBREPAction = m_analysisMenu->addAction("Save Edges to BREP");
    m_saveEdgesToBREPAction->setShortcut(tr("Ctrl+B"));
    m_meshRemovalAction = m_analysisMenu->addAction("Remove Meshable Parts");
    m_meshRemovalAction->setShortcut(tr("Ctrl+M"));
    
    // Add STL Export Diagnosis action
    m_stlExportDiagnosisAction = m_analysisMenu->addAction("STL Export Diagnosis");
    m_stlExportDiagnosisAction->setShortcut(tr("Ctrl+D"));
    
    // Add STL Multi-Level Export action
    m_stlMultiLevelExportAction = m_analysisMenu->addAction("STL Multi-Level Export");
    m_stlMultiLevelExportAction->setShortcut(tr("Ctrl+X"));
    
    // Add Compound Curve Extractor action
    m_compoundCurveExtractAction = m_analysisMenu->addAction("Extract Compound Curves");
    m_compoundCurveExtractAction->setShortcut(tr("Ctrl+U"));
    
    // Add Topology Exploration action
    m_topologyExplorationAction = m_analysisMenu->addAction("Topology Exploration");
    m_topologyExplorationAction->setShortcut(tr("Ctrl+T"));
    
    // Add Import Curve to File action
    m_importCurveToFileAction = m_analysisMenu->addAction("Import Curve to File");
    m_importCurveToFileAction->setShortcut(tr("Ctrl+I"));
    
    // Add Save All Curves to Single BREP action
    m_saveAllCurvesToSingleBREPAction = m_analysisMenu->addAction("Save All Curves to Single BREP");
    m_saveAllCurvesToSingleBREPAction->setShortcut(tr("Ctrl+A"));
    
    // Add Save Curves as Points action
    m_saveCurvesAsPointsAction = m_analysisMenu->addAction("Save Curves as Points");
    m_saveCurvesAsPointsAction->setShortcut(tr("Ctrl+P"));

    m_occWidget = new OccWidget(this);
    m_mainLayout->addWidget(m_occWidget);

    m_stepLoader = new STEPLoader(this);
    m_mainLayout->setStretch(1, 1);
}

void AnotherMainWindow::setupConnections()
{
    // Connect menu actions instead of buttons
    connect(m_openAction, &QAction::triggered, this, &AnotherMainWindow::openSTEPFile);
    connect(m_loadStepAction, &QAction::triggered, this, &AnotherMainWindow::loadStepWithStatistics);
    connect(m_exportSTLAction, &QAction::triggered, this, &AnotherMainWindow::exportSTLFile);
    connect(m_meshAbilityAnalysisAction, &QAction::triggered, this, &AnotherMainWindow::performMeshAbilityAnalysis);
    connect(m_zoomAllAction, &QAction::triggered, this, &AnotherMainWindow::zoomAll);
    connect(m_curveAnalysisAction, &QAction::triggered, this, &AnotherMainWindow::performCurveAnalysis);
    connect(m_shapeStatisticsAction, &QAction::triggered, this, &AnotherMainWindow::performShapeStatistics);
    connect(m_saveEdgesToBREPAction, &QAction::triggered, this, &AnotherMainWindow::saveEdgesToBREPFile);
    connect(m_meshRemovalAction, &QAction::triggered, this, &AnotherMainWindow::performMeshRemoval);
    connect(m_stlExportDiagnosisAction, &QAction::triggered, this, &AnotherMainWindow::performSTLExportDiagnosis);
    connect(m_stlMultiLevelExportAction, &QAction::triggered, this, &AnotherMainWindow::performSTLMultiLevelExport);
    connect(m_compoundCurveExtractAction, &QAction::triggered, this, &AnotherMainWindow::extractCompoundCurves);
    connect(m_topologyExplorationAction, &QAction::triggered, this, &AnotherMainWindow::performTopologyExploration);
    connect(m_importCurveToFileAction, &QAction::triggered, this, &AnotherMainWindow::performCurveExportToFiles);
    connect(m_saveAllCurvesToSingleBREPAction, &QAction::triggered, this, &AnotherMainWindow::saveAllCurvesToSingleBREP);
    connect(m_saveCurvesAsPointsAction, &QAction::triggered, this, &AnotherMainWindow::saveCurvesAsPoints);
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

        #ifdef ENABLE_CONSOLE_OUTPUT
        printf("Opening STEP file: %s\n", filePath.toLocal8Bit().constData());
        #endif

        // Load in a separate thread to avoid UI blocking
        QThreadPool::globalInstance()->start([this, filePath]() {
            bool success = m_stepLoader->loadSTEPFile(filePath);
            QString message = success ? "load success" : "load failed";
            
            #ifdef ENABLE_CONSOLE_OUTPUT
            printf("STEP file load %s\n", success ? "successful" : "failed");
            #endif
            
            QMetaObject::invokeMethod(this, [this, success, message]() {
                onFileLoaded(success, message);
                }, Qt::QueuedConnection);
        });
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

void AnotherMainWindow::performMeshAbilityAnalysis()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Meshability Analysis", "No shapes loaded to analyze. Please open a STEP file first.");
        return;
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Performing meshability analysis...");

    // Perform analysis in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes]() {
        int totalShapes = shapes.size();
        int meshableCount = 0;
        int nonMeshableCount = 0;
        std::stringstream overallReport;
        overallReport << "Meshability Analysis Report" << std::endl;
        overallReport << "================================" << std::endl;
        overallReport << "Total shapes: " << totalShapes << std::endl;
        overallReport << std::endl;

        for (int i = 0; i < shapes.size(); ++i) {
            const auto& shape = shapes[i];
            std::stringstream shapeReport;
            shapeReport << "Shape " << (i + 1) << "/" << totalShapes << std::endl;
            shapeReport << "--------------------------------" << std::endl;

            try {
                // Create and configure separator
                MeshabilitySeparator separator;
                separator.SetDeflection(0.0000001);
                separator.SetAngle(0.5);
                separator.SetTryFixBeforeMeshing(true);
                separator.EnableCaching(true);

                // Perform analysis
                TopoDS_Compound meshableParts, nonMeshableParts;
                bool success = separator.Separate(shape, meshableParts, nonMeshableParts);

                if (success) {
                    // Get detailed report
                    std::string analysisReport = separator.GetAnalysisReport();
                    shapeReport << analysisReport << std::endl;

                    // Count results
                    const auto& meshableInfo = separator.GetMeshableInfo();
                    const auto& nonMeshableInfo = separator.GetNonMeshableInfo();
                    meshableCount += meshableInfo.size();
                    nonMeshableCount += nonMeshableInfo.size();

                    // Save results to files
                    std::string shapePrefix = "shape_" + std::to_string(i + 1) + "_";
                    if (!meshableParts.IsNull()) {
                        StlAPI_Writer writer;
                        std::string stlPath = shapePrefix + "meshable.stl";
                        if (writer.Write(meshableParts, stlPath.c_str())) {
                            shapeReport << "Meshable parts saved to: " << stlPath << std::endl;
                        }
                    }

                    if (!nonMeshableParts.IsNull()) {
                        std::string brepPath = shapePrefix + "non_meshable.brep";
                        if (BRepTools::Write(nonMeshableParts, brepPath.c_str())) {
                            shapeReport << "Non-meshable parts saved to: " << brepPath << std::endl;
                        }
                    }
                } else {
                    shapeReport << "Analysis failed for this shape." << std::endl;
                }
            } catch (const std::exception& e) {
                shapeReport << "Exception during analysis: " << e.what() << std::endl;
            } catch (...) {
                shapeReport << "Unknown exception during analysis." << std::endl;
            }

            shapeReport << "================================" << std::endl;
            overallReport << shapeReport.str() << std::endl;
        }

        // Add summary
        overallReport << "Overall Summary" << std::endl;
        overallReport << "================================" << std::endl;
        overallReport << "Total meshable parts: " << meshableCount << std::endl;
        overallReport << "Total non-meshable parts: " << nonMeshableCount << std::endl;
        overallReport << "Meshability rate: " << std::fixed << std::setprecision(1)
                     << (totalShapes > 0 ? (100.0 * meshableCount / (meshableCount + nonMeshableCount)) : 0.0)
                     << "%" << std::endl;

        #ifdef ENABLE_CONSOLE_OUTPUT
        std::cout << overallReport.str() << std::endl;
        #endif

        // Save report to file
        std::ofstream reportFile("meshability_analysis_report.txt");
        if (reportFile.is_open()) {
            reportFile << overallReport.str();
            reportFile.close();
        }

        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText("Meshability analysis completed");
            QMessageBox::information(this, "Meshability Analysis", 
                "Meshability analysis completed. Check the console output and generated files for detailed results.");
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::performSTLExportDiagnosis()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "STL Export Diagnosis", "No shapes loaded to analyze. Please open a STEP file first.");
        return;
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Performing STL export diagnosis...");

    // Perform diagnosis in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes]() {
        // Create STLExportDiagnoser
        STLExportDiagnoser diagnoser;
        diagnoser.setDeflection(0.01);
        
        // Diagnose all shapes
        std::vector<MeshDiagnosis> allDiagnoses;
        for (const auto& shape : shapes) {
            std::vector<MeshDiagnosis> shapeDiagnoses = diagnoser.diagnoseShape(shape);
            allDiagnoses.insert(allDiagnoses.end(), shapeDiagnoses.begin(), shapeDiagnoses.end());
        }
        
        // Generate detailed report
        std::string report = diagnoser.generateReport(allDiagnoses);
        
        // Get exportable and non-exportable shapes
        std::vector<TopoDS_Shape> exportableShapes = diagnoser.getExportableShapes(allDiagnoses);
        std::vector<TopoDS_Shape> nonExportableShapes = diagnoser.getNonExportableShapes(allDiagnoses);
        
        // Save report to file
        std::ofstream reportFile("stl_export_diagnosis_report.txt");
        if (reportFile.is_open()) {
            reportFile << report;
            reportFile.close();
        }
        
        // Export exportable shapes to STL if any
        bool exportSuccess = false;
        if (!exportableShapes.empty()) {
            exportSuccess = diagnoser.exportToSTL(exportableShapes, "exportable_shapes.stl");
        }
        
        // Save non-exportable shapes to BREP if any
        bool brepSaveSuccess = false;
        if (!nonExportableShapes.empty()) {
            brepSaveSuccess = diagnoser.saveNonMeshablePartsToBREP(allDiagnoses, "non_exportable_shapes.brep");
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, allDiagnoses, exportableShapes, nonExportableShapes, exportSuccess, brepSaveSuccess]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText("STL export diagnosis completed");
            
            // Display results in message box
            QString statsMessage = QString(
                "STL Export Diagnosis Results:\n"
                "Total entities: %1\n"
                "Exportable entities: %2\n"
                "Non-exportable entities: %3\n"
                "Exportable faces: %4\n"
                "Triangulated faces: %5\n"
                "\nExportable shapes %6 to exportable_shapes.stl\n"
                "Non-exportable shapes %7 to non_exportable_shapes.brep"
            );
            
            // Calculate total faces and triangulated faces
            int totalFaces = 0;
            int totalTriangulatedFaces = 0;
            for (const auto& diag : allDiagnoses) {
                totalFaces += diag.faceCount;
                totalTriangulatedFaces += diag.triangulatedFaceCount;
            }
            
            statsMessage = statsMessage
                .arg(allDiagnoses.size())
                .arg(exportableShapes.size())
                .arg(nonExportableShapes.size())
                .arg(totalFaces)
                .arg(totalTriangulatedFaces)
                .arg(exportSuccess ? "successfully exported" : "failed to export")
                .arg(brepSaveSuccess ? "successfully saved" : "failed to save");
            
            QMessageBox::information(this, "STL Export Diagnosis", statsMessage);
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::performSTLMultiLevelExport()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "STL Multi-Level Export", "No shapes loaded to export. Please open a STEP file first.");
        return;
    }

    // Open file dialog to choose save location
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save STL File",
        "",
        "STL Files (*.stl)"
    );
    
    if (filePath.isEmpty()) {
        return; // User canceled
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Performing STL multi-level export...");

    // Perform export in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes, filePath]() {
        bool exportSuccess = false;
        std::string exportMessage;
        
        try {
            // Create STLMultiLevelExporter
            STLMultiLevelExporter exporter;
            exporter.setDeflection(0.01);
            
            // Combine all shapes into a single compound shape for export
            BRep_Builder builder;
            TopoDS_Compound combinedShape;
            builder.MakeCompound(combinedShape);
            
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // Perform multi-level export
            exportSuccess = exporter.exportToSTL(combinedShape, filePath.toStdString());
            
            // Generate report
            std::string report = exporter.generateReport();
            
            // Save report to file
            std::string reportFilePath = filePath.toStdString() + "_export_report.txt";
            std::ofstream reportFile(reportFilePath);
            if (reportFile.is_open()) {
                reportFile << report;
                reportFile.close();
            }
            
            exportMessage = exportSuccess ? "STL export completed successfully." : "STL export failed.";
            exportMessage += "\nReport saved to: " + reportFilePath;
            
        } catch (const Standard_Failure& e) {
            exportSuccess = false;
            exportMessage = "STL export failed: " + std::string(e.GetMessageString());
        } catch (const std::exception& e) {
            exportSuccess = false;
            exportMessage = "STL export failed: " + std::string(e.what());
        } catch (...) {
            exportSuccess = false;
            exportMessage = "STL export failed: Unknown error.";
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, exportSuccess, exportMessage, filePath]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText(exportSuccess ? "STL multi-level export completed" : "STL multi-level export failed");
            
            if (exportSuccess) {
                QMessageBox::information(this, "STL Multi-Level Export", 
                    QString::fromStdString(exportMessage) + "\n\nFile saved to: " + filePath);
            } else {
                QMessageBox::critical(this, "STL Multi-Level Export", 
                    QString::fromStdString(exportMessage));
            }
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::extractCompoundCurves()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Extract Compound Curves", "No shapes loaded to extract curves from. Please open a STEP file first.");
        return;
    }

    // Open file dialog to choose save location
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Curves to BREP",
        "",
        "BREP Files (*.brep)"
    );
    
    if (filePath.isEmpty()) {
        return; // User canceled
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Extracting curves from compound shapes...");

    // Perform extraction in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes, filePath]() {
        bool extractionSuccess = false;
        std::string extractionMessage;
        int curveCount = 0;
        
        try {
            // Create CompoundCurveExtractor
            CompoundCurveExtractor extractor;
            
            // Combine all shapes into a single compound shape for extraction
            BRep_Builder builder;
            TopoDS_Compound combinedShape;
            builder.MakeCompound(combinedShape);
            
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // Extract curves from compound shapes
            extractionSuccess = extractor.extractCurvesFromCompound(combinedShape);
            curveCount = extractor.getCurveCount();
            
            if (extractionSuccess && curveCount > 0) {
                // Save curves to BREP file
                if (extractor.saveCurvesToBREP(filePath.toStdString())) {
                    extractionMessage = "Successfully extracted " + std::to_string(curveCount) + " curves from compound shapes.";
                } else {
                    extractionSuccess = false;
                    extractionMessage = "Failed to save extracted curves to BREP file.";
                }
            } else {
                extractionSuccess = false;
                extractionMessage = "No curves could be extracted from the compound shapes.";
            }
            
        } catch (const Standard_Failure& e) {
            extractionSuccess = false;
            extractionMessage = "Curve extraction failed: " + std::string(e.GetMessageString());
        } catch (const std::exception& e) {
            extractionSuccess = false;
            extractionMessage = "Curve extraction failed: " + std::string(e.what());
        } catch (...) {
            extractionSuccess = false;
            extractionMessage = "Curve extraction failed: Unknown error.";
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, extractionSuccess, extractionMessage, curveCount, filePath]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText(extractionSuccess ? "Curve extraction completed" : "Curve extraction failed");
            
            if (extractionSuccess) {
                QMessageBox::information(this, "Extract Compound Curves", 
                    QString::fromStdString(extractionMessage) + "\n\nFile saved to: " + filePath);
            } else {
                QMessageBox::critical(this, "Extract Compound Curves", 
                    QString::fromStdString(extractionMessage));
            }
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::performTopologyExploration()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Topology Exploration", "No shapes loaded to explore. Please open a STEP file first.");
        return;
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Performing topology exploration...");

    // Perform exploration in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes]() {
        std::string explorationMessage;
        std::string topologyStats;
        std::string curveStats;
        int uniqueCurvesCount = 0;
        int topologyCount = 0;
        
        try {
            // Create TopologyExplorer
            TopologyExplorer explorer;
            
            // Combine all shapes into a single compound shape for exploration
            BRep_Builder builder;
            TopoDS_Compound combinedShape;
            builder.MakeCompound(combinedShape);
            
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // Perform topology exploration
            if (explorer.Explore(combinedShape)) {
                // Generate statistics
                TCollection_ExtendedString topoStats, crvStats;
                explorer.GetTopologyStatistics(topoStats);
                explorer.GetCurveStatistics(crvStats);
                
                // Convert TCollection_ExtendedString to std::string
                Standard_Integer topoLen = topoStats.LengthOfCString();
                char* topoBuffer = new char[topoLen + 1];
                topoStats.ToUTF8CString(reinterpret_cast<Standard_PCharacter&>(topoBuffer));
                topologyStats = topoBuffer;
                delete[] topoBuffer;
                
                Standard_Integer crvLen = crvStats.LengthOfCString();
                char* crvBuffer = new char[crvLen + 1];
                crvStats.ToUTF8CString(reinterpret_cast<Standard_PCharacter&>(crvBuffer));
                curveStats = crvBuffer;
                delete[] crvBuffer;
                
                uniqueCurvesCount = explorer.GetUniqueCurves3D().Extent();
                topologyCount = 0;
                for (int i = TopAbs_COMPOUND; i < TopAbs_SHAPE; i++) {
                    topologyCount += explorer.GetShapesByType((TopAbs_ShapeEnum)i).Extent();
                }
                
                explorationMessage = "Topology exploration completed successfully.\n";
                explorationMessage += "\nTopology Statistics:\n" + topologyStats;
                explorationMessage += "\n\nCurve Statistics:\n" + curveStats;
            } else {
                explorationMessage = "Topology exploration failed.\nNo valid shapes could be explored.";
            }
            
        } catch (const Standard_Failure& e) {
            explorationMessage = "Topology exploration failed: " + std::string(e.GetMessageString());
        } catch (const std::exception& e) {
            explorationMessage = "Topology exploration failed: " + std::string(e.what());
        } catch (...) {
            explorationMessage = "Topology exploration failed: Unknown error.";
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, explorationMessage, uniqueCurvesCount, shapes]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText("Topology exploration completed");
            
            // Show exploration results in a dialog
            QMessageBox::information(this, "Topology Exploration Results", 
                QString::fromStdString(explorationMessage));
            
            // Ask user if they want to export the curves
            if (uniqueCurvesCount > 0) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this, "Export Curves", 
                    QString::fromStdString("Do you want to export the " + std::to_string(uniqueCurvesCount) + " unique curves to STEP file?"),
                    QMessageBox::Yes | QMessageBox::No);
                
                if (reply == QMessageBox::Yes) {
                    // Open file dialog to choose save location
                    QString filePath = QFileDialog::getSaveFileName(
                        this,
                        "Export Curves to STEP",
                        "",
                        "STEP Files (*.step *.stp)"
                    );
                    
                    if (!filePath.isEmpty()) {
                        // Perform export in a new thread
                        QThreadPool::globalInstance()->start([this, filePath, shapes]() {
                            try {
                                // Create TopologyExplorer again for export
                                TopologyExplorer explorer;
                                
                                // Combine shapes again
                                BRep_Builder builder;
                                TopoDS_Compound combinedShape;
                                builder.MakeCompound(combinedShape);
                                
                                for (const auto& shape : shapes) {
                                    builder.Add(combinedShape, shape);
                                }
                                
                                explorer.Explore(combinedShape);
                                
                                // Export curves to STEP
                                TCollection_ExtendedString extendedFilePath(filePath.toStdString().c_str());
                                if (explorer.ExportCurvesToSTEP(extendedFilePath)) {
                                    QMetaObject::invokeMethod(this, [this, filePath]() {
                                        QMessageBox::information(this, "Export Curves", 
                                            "Curves exported successfully to:\n" + filePath);
                                    }, Qt::QueuedConnection);
                                } else {
                                    QMetaObject::invokeMethod(this, [this]() {
                                        QMessageBox::critical(this, "Export Curves", 
                                            "Failed to export curves to STEP file.");
                                    }, Qt::QueuedConnection);
                                }
                                
                            } catch (...) {
                                QMetaObject::invokeMethod(this, [this]() {
                                    QMessageBox::critical(this, "Export Curves", 
                                        "An error occurred during curve export.");
                                }, Qt::QueuedConnection);
                            }
                        });
                    }
                }
            }
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::loadStepWithStatistics()
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

#ifdef ENABLE_CONSOLE_OUTPUT
		printf("Loading STEP file with statistics: %s\n", filePath.toLocal8Bit().constData());
#endif

		// Load and analyze in a separate thread to avoid UI blocking
		QThreadPool::globalInstance()->start([this, filePath]() {
			// Load the STEP file first
			bool success = m_stepLoader->loadSTEPFile(filePath);
			QString message = success ? "load success" : "load failed";

			int lineCount = 0;
			int curveCount = 0;
			int ellipseCount = 0;
			int solidCount = 0;
			int bezierCount = 0;
			int bsplineCount = 0;
			int trimmedCount = 0;
			int offsetCount = 0;

			if (success) {
				const auto& shapes = m_stepLoader->getShapes();

#ifdef ENABLE_CONSOLE_OUTPUT
				printf("Loaded %d shapes from STEP file\n", (int)shapes.size());
#endif

				// Analyze each shape
				for (const auto& shape : shapes) {
					// Count solids
					if (shape.ShapeType() == TopAbs_SOLID) {
					    solidCount++;
					}
					
					// Explore all edges to count lines, curves, and ellipses
					TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
					while (edgeExplorer.More()) {
					    const TopoDS_Edge& edge = TopoDS::Edge(edgeExplorer.Current());
					    
					    // Get the curve geometry
					    Standard_Real first, last;
					    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
					    
					    if (!curve.IsNull()) {
					        // Check the type of curve
					        if (curve->DynamicType() == STANDARD_TYPE(Geom_Line)) {
					            lineCount++;
					        } else if (curve->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
					            curveCount++;
					        } else if (curve->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
					            ellipseCount++;
					        }
					        else if (curve->DynamicType() == STANDARD_TYPE(Geom_BezierCurve)) {
					            bezierCount++;
					        }
					        else if (curve->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
					            bsplineCount++;
					        }
					        else if (curve->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) {
					            trimmedCount++;
					        }
					        else if (curve->DynamicType() == STANDARD_TYPE(Geom_OffsetCurve)) {
					            offsetCount++;
					        }
					        else {
					            // Other curves
					            curveCount++;
					        }
					    }
					    
					    edgeExplorer.Next();
					}
				}

#ifdef ENABLE_CONSOLE_OUTPUT
				printf("Geometry Statistics for %s:\n", filePath.toLocal8Bit().constData());
				printf("  Lines: %d\n", lineCount);
				printf("  Curves: %d\n", curveCount);
				printf("  Ellipses: %d\n", ellipseCount);
				printf("  Solids: %d\n", solidCount);
				printf("  BezierCurve: %d\n", bezierCount);
				printf("  BSplineCurve: %d\n", bsplineCount);
				printf("  TrimmedCurve: %d\n", trimmedCount);
				printf("  OffsetCurve: %d\n", offsetCount);
#endif
			}
			else {
#ifdef ENABLE_CONSOLE_OUTPUT
				printf("Failed to load STEP file: %s\n", filePath.toLocal8Bit().constData());
#endif
			}

            //Update UI in main thread
			QMetaObject::invokeMethod(this, [this, success, message, lineCount, curveCount, ellipseCount, solidCount, bezierCount, bsplineCount, trimmedCount, offsetCount]() {
			    m_progressBar->setVisible(false);
			    m_infoLabel->setText(message);
			    
			    if (success) {
			        // Display the statistics
			        QString statsMessage = QString(
			            "STEP File Analysis:\n"
			            "Lines: %1\n"
			            "Curves: %2\n"
			            "Ellipses: %3\n"
			            "Solids: %4\n"
			            "BezierCurve:%5\n"
			            "BSplineCurve:%6\n"
			            "TrimmedCurve:%7\n"
			            "Geom_OffsetCurve:%8"
			        ).arg(lineCount).arg(curveCount).arg(ellipseCount).arg(solidCount).arg(bezierCount).arg(bsplineCount).arg(trimmedCount).arg(offsetCount);
			        
			        QMessageBox::information(this, "STEP Geometry Statistics", statsMessage);
			        
			        // Load shapes into OCC widget
			        //const auto& shapes = m_stepLoader->getShapes();
			        //for (const auto& shape : shapes) {
			        //    m_occWidget->displayShape(shape);
			        //}
			        //m_occWidget->fitAll();
			    } else {
			        QMessageBox::critical(this, "ERROR", "CAN NOT LOAD STEP FILE");
			    }
			}, Qt::QueuedConnection);
			});
	}
}

void AnotherMainWindow::performCurveAnalysis()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Curve Analysis", "No shapes loaded to analyze. Please open a STEP file first.");
        return;
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Performing curve analysis...");

    // Perform analysis in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes]() {
        int totalEdges = 0;
        int lineCount = 0;
        int circleCount = 0;
        int ellipseCount = 0;
        int parabolaCount = 0;
        int hyperbolaCount = 0;
        int bezierCount = 0;
        int bsplineCount = 0;
        int otherCount = 0;
        
        double totalLength = 0.0;
        
        // Create CurveExtractor
        CurveExtractor extractor;
        
        for (int i = 0; i < shapes.size(); ++i) {
            const auto& shape = shapes[i];
            
            // Extract curve geometry info
            Handle(TopTools_HSequenceOfShape) extractedEdges;
            std::vector<CurveGeometryInfo> curveInfos;
            extractor.extractCurveGeometryInfo(shape, extractedEdges, curveInfos);
            
            totalEdges += curveInfos.size();
            
            // Analyze each curve
            for (const auto& info : curveInfos) {
                if (info.curve.IsNull()) {
                    continue;
                }
                
                // Add to total length
                totalLength += info.length;
                
                // Count curve types
                const Handle(Standard_Type)& curveType = info.curve->DynamicType();
                if (curveType == STANDARD_TYPE(Geom_Line)) {
                    lineCount++;
                } else if (curveType == STANDARD_TYPE(Geom_Circle)) {
                    circleCount++;
                } else if (curveType == STANDARD_TYPE(Geom_Ellipse)) {
                    ellipseCount++;
                } else if (curveType == STANDARD_TYPE(Geom_Parabola)) {
                    parabolaCount++;
                } else if (curveType == STANDARD_TYPE(Geom_Hyperbola)) {
                    hyperbolaCount++;
                } else if (curveType == STANDARD_TYPE(Geom_BezierCurve)) {
                    bezierCount++;
                } else if (curveType == STANDARD_TYPE(Geom_BSplineCurve)) {
                    bsplineCount++;
                } else {
                    otherCount++;
                }
                
                // Print detailed info for each curve (optional)
                #ifdef ENABLE_CONSOLE_OUTPUT
                CurveExtractor::printCurveInfo(info);
                #endif
            }
        }
        
        // Generate report
        std::stringstream report;
        report << "Curve Analysis Report" << std::endl;
        report << "====================" << std::endl;
        report << "Total edges: " << totalEdges << std::endl;
        report << "Total curve length: " << totalLength << std::endl;
        report << std::endl;
        report << "Curve Type Distribution:" << std::endl;
        report << "------------------------" << std::endl;
        report << "Lines: " << lineCount << " (" << (totalEdges > 0 ? (lineCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "Circles: " << circleCount << " (" << (totalEdges > 0 ? (circleCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "Ellipses: " << ellipseCount << " (" << (totalEdges > 0 ? (ellipseCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "Parabolas: " << parabolaCount << " (" << (totalEdges > 0 ? (parabolaCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "Hyperbolas: " << hyperbolaCount << " (" << (totalEdges > 0 ? (hyperbolaCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "Bezier curves: " << bezierCount << " (" << (totalEdges > 0 ? (bezierCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "BSpline curves: " << bsplineCount << " (" << (totalEdges > 0 ? (bsplineCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        report << "Other curves: " << otherCount << " (" << (totalEdges > 0 ? (otherCount * 100.0 / totalEdges) : 0.0) << "%)" << std::endl;
        
        #ifdef ENABLE_CONSOLE_OUTPUT
        std::cout << report.str() << std::endl;
        #endif
        
        // Save report to file
        std::ofstream reportFile("curve_analysis_report.txt");
        if (reportFile.is_open()) {
            reportFile << report.str();
            reportFile.close();
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, totalEdges, lineCount, circleCount, ellipseCount, parabolaCount, hyperbolaCount, bezierCount, bsplineCount, otherCount, totalLength]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText("Curve analysis completed");
            
            // Display results in message box
            QString statsMessage = QString(
                "Curve Analysis Results:\n"
                "Total edges: %1\n"
                "Total curve length: %2\n"
                "\n"
                "Curve Type Distribution:\n"
                "Lines: %3 (%4%)\n"
                "Circles: %5 (%6%)\n"
                "Ellipses: %7 (%8%)\n"
                "Parabolas: %9 (%10%)\n"
                "Hyperbolas: %11 (%12%)\n"
                "Bezier curves: %13 (%14%)\n"
                "BSpline curves: %15 (%16%)\n"
                "Other curves: %17 (%18%)"
            )
            .arg(totalEdges)
            .arg(totalLength, 0, 'f', 3)
            .arg(lineCount).arg(totalEdges > 0 ? (lineCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(circleCount).arg(totalEdges > 0 ? (circleCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(ellipseCount).arg(totalEdges > 0 ? (ellipseCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(parabolaCount).arg(totalEdges > 0 ? (parabolaCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(hyperbolaCount).arg(totalEdges > 0 ? (hyperbolaCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(bezierCount).arg(totalEdges > 0 ? (bezierCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(bsplineCount).arg(totalEdges > 0 ? (bsplineCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1)
            .arg(otherCount).arg(totalEdges > 0 ? (otherCount * 100.0 / totalEdges) : 0.0, 0, 'f', 1);
            
            QMessageBox::information(this, "Curve Analysis", statsMessage);
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::performShapeStatistics()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Shape Statistics", "No shapes loaded to analyze. Please open a STEP file first.");
        return;
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Performing shape statistics...");

    // Perform statistics in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes]() {
        // Create ShapeStatistics
        ShapeStatistics stats;
        
        // Compute statistics for all shapes
        for (const auto& shape : shapes) {
            stats.computeStatistics(shape);
        }
        
        // Generate report
        std::stringstream report;
        report << "Shape Statistics Report" << std::endl;
        report << "========================" << std::endl;
        report << "Vertex count: " << stats.getVertexCount() << std::endl;
        report << "Edge count: " << stats.getEdgeCount() << std::endl;
        report << "Wire count: " << stats.getWireCount() << std::endl;
        report << "Face count: " << stats.getFaceCount() << std::endl;
        report << "Shell count: " << stats.getShellCount() << std::endl;
        report << "Solid count: " << stats.getSolidCount() << std::endl;
        report << "CompSolid count: " << stats.getCompSolidCount() << std::endl;
        report << "Compound count: " << stats.getCompoundCount() << std::endl;
        report << "Total shape count: " << stats.getTotalShapeCount() << std::endl;
        
        #ifdef ENABLE_CONSOLE_OUTPUT
        stats.printStatistics();
        std::cout << report.str() << std::endl;
        #endif
        
        // Save report to file
        std::ofstream reportFile("shape_statistics_report.txt");
        if (reportFile.is_open()) {
            reportFile << report.str();
            reportFile.close();
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, stats]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText("Shape statistics completed");
            
            // Display results in message box
            QString statsMessage = QString(
                "Shape Statistics Results:\n"
                "Vertex count: %1\n"
                "Edge count: %2\n"
                "Wire count: %3\n"
                "Face count: %4\n"
                "Shell count: %5\n"
                "Solid count: %6\n"
                "CompSolid count: %7\n"
                "Compound count: %8\n"
                "Total shape count: %9"
            )
            .arg(stats.getVertexCount())
            .arg(stats.getEdgeCount())
            .arg(stats.getWireCount())
            .arg(stats.getFaceCount())
            .arg(stats.getShellCount())
            .arg(stats.getSolidCount())
            .arg(stats.getCompSolidCount())
            .arg(stats.getCompoundCount())
            .arg(stats.getTotalShapeCount());
            
            QMessageBox::information(this, "Shape Statistics", statsMessage);
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::saveEdgesToBREPFile()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Save Edges to BREP", "No shapes loaded. Please open a STEP file first.");
        return;
    }
    
    // Open file dialog to choose save location
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Edges to BREP",
        "",
        "BREP Files (*.brep)"
    );
    
    if (filePath.isEmpty()) {
        return; // User canceled
    }
    
    m_progressBar->setVisible(true);
    m_infoLabel->setText("Saving edges to BREP file...");
    
    // Save in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes, filePath]() {
        bool success = false;
        QString message;
        
        try {
            // Create ShapeStatistics
            ShapeStatistics stats;
            
            // Compute statistics for all shapes
            for (const auto& shape : shapes) {
                stats.computeStatistics(shape);
            }
            
            // Save edges to BREP file
            success = stats.saveEdgesToBREP(filePath.toStdString());
            message = success ? "Edges saved successfully." : "Failed to save edges.";
        } catch (const std::exception& e) {
            message = QString("Error: %1").arg(e.what());
            success = false;
        } catch (...) {
            message = "Unknown error occurred.";
            success = false;
        }
        
        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, success, message, filePath]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText(message);
            
            if (success) {
                QMessageBox::information(this, "Save Edges to BREP", 
                    QString("Edges saved successfully to:\n%1").arg(filePath));
            } else {
                QMessageBox::critical(this, "Save Edges to BREP", message);
            }
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::performMeshRemoval()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Remove Meshable Parts", "No shapes loaded. Please open a STEP file first.");
        return;
    }
    
    m_progressBar->setVisible(true);
    m_infoLabel->setText("Removing meshable parts...");
    
    // Perform mesh removal in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes]() {
        try {
            // Create MeshRemover object
            MeshRemover remover(0.01, 0.5);
            
            // Process all shapes
            TopoDS_Compound combinedShape;
            BRep_Builder builder;
            builder.MakeCompound(combinedShape);
            
            // 合并所有形状到一个复合形状
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // 移除可三角剖分的部分
            TopoDS_Shape outputShape;
            remover.removeMeshableParts(combinedShape, outputShape);
            
            // 获取统计信息
            int meshableCount = remover.getMeshablePartCount();
            int nonMeshableCount = remover.getNonMeshablePartCount();
            
            // 更新UI在主线程
            QMetaObject::invokeMethod(this, [this, outputShape, meshableCount, nonMeshableCount]() {
                m_progressBar->setVisible(false);
                m_infoLabel->setText("Mesh removal completed");
                
                // 清除当前显示
                m_occWidget->eraseAll();
                
                // 显示剩余的不可三角剖分部分
                if (!outputShape.IsNull()) {
                    m_occWidget->displayShape(outputShape);
                    m_occWidget->fitAll();
                }
                
                // 显示统计信息
                QString statsMessage = QString(
                    "Mesh Removal Results:\n"
                    "Meshable parts removed: %1\n"
                    "Non-meshable parts remaining: %2\n"
                    "Total parts processed: %3"
                ).arg(meshableCount).arg(nonMeshableCount).arg(meshableCount + nonMeshableCount);
                
                QMessageBox::information(this, "Remove Meshable Parts", statsMessage);
            }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            QMetaObject::invokeMethod(this, [this, e]() {
                m_progressBar->setVisible(false);
                m_infoLabel->setText("Mesh removal failed");
                QMessageBox::critical(this, "Remove Meshable Parts", 
                    QString("Error during mesh removal: %1").arg(e.what()));
            }, Qt::QueuedConnection);
        } catch (...) {
            QMetaObject::invokeMethod(this, [this]() {
                m_progressBar->setVisible(false);
                m_infoLabel->setText("Mesh removal failed");
                QMessageBox::critical(this, "Remove Meshable Parts", 
                    "Unknown error during mesh removal");
            }, Qt::QueuedConnection);
        }
    });
}

void AnotherMainWindow::performCurveExportToFiles()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Import Curve to File", "No shapes loaded to process. Please open a STEP file first.");
        return;
    }

    // Ask user to select output directory
    QString outputDir = QFileDialog::getExistingDirectory(
        this,
        "Select Output Directory",
        ".",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (outputDir.isEmpty()) {
        return; // User canceled
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Processing and saving curves...");

    // Perform processing in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes, outputDir]() {
        bool success = false;
        std::string message;
        std::map<std::string, int> curveStats;

        try {
            // Create importCurveToFile instance
            importCurveToFile curveImporter;
            
            // Combine all shapes into a single compound shape
            BRep_Builder builder;
            TopoDS_Compound combinedShape;
            builder.MakeCompound(combinedShape);
            
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // Process and save curves
            success = curveImporter.processAndSaveCurves(combinedShape, outputDir.toStdString());
            
            if (success) {
                // Get curve statistics
                curveStats = curveImporter.getCurveTypeStats();
                message = "Successfully processed and saved curves to BREP files.\n";
                message += "Output directory: " + outputDir.toStdString() + "\n\n";
                message += "Curve Type Statistics:\n";
                
                for (const auto& pair : curveStats) {
                    message += pair.first + ": " + std::to_string(pair.second) + "\n";
                }
            } else {
                message = "Failed to process and save curves.\n";
            }
        } catch (const Standard_Failure& e) {
            message = "OCCT Exception: " + std::string(e.GetMessageString()) + "\n";
        } catch (const std::exception& e) {
            message = "Exception: " + std::string(e.what()) + "\n";
        } catch (...) {
            message = "Unknown error occurred during curve processing.\n";
        }

        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, success, message]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText(success ? "Curve export completed" : "Curve export failed");
            
            QMessageBox::information(this, "Import Curve to File", 
                QString::fromStdString(message));
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::saveAllCurvesToSingleBREP()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Save All Curves to Single BREP", "No shapes loaded to process. Please open a STEP file first.");
        return;
    }

    // Ask user to select output file
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save All Curves to BREP",
        "all_curves.brep",
        "BREP Files (*.brep)"
    );

    if (filePath.isEmpty()) {
        return; // User canceled
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Processing and saving all curves to single BREP...");

    // Perform processing in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes, filePath]() {
        bool success = false;
        std::string message;
        std::map<std::string, int> curveStats;

        try {
            // Create importCurveToFile instance
            importCurveToFile curveImporter;
            
            // Combine all shapes into a single compound shape
            BRep_Builder builder;
            TopoDS_Compound combinedShape;
            builder.MakeCompound(combinedShape);
            
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // Process and save all curves to single BREP file
            success = curveImporter.processAndSaveAllCurvesToSingleBREP(combinedShape, filePath.toStdString());
            
            if (success) {
                // Get curve statistics
                curveStats = curveImporter.getCurveTypeStats();
                message = "Successfully processed and saved all curves to single BREP file.\n";
                message += "Output file: " + filePath.toStdString() + "\n\n";
                message += "Curve Type Statistics:\n";
                
                for (const auto& pair : curveStats) {
                    message += pair.first + ": " + std::to_string(pair.second) + "\n";
                }
            } else {
                message = "Failed to process and save curves.\n";
            }
        } catch (const Standard_Failure& e) {
            message = "OCCT Exception: " + std::string(e.GetMessageString()) + "\n";
        } catch (const std::exception& e) {
            message = "Exception: " + std::string(e.what()) + "\n";
        } catch (...) {
            message = "Unknown error occurred during curve processing.\n";
        }

        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, success, message, filePath]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText(success ? "Curve export completed" : "Curve export failed");
            
            QMessageBox::information(this, "Save All Curves to Single BREP", 
                QString::fromStdString(message));
        }, Qt::QueuedConnection);
    });
}

void AnotherMainWindow::saveCurvesAsPoints()
{
    // Check if there are any shapes loaded
    const auto& shapes = m_stepLoader->getShapes();
    if (shapes.empty()) {
        QMessageBox::information(this, "Save Curves as Points", "No shapes loaded to process. Please open a STEP file first.");
        return;
    }

    // Ask user to select output file
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Curves as Points",
        "curves_points.txt",
        "Text Files (*.txt)"
    );

    if (filePath.isEmpty()) {
        return; // User canceled
    }

    m_progressBar->setVisible(true);
    m_infoLabel->setText("Processing and saving curves as points...");

    // Perform processing in a separate thread to avoid UI blocking
    QThreadPool::globalInstance()->start([this, shapes, filePath]() {
        bool success = false;
        std::string message;
        std::map<std::string, int> curveStats;

        try {
            // Create importCurveToFile instance
            importCurveToFile curveImporter;
            
            // Combine all shapes into a single compound shape
            BRep_Builder builder;
            TopoDS_Compound combinedShape;
            builder.MakeCompound(combinedShape);
            
            for (const auto& shape : shapes) {
                builder.Add(combinedShape, shape);
            }
            
            // Process and save curves as points
            success = curveImporter.processAndSaveCurvesAsPoints(combinedShape, filePath.toStdString());
            
            if (success) {
                // Get curve statistics
                curveStats = curveImporter.getCurveTypeStats();
                message = "Successfully processed and saved curves as points.\n";
                message += "Output file: " + filePath.toStdString() + "\n\n";
                message += "Curve Type Statistics:\n";
                
                for (const auto& pair : curveStats) {
                    message += pair.first + ": " + std::to_string(pair.second) + "\n";
                }
            } else {
                message = "Failed to process and save curves as points.\n";
            }
        } catch (const Standard_Failure& e) {
            message = "OCCT Exception: " + std::string(e.GetMessageString()) + "\n";
        } catch (const std::exception& e) {
            message = "Exception: " + std::string(e.what()) + "\n";
        } catch (...) {
            message = "Unknown error occurred during curve processing.\n";
        }

        // Update UI in main thread
        QMetaObject::invokeMethod(this, [this, success, message, filePath]() {
            m_progressBar->setVisible(false);
            m_infoLabel->setText(success ? "Curve point export completed" : "Curve point export failed");
            
            QMessageBox::information(this, "Save Curves as Points", 
                QString::fromStdString(message));
        }, Qt::QueuedConnection);
    });
}