#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "resourcemanager.h"
#include "graphwidget.h"

class QListWidget;
class QTextEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QLineEdit;
class QComboBox;
class QTabWidget;
class QTableWidget;
class QGroupBox;
class QProgressBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onAddProcess();
    void onRemoveProcess();

    void onAddResource();
    void onRemoveResource();

    void onRequestResource();
    void onAllocateResource();
    void onReleaseResource();

    void onDetectDeadlock();
    void onAutoRecover();
    void onManualTerminate();

    void onLoadTestCase(int index);
    void onResetSystem();
    void onStepSimulation();
    void onTogglePeriodicDetection(bool checked);
    void periodicDetect();

    void refreshAll();
    void refreshProcessList();
    void refreshResourceList();
    void refreshAllocationTable();
    void refreshLog();
    void updateStatusBar();

private:
    void setupUI();
    void setupMenuBar();
    void setupStyleSheet();
    void applyDarkTheme();

    void runTestCase0();
    void runTestCase1();
    void runTestCase2();
    void runTestCase3();
    void runTestCase4();

    ResourceManager m_rm;
    GraphWidget    *m_graphWidget;


    QListWidget  *m_processList;
    QListWidget  *m_resourceList;
    QTextEdit    *m_logView;
    QTableWidget *m_allocTable;
    QLabel       *m_statusLabel;
    QLabel       *m_deadlockBanner;


    QLineEdit    *m_procNameEdit;
    QLineEdit    *m_resNameEdit;
    QSpinBox     *m_resInstancesSpin;
    QComboBox    *m_allocProcCombo;
    QComboBox    *m_allocResCombo;
    QComboBox    *m_terminateProcCombo;

    QTimer       *m_periodicTimer;


    QVector<std::function<void()>> m_simSteps;
    int m_simStep;
};

#endif
