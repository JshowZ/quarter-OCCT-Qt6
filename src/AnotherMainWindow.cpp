#include "AnotherMainWindow.h"
#include "OccWidget.h"
#include "STEPLoader.h"
#include "MeshabilitySeparator.h"
#include "CurveExtractor.h"
#include "ShapeStatistics.h"

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