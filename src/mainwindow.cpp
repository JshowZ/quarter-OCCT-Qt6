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

// OCCT STEP读取相关头文件
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopExp_Explorer.hxx>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_currentMode(OCCT_Point),
    m_isDrawing(false)
{
    // 设置窗口标题
    this->setWindowTitle("几何图形绘制工具");
    
    // 初始化界面
    initUI();
    
    // 创建并初始化OCCT几何绘制器
    m_occtGeometry = new OCCTGeometry(this);
    
    // 创建并初始化QuarterViewer
    m_quarterViewer = new QuarterViewer(this);
    //m_quarterViewer->initialize();
    
    // 将QuarterViewer添加到主布局中
    m_mainLayout->addWidget(m_quarterViewer);
    
    // 连接信号和槽
    connect(m_quarterViewer, &QuarterViewer::mousePressed, this, &MainWindow::onMousePressed);
    connect(m_quarterViewer, &QuarterViewer::mouseMoved, this, &MainWindow::onMouseMoved);
    connect(m_quarterViewer, &QuarterViewer::mouseReleased, this, &MainWindow::onMouseReleased);
    connect(m_occtGeometry, &OCCTGeometry::geometryChanged, this, &MainWindow::updateShapes);
    
    // 设置初始绘制模式
    m_quarterViewer->setCurrentMode(m_currentMode);
    m_occtGeometry->setCurrentGeometryType(static_cast<OCCTGeometryType>(m_currentMode));
    
    // 连接菜单项信号和槽
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
    // 清理界面控件
    delete m_quarterViewer;
    delete m_occtGeometry;
    delete m_mainLayout;
    delete m_centralWidget;
    // 菜单和动作由Qt自动管理，不需要手动删除
}

// 初始化UI界面
void MainWindow::initUI()
{
    // 创建中心部件和主布局
    m_centralWidget = new QWidget(this);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 设置中心部件
    setCentralWidget(m_centralWidget);
    
    // 创建菜单栏
    m_menuBar = menuBar();
    
    // 创建几何图形菜单
    m_geometryMenu = m_menuBar->addMenu("几何图形");
    
    // 创建几何图形菜单项
    m_actionPoint = new QAction("点", this);
    m_actionLine = new QAction("直线", this);
    m_actionCurve = new QAction("曲线", this);
    m_actionRectangle = new QAction("矩形", this);
    m_actionCircle = new QAction("圆形", this);
    m_actionEllipse = new QAction("椭圆", this);
    
    // 添加几何图形菜单项
    m_geometryMenu->addAction(m_actionPoint);
    m_geometryMenu->addAction(m_actionLine);
    m_geometryMenu->addAction(m_actionCurve);
    m_geometryMenu->addAction(m_actionRectangle);
    m_geometryMenu->addAction(m_actionCircle);
    m_geometryMenu->addAction(m_actionEllipse);
    
    // 创建样式设置菜单
    m_styleMenu = m_menuBar->addMenu("样式设置");
    
    // 创建样式设置菜单项
    m_actionSetLineStyle = new QAction("设置线型", this);
    m_actionSetColor = new QAction("设置颜色", this);
    m_actionSetWidth = new QAction("设置线宽", this);
    
    // 添加样式设置菜单项
    m_styleMenu->addAction(m_actionSetLineStyle);
    m_styleMenu->addAction(m_actionSetColor);
    m_styleMenu->addAction(m_actionSetWidth);
    
    // 创建文件菜单
    QMenu *fileMenu = m_menuBar->addMenu("文件");
    
    // 创建打开STEP文件菜单项
    m_actionOpenSTEP = new QAction("打开STEP文件", this);
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
    // 打开线型设置对话框
    QStringList styles = {"实线", "虚线", "点线"};
    QString style = QInputDialog::getItem(this, "设置线型", "选择线型:", styles, 0, false);
    
    if (!style.isEmpty()) {
        int styleIndex = styles.indexOf(style);
        m_occtGeometry->setLineStyle(styleIndex);
        qDebug() << "Line style set to:" << style;
    }
}

void MainWindow::on_actionSetColor_triggered()
{
    // 打开颜色选择对话框
    QColor color = QColorDialog::getColor(Qt::black, this, "选择颜色");
    
    if (color.isValid()) {
        m_occtGeometry->setLineColor(color);
        qDebug() << "Line color set to:" << color.name();
    }
}

void MainWindow::on_actionSetWidth_triggered()
{
    // 打开线宽设置对话框
    bool ok;
    int width = QInputDialog::getInt(this, "设置线宽", "线宽 (1-20):", 2, 1, 20, 1, &ok);
    
    if (ok) {
        m_occtGeometry->setLineWidth(width);
        qDebug() << "Line width set to:" << width;
    }
}

// 鼠标按下事件处理
void MainWindow::onMousePressed(const gp_Pnt &point)
{
    m_isDrawing = true;
    m_occtGeometry->startDrawing(point);
    qDebug() << "Mouse pressed at:" << point.X() << "," << point.Y() << "," << point.Z();
}

// 鼠标移动事件处理
void MainWindow::onMouseMoved(const gp_Pnt &point)
{
    if (m_isDrawing && m_currentMode == OCCT_Curve) {
        m_occtGeometry->continueDrawing(point);
        qDebug() << "Mouse moved at:" << point.X() << "," << point.Y() << "," << point.Z();
    }
}

// 鼠标释放事件处理
void MainWindow::onMouseReleased(const gp_Pnt &point)
{
    if (m_isDrawing) {
        m_isDrawing = false;
        m_occtGeometry->finishDrawing(point);
        qDebug() << "Mouse released at:" << point.X() << "," << point.Y() << "," << point.Z();
    }
}

// 更新视图中的形状
void MainWindow::updateShapes()
{
    // 清除当前所有形状
    m_quarterViewer->clearAllShapes();
    
    // 添加所有新形状
    const auto &shapes = m_occtGeometry->getShapes();
    const auto &colors = m_occtGeometry->getColors();
    const auto &lineStyles = m_occtGeometry->getLineStyles();
    const auto &lineWidths = m_occtGeometry->getLineWidths();
    
    for (size_t i = 0; i < shapes.size(); ++i) {
        m_quarterViewer->addShape(shapes[i], colors[i], lineStyles[i], lineWidths[i]);
    }
    
    // 渲染视图
    m_quarterViewer->render();
}

// 打开STEP文件槽函数
void MainWindow::on_actionOpenSTEP_triggered()
{
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        tr("打开STEP文件"), 
        "", 
        tr("STEP文件 (*.step *.stp)")
    );
    
    if (filePath.isEmpty()) {
        return; // 用户取消选择
    }
    
    try {
        // 使用OCCT读取STEP文件
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toStdString().c_str());
        
        if (status != IFSelect_RetDone) {
            qDebug() << "Failed to read STEP file:" << filePath;
            QMessageBox::warning(this, tr("错误"), tr("无法读取STEP文件"));
            return;
        }
        
        // 转换所有根形状
        reader.TransferRoots();
        
        // 获取读取的形状数量
        int shapeCount = reader.NbRootsForTransfer();
        qDebug() << "Found" << shapeCount << "shapes in STEP file";
        
        // 清除当前所有形状
        m_quarterViewer->clearAllShapes();
        
        // 遍历所有形状并添加到视图
        for (int i = 1; i <= shapeCount; ++i) {
            TopoDS_Shape shape = reader.Shape(i);
            if (!shape.IsNull()) {
                // 使用默认颜色和样式
                Quantity_Color color(Quantity_NOC_BLUE);
                m_quarterViewer->addShape(shape, color, 0, 1);
                qDebug() << "Added shape" << i << "to scene";
            }
        }
        
        // 渲染视图
        m_quarterViewer->render();
        
        QMessageBox::information(this, tr("成功"), tr("STEP文件读取成功"));
        
    } catch (const Standard_Failure &e) {
        qDebug() << "OCCT exception when reading STEP file:" << e.GetMessageString();
        QMessageBox::warning(this, tr("错误"), tr("读取STEP文件时发生OCCT异常"));
    } catch (...) {
        qDebug() << "Unknown exception when reading STEP file";
        QMessageBox::warning(this, tr("错误"), tr("读取STEP文件时发生未知异常"));
    }
}