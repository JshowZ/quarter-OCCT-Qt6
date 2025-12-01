#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

class OccWidget;
class STEPLoader;

class AnotherMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    AnotherMainWindow(QWidget* parent = nullptr);
    ~AnotherMainWindow();

private slots:
    void openSTEPFile();
    void zoomAll();
    void onFileLoaded(bool success, const QString& message);
    void exportSTLFile();

private:
    void setupUI();
    void setupConnections();

    OccWidget* m_occWidget;
    STEPLoader* m_stepLoader;

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlLayout;
    QLabel* m_infoLabel;
    QProgressBar* m_progressBar;
    
    // Menu related members
    QMenuBar* m_menuBar;
    QMenu* m_fileMenu;
    QMenu* m_viewMenu;
    QAction* m_openAction;
    QAction* m_zoomAllAction;
    QAction* m_exportSTLAction;
};