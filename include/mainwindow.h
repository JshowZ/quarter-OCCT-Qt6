#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "occtgeometry.h"
#include "quarterviewer.h"
#include <gp_Pnt.hxx>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionPoint_triggered();
    void on_actionLine_triggered();
    void on_actionCurve_triggered();
    void on_actionRectangle_triggered();
    void on_actionCircle_triggered();
    void on_actionEllipse_triggered();
    void on_actionSetLineStyle_triggered();
    void on_actionSetColor_triggered();
    void on_actionSetWidth_triggered();
    void on_actionOpenSTEP_triggered();
    
    // 鼠标事件处理槽
    void onMousePressed(const gp_Pnt &point);
    void onMouseMoved(const gp_Pnt &point);
    void onMouseReleased(const gp_Pnt &point);
    
    // 视图更新槽
    void updateShapes();

private:
    // 主布局
    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;
    
    // 菜单栏相关
    QMenuBar *m_menuBar;
    QMenu *m_geometryMenu;
    QMenu *m_styleMenu;
    
    // 菜单项
    QAction *m_actionPoint;
    QAction *m_actionLine;
    QAction *m_actionCurve;
    QAction *m_actionRectangle;
    QAction *m_actionCircle;
    QAction *m_actionEllipse;
    QAction *m_actionSetLineStyle;
    QAction *m_actionSetColor;
    QAction *m_actionSetWidth;
    QAction *m_actionOpenSTEP;
    
    // OCCT几何绘制器
    OCCTGeometry *m_occtGeometry;
    
    // QuarterViewer
    QuarterViewer *m_quarterViewer;
    
    // 当前绘制模式
    int m_currentMode;
    
    // 绘制状态标志
    bool m_isDrawing;
    
    // 初始化UI
    void initUI();
};

#endif // MAINWINDOW_H