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
    
    // Mouse event handling slots
    void onMousePressed(const gp_Pnt &point);
    void onMouseMoved(const gp_Pnt &point);
    void onMouseReleased(const gp_Pnt &point);
    
    // View update slot
    void updateShapes();

private:
    // Main layout
    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;
    
    // Menu bar related
    QMenuBar *m_menuBar;
    QMenu *m_geometryMenu;
    QMenu *m_styleMenu;
    QMenu *m_fileMenu;
    
    // Menu items
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
    
    // OCCT geometry drawer
    OCCTGeometry *m_occtGeometry;
    
    // QuarterViewer
    QuarterViewer *m_quarterViewer;
    
    // Current drawing mode
    int m_currentMode;
    
    // Drawing status flag
    bool m_isDrawing;
    
    // Initialize UI
    void initUI();
};

#endif // MAINWINDOW_H