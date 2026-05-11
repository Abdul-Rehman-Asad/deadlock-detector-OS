#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QScrollBar>
#include <QFont>
#include <QCheckBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QApplication>
#include <functional>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_periodicTimer(new QTimer(this)),
    m_simStep(0)
{
    setWindowTitle("Deadlock Detection Tool — CS-2006 OS Project");
    setMinimumSize(1200, 750);

    setupUI();
    setupMenuBar();
    applyDarkTheme();

    connect(m_periodicTimer, &QTimer::timeout, this, &MainWindow::periodicDetect);

    refreshAll();
    statusBar()->showMessage("Ready  |  Group: Abdul Rehman Asad · Muhammad Ghanim Khan · Ali Murtaza Sajjad");
}

MainWindow::~MainWindow() {}

void MainWindow::applyDarkTheme()
{
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #071407;
            color: #8cff80;
            font-family: 'Courier New', Courier, monospace;
        }
        QGroupBox {
            border: 1px solid #1e3060;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 10px;
            font-weight: bold;
            color: #39ff14;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QPushButton {
            background-color: #1a2a4a;
            color: #66ff66;
            border: 1px solid #2a4a8a;
            border-radius: 4px;
            padding: 5px 12px;
            font-family: 'Courier New';
        }
        QPushButton:hover {
            background-color: #243a6a;
            border-color: #39ff14;
        }
        QPushButton:pressed {
            background-color: #0f1e3a;
        }
        QPushButton#btn_detect {
            background-color: #1a3a1a;
            color: #60ff80;
            border-color: #2a7a2a;
            font-weight: bold;
        }
        QPushButton#btn_detect:hover { background-color: #244a24; }
        QPushButton#btn_recover {
            background-color: #3a1a1a;
            color: #ff8060;
            border-color: #8a2a2a;
            font-weight: bold;
        }
        QPushButton#btn_recover:hover { background-color: #4a2424; }
        QPushButton#btn_reset {
            background-color: #2a1a3a;
            color: #39ff14;
            border-color: #6a2a9a;
        }
        QListWidget, QTextEdit, QTableWidget {
            background-color: #080b14;
            color: #a8c8e8;
            border: 1px solid #1a2a4a;
            border-radius: 4px;
            font-family: 'Courier New';
            font-size: 11px;
        }
        QListWidget::item:selected, QTableWidget::item:selected {
            background-color: #1a3a6a;
            color: #ffffff;
        }
        QListWidget::item:hover { background-color: #121c30; }
        QLineEdit, QSpinBox, QComboBox {
            background-color: #080b14;
            color: #a8c8e8;
            border: 1px solid #1a2a4a;
            border-radius: 3px;
            padding: 3px 6px;
            font-family: 'Courier New';
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #0d1220;
            color: #a8c8e8;
            selection-background-color: #1a3a6a;
        }
        QTabWidget::pane { border: 1px solid #1a2a4a; }
        QTabBar::tab {
            background-color: #071407;
            color: #6080a0;
            border: 1px solid #1a2a4a;
            padding: 5px 14px;
            border-bottom: none;
        }
        QTabBar::tab:selected {
            background-color: #141828;
            color: #66ff66;
            border-top: 2px solid #4080cc;
        }
        QHeaderView::section {
            background-color: #101828;
            color: #7090c0;
            border: 1px solid #1a2a4a;
            padding: 4px;
            font-family: 'Courier New';
        }
        QTableWidget { gridline-color: #141e30; }
        QScrollBar:vertical {
            background: #080b14; width: 10px; border: none;
        }
        QScrollBar::handle:vertical {
            background: #1a3060; border-radius: 5px; min-height: 20px;
        }
        QSplitter::handle { background: #1a2a4a; }
        QLabel#deadlockBanner {
            background-color: #3a0808;
            color: #ff6060;
            border: 1px solid #8a1010;
            border-radius: 4px;
            padding: 4px 12px;
            font-weight: bold;
            font-size: 13px;
        }
        QLabel#statusOk {
            color: #50e080;
            font-weight: bold;
        }
        QStatusBar { background: #080b14; color: #4060a0; }
        QMenuBar {
            background: #080b14;
            color: #7090c0;
            border-bottom: 1px solid #1a2a4a;
        }
        QMenuBar::item:selected { background: #1a2a4a; color: #a8c8e8; }
        QMenu {
            background: #0d1220;
            color: #a8c8e8;
            border: 1px solid #1a2a4a;
        }
        QMenu::item:selected { background: #1a3a6a; }
        QCheckBox { color: #7090c0; }
        QCheckBox::indicator:checked { background: #4080cc; border: 1px solid #6090e0; }
    )");
}


void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLay = new QVBoxLayout(central);
    mainLay->setSpacing(4);
    mainLay->setContentsMargins(6, 6, 6, 6);


    m_deadlockBanner = new QLabel("  *** DEADLOCK DETECTED ***");
    m_deadlockBanner->setObjectName("deadlockBanner");
    m_deadlockBanner->setAlignment(Qt::AlignCenter);
    m_deadlockBanner->setVisible(false);
    m_deadlockBanner->setFixedHeight(32);
    mainLay->addWidget(m_deadlockBanner);


    QSplitter *mainSplit = new QSplitter(Qt::Horizontal, this);
    mainLay->addWidget(mainSplit, 1);


    QWidget *leftPanel = new QWidget;
    QVBoxLayout *leftLay = new QVBoxLayout(leftPanel);
    leftLay->setSpacing(6);
    leftLay->setContentsMargins(4, 4, 4, 4);
    leftPanel->setFixedWidth(290);
    mainSplit->addWidget(leftPanel);


    QGroupBox *procGroup = new QGroupBox("Processes");
    QVBoxLayout *procLay = new QVBoxLayout(procGroup);

    m_processList = new QListWidget;
    m_processList->setMaximumHeight(120);
    procLay->addWidget(m_processList);

    QHBoxLayout *addProcRow = new QHBoxLayout;
    m_procNameEdit = new QLineEdit;
    m_procNameEdit->setPlaceholderText("Process name...");
    addProcRow->addWidget(m_procNameEdit);
    QPushButton *btnAddProc = new QPushButton("+ Add");
    addProcRow->addWidget(btnAddProc);
    procLay->addLayout(addProcRow);

    QPushButton *btnRemProc = new QPushButton("– Remove Selected");
    procLay->addWidget(btnRemProc);
    leftLay->addWidget(procGroup);


    QGroupBox *resGroup = new QGroupBox("Resources");
    QVBoxLayout *resLay = new QVBoxLayout(resGroup);

    m_resourceList = new QListWidget;
    m_resourceList->setMaximumHeight(100);
    resLay->addWidget(m_resourceList);

    QHBoxLayout *addResRow = new QHBoxLayout;
    m_resNameEdit = new QLineEdit;
    m_resNameEdit->setPlaceholderText("Resource name...");
    addResRow->addWidget(m_resNameEdit);
    m_resInstancesSpin = new QSpinBox;
    m_resInstancesSpin->setRange(1, 10);
    m_resInstancesSpin->setValue(1);
    m_resInstancesSpin->setFixedWidth(50);
    m_resInstancesSpin->setToolTip("Instances");
    addResRow->addWidget(m_resInstancesSpin);
    QPushButton *btnAddRes = new QPushButton("+ Add");
    addResRow->addWidget(btnAddRes);
    resLay->addLayout(addResRow);

    QPushButton *btnRemRes = new QPushButton("– Remove Selected");
    resLay->addWidget(btnRemRes);
    leftLay->addWidget(resGroup);


    QGroupBox *allocGroup = new QGroupBox("Allocation Operations");
    QGridLayout *allocLay = new QGridLayout(allocGroup);

    allocLay->addWidget(new QLabel("Process:"), 0, 0);
    m_allocProcCombo = new QComboBox;
    allocLay->addWidget(m_allocProcCombo, 0, 1);

    allocLay->addWidget(new QLabel("Resource:"), 1, 0);
    m_allocResCombo = new QComboBox;
    allocLay->addWidget(m_allocResCombo, 1, 1);

    QPushButton *btnRequest  = new QPushButton(">> Request (Wait if busy)");
    QPushButton *btnAllocate = new QPushButton(">> Allocate (Force Grant)");
    QPushButton *btnRelease  = new QPushButton(">> Release Resource");
    btnRequest->setStyleSheet("QPushButton { color: #ffcc40; background: #1a2a0a; border: 1px solid #4a6a1a; padding: 5px; }  QPushButton:hover { background: #2a3a1a; }");
    btnAllocate->setStyleSheet("QPushButton { color: #60ff80; background: #0a1a2a; border: 1px solid #1a4a6a; padding: 5px; } QPushButton:hover { background: #1a2a3a; }");
    btnRelease->setStyleSheet("QPushButton { color: #ff8060; background: #2a1a0a; border: 1px solid #6a3a1a; padding: 5px; }  QPushButton:hover { background: #3a2a1a; }");
    allocLay->addWidget(btnRequest,  2, 0, 1, 2);
    allocLay->addWidget(btnAllocate, 3, 0, 1, 2);
    allocLay->addWidget(btnRelease,  4, 0, 1, 2);
    leftLay->addWidget(allocGroup);


    QGroupBox *detectGroup = new QGroupBox("Detection & Recovery");
    QVBoxLayout *detectLay = new QVBoxLayout(detectGroup);

    QPushButton *btnDetect  = new QPushButton("[+] Detect Deadlock");
    btnDetect->setObjectName("btn_detect");
    QPushButton *btnRecover = new QPushButton("[!] Auto-Recover (Terminate)");
    btnRecover->setObjectName("btn_recover");

    QHBoxLayout *manualRow = new QHBoxLayout;
    m_terminateProcCombo = new QComboBox;
    QPushButton *btnManTerm = new QPushButton("Kill");
    btnManTerm->setObjectName("btn_recover");
    manualRow->addWidget(new QLabel("Manual:"));
    manualRow->addWidget(m_terminateProcCombo, 1);
    manualRow->addWidget(btnManTerm);

    QCheckBox *chkPeriodic = new QCheckBox("Periodic detection (3s)");
    QPushButton *btnReset   = new QPushButton("[R] Reset System");
    btnReset->setObjectName("btn_reset");

    detectLay->addWidget(btnDetect);
    detectLay->addWidget(btnRecover);
    detectLay->addLayout(manualRow);
    detectLay->addWidget(chkPeriodic);
    detectLay->addWidget(btnReset);
    leftLay->addWidget(detectGroup);


    QGroupBox *testGroup = new QGroupBox("Test Cases");
    QVBoxLayout *testLay = new QVBoxLayout(testGroup);
    QComboBox *testCombo = new QComboBox;
    testCombo->addItem("TC0: No Deadlock");
    testCombo->addItem("TC1: Single Deadlock (2 procs)");
    testCombo->addItem("TC2: Multi-process Deadlock (3)");
    testCombo->addItem("TC3: Multiple Deadlocks");
    testCombo->addItem("TC4: Stress Test (5 procs)");
    testLay->addWidget(testCombo);
    QPushButton *btnLoadTest = new QPushButton("Load Test Case");
    testLay->addWidget(btnLoadTest);

    QPushButton *btnStep = new QPushButton("[>] Step Simulation");
    testLay->addWidget(btnStep);
    leftLay->addWidget(testGroup);

    leftLay->addStretch();


    QWidget *centerPanel = new QWidget;
    QVBoxLayout *centerLay = new QVBoxLayout(centerPanel);
    centerLay->setContentsMargins(4, 4, 4, 4);

    QLabel *graphTitle = new QLabel("  Wait-For Graph  (drag nodes to reposition)");
    graphTitle->setStyleSheet("color: #4878b8; font-size: 11px; border-bottom: 1px solid #1a2a4a;");
    centerLay->addWidget(graphTitle);

    m_graphWidget = new GraphWidget;
    centerLay->addWidget(m_graphWidget, 1);
    mainSplit->addWidget(centerPanel);


    QTabWidget *rightTabs = new QTabWidget;
    mainSplit->addWidget(rightTabs);


    QWidget *allocTab = new QWidget;
    QVBoxLayout *allocTabLay = new QVBoxLayout(allocTab);
    m_allocTable = new QTableWidget;
    m_allocTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_allocTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    allocTabLay->addWidget(m_allocTable);
    rightTabs->addTab(allocTab, "Allocation Matrix");

    QWidget *logTab = new QWidget;
    QVBoxLayout *logLay = new QVBoxLayout(logTab);
    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setFont(QFont("Courier New", 9));
    logLay->addWidget(m_logView);
    QHBoxLayout *logBtns = new QHBoxLayout;
    QPushButton *btnClearLog = new QPushButton("Clear Log");
    QPushButton *btnSaveLog  = new QPushButton("Save Log");
    logBtns->addWidget(btnClearLog);
    logBtns->addWidget(btnSaveLog);
    logBtns->addStretch();
    logLay->addLayout(logBtns);
    rightTabs->addTab(logTab, "Event Log");


    QWidget *snapTab = new QWidget;
    QVBoxLayout *snapLay = new QVBoxLayout(snapTab);
    QTextEdit *snapView = new QTextEdit;
    snapView->setReadOnly(true);
    snapView->setObjectName("snapView");
    snapLay->addWidget(snapView);
    QPushButton *btnRefreshSnap = new QPushButton("Refresh Snapshot");
    snapLay->addWidget(btnRefreshSnap);
    rightTabs->addTab(snapTab, "State Snapshot");

    mainSplit->setSizes({290, 560, 340});


    connect(btnAddProc, &QPushButton::clicked, this, &MainWindow::onAddProcess);
    connect(m_procNameEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddProcess);
    connect(btnRemProc, &QPushButton::clicked, this, &MainWindow::onRemoveProcess);

    connect(btnAddRes, &QPushButton::clicked, this, &MainWindow::onAddResource);
    connect(m_resNameEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddResource);
    connect(btnRemRes, &QPushButton::clicked, this, &MainWindow::onRemoveResource);

    connect(btnRequest,  &QPushButton::clicked, this, &MainWindow::onRequestResource);
    connect(btnAllocate, &QPushButton::clicked, this, &MainWindow::onAllocateResource);
    connect(btnRelease,  &QPushButton::clicked, this, &MainWindow::onReleaseResource);

    connect(btnDetect,  &QPushButton::clicked, this, &MainWindow::onDetectDeadlock);
    connect(btnRecover, &QPushButton::clicked, this, &MainWindow::onAutoRecover);
    connect(btnManTerm, &QPushButton::clicked, this, &MainWindow::onManualTerminate);
    connect(btnReset,   &QPushButton::clicked, this, &MainWindow::onResetSystem);

    connect(chkPeriodic, &QCheckBox::toggled, this, &MainWindow::onTogglePeriodicDetection);

    connect(btnLoadTest, &QPushButton::clicked, this, [this, testCombo](){
        onLoadTestCase(testCombo->currentIndex());
    });
    connect(btnStep, &QPushButton::clicked, this, &MainWindow::onStepSimulation);

    connect(btnClearLog, &QPushButton::clicked, this, [this](){
        m_rm.clearLog();
        m_logView->clear();
    });
    connect(btnSaveLog, &QPushButton::clicked, this, [this](){
        QString fname = QFileDialog::getSaveFileName(this, "Save Log", "deadlock_log.txt", "Text (*.txt)");
        if (!fname.isEmpty()) {
            QFile f(fname);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&f);
                for (auto &e : m_rm.log())
                    out << "[" << e.timestamp << "] [" << e.type << "] " << e.message << "\n";
            }
        }
    });

    connect(btnRefreshSnap, &QPushButton::clicked, this, [snapView, this](){
        snapView->setPlainText(m_rm.stateSnapshot());
    });
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Reset System", this, &MainWindow::onResetSystem);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, []{ QApplication::quit(); });

    QMenu *detectMenu = menuBar()->addMenu("&Detect");
    detectMenu->addAction("Detect Deadlock", this, &MainWindow::onDetectDeadlock);
    detectMenu->addAction("Auto-Recover", this, &MainWindow::onAutoRecover);

    QMenu *testMenu = menuBar()->addMenu("&Tests");
    testMenu->addAction("TC0: No Deadlock",         this, [this](){ onLoadTestCase(0); });
    testMenu->addAction("TC1: Single Deadlock",     this, [this](){ onLoadTestCase(1); });
    testMenu->addAction("TC2: Multi Deadlock",      this, [this](){ onLoadTestCase(2); });
    testMenu->addAction("TC3: Multiple Deadlocks",  this, [this](){ onLoadTestCase(3); });
    testMenu->addAction("TC4: Stress Test",         this, [this](){ onLoadTestCase(4); });

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About", this, [this](){
        QMessageBox::information(this, "About",
                                 "Deadlock Detection Tool\n"
                                 "Course: CS-2006 Operating Systems\n"
                                 "Instructor: Dr. Ghufran Ahmed\n\n"
                                 "Group Members:\n"
                                 "  Abdul Rehman Asad    24k-0722\n"
                                 "  Muhammad Ghanim Khan 24k-0850\n"
                                 "  Ali Murtaza Sajjad  24k-0763\n\n"
                                 "Algorithm: DFS-based cycle detection O(V+E)\n"
                                 "Recovery: Process termination / preemption");
    });
}


void MainWindow::refreshAll()
{
    refreshProcessList();
    refreshResourceList();
    refreshAllocationTable();
    refreshLog();
    m_graphWidget->updateGraph(&m_rm);
    updateStatusBar();


    m_allocProcCombo->clear();
    m_allocResCombo->clear();
    m_terminateProcCombo->clear();
    for (auto &p : m_rm.processes()) {
        m_allocProcCombo->addItem(p.name, p.id);
        m_terminateProcCombo->addItem(p.name, p.id);
    }
    for (auto &r : m_rm.resources())
        m_allocResCombo->addItem(r.name, r.id);
}

void MainWindow::refreshProcessList()
{
    m_processList->clear();
    for (auto &p : m_rm.processes()) {
        QString status = p.isDeadlocked ? " [DEADLOCK]"
                         : p.isBlocked   ? " [BLOCKED]"
                                        : " [RUNNING]";
        auto *item = new QListWidgetItem(
            QString("P%1: %2%3").arg(p.id).arg(p.name).arg(status));
        item->setData(Qt::UserRole, p.id);
        if (p.isDeadlocked) item->setForeground(QColor(255, 80, 80));
        else if (p.isBlocked) item->setForeground(QColor(255, 180, 0));
        else item->setForeground(QColor(80, 200, 120));
        m_processList->addItem(item);
    }
}

void MainWindow::refreshResourceList()
{
    m_resourceList->clear();
    for (auto &r : m_rm.resources()) {
        auto *item = new QListWidgetItem(
            QString("R%1: %2  [%3/%4 free]")
                .arg(r.id).arg(r.name)
                .arg(r.availableInstances).arg(r.totalInstances));
        item->setData(Qt::UserRole, r.id);
        if (r.availableInstances == 0)
            item->setForeground(QColor(255, 120, 60));
        else
            item->setForeground(QColor(80, 180, 255));
        m_resourceList->addItem(item);
    }
}

void MainWindow::refreshAllocationTable()
{
    const auto &procs = m_rm.processes();
    const auto &ress  = m_rm.resources();
    const auto &alloc = m_rm.allocation();
    const auto &reqs  = m_rm.requests();

    int rows = procs.size();
    int cols = ress.size() * 2;

    m_allocTable->setRowCount(rows);
    m_allocTable->setColumnCount(cols);

    QStringList hdr;
    for (auto &r : ress) {
        hdr << r.name + "\n(held)";
        hdr << r.name + "\n(wait)";
    }
    m_allocTable->setHorizontalHeaderLabels(hdr);

    QStringList rowHdr;
    int row = 0;
    for (auto &p : procs) {
        rowHdr << p.name;
        int col = 0;
        for (auto &r : ress) {
            int held = alloc.value(p.id).value(r.id, 0);
            int wait = reqs.value(p.id).value(r.id, 0);

            auto *heldItem = new QTableWidgetItem(held > 0 ? QString::number(held) : "·");
            auto *waitItem = new QTableWidgetItem(wait > 0 ? QString::number(wait) : "·");
            heldItem->setTextAlignment(Qt::AlignCenter);
            waitItem->setTextAlignment(Qt::AlignCenter);
            if (held > 0) heldItem->setForeground(QColor(80, 200, 120));
            if (wait > 0) waitItem->setForeground(QColor(255, 180, 0));
            m_allocTable->setItem(row, col,   heldItem);
            m_allocTable->setItem(row, col+1, waitItem);
            col += 2;
        }
        row++;
    }
    m_allocTable->setVerticalHeaderLabels(rowHdr);
}

void MainWindow::refreshLog()
{
    m_logView->clear();
    for (auto &e : m_rm.log()) {
        QString colorTag;
        if      (e.type == "DEADLOCK") colorTag = "#ff5050";
        else if (e.type == "RECOVERY") colorTag = "#ff9050";
        else if (e.type == "WARN")     colorTag = "#ffcc40";
        else if (e.type == "ERROR")    colorTag = "#ff4040";
        else                           colorTag = "#70a8d0";

        m_logView->append(
            QString("<span style='color:#445566'>[%1]</span> "
                    "<span style='color:%2'>[%3]</span> "
                    "<span style='color:#a8c8e8'>%4</span>")
                .arg(e.timestamp, colorTag, e.type, e.message));
    }
    m_logView->verticalScrollBar()->setValue(
        m_logView->verticalScrollBar()->maximum());
}

void MainWindow::updateStatusBar()
{
    int procs = m_rm.processes().size();
    int ress  = m_rm.resources().size();
    int blocked = 0;
    for (auto &p : m_rm.processes())
        if (p.isBlocked) blocked++;

    statusBar()->showMessage(
        QString("Processes: %1  |  Resources: %2  |  Blocked: %3  |  "
                "Group: Abdul Rehman Asad · Muhammad Ghanim Khan · Ali Murtaza Sajjad")
            .arg(procs).arg(ress).arg(blocked));
}


void MainWindow::onAddProcess()
{
    QString name = m_procNameEdit->text().trimmed();
    if (name.isEmpty()) { QMessageBox::warning(this, "Error", "Process name cannot be empty."); return; }
    m_rm.addProcess(name);
    m_procNameEdit->clear();
    refreshAll();
}

void MainWindow::onRemoveProcess()
{
    auto *item = m_processList->currentItem();
    if (!item) { QMessageBox::warning(this, "Error", "Select a process first."); return; }
    int pid = item->data(Qt::UserRole).toInt();
    m_rm.removeProcess(pid);
    refreshAll();
}


void MainWindow::onAddResource()
{
    QString name = m_resNameEdit->text().trimmed();
    if (name.isEmpty()) { QMessageBox::warning(this, "Error", "Resource name cannot be empty."); return; }
    m_rm.addResource(name, m_resInstancesSpin->value());
    m_resNameEdit->clear();
    refreshAll();
}

void MainWindow::onRemoveResource()
{
    auto *item = m_resourceList->currentItem();
    if (!item) { QMessageBox::warning(this, "Error", "Select a resource first."); return; }
    int rid = item->data(Qt::UserRole).toInt();
    m_rm.removeResource(rid);
    refreshAll();
}


void MainWindow::onRequestResource()
{
    int pid = m_allocProcCombo->currentData().toInt();
    int rid = m_allocResCombo->currentData().toInt();
    m_rm.requestResource(pid, rid);
    refreshAll();
}

void MainWindow::onAllocateResource()
{
    int pid = m_allocProcCombo->currentData().toInt();
    int rid = m_allocResCombo->currentData().toInt();
    m_rm.allocateResource(pid, rid);
    refreshAll();
}

void MainWindow::onReleaseResource()
{
    int pid = m_allocProcCombo->currentData().toInt();
    int rid = m_allocResCombo->currentData().toInt();
    m_rm.releaseResource(pid, rid);
    refreshAll();
}


void MainWindow::onDetectDeadlock()
{
    QVector<int> deadlocked = m_rm.detectDeadlock();
    QVector<QVector<int>> cycles = m_rm.getAllCycles();

    m_graphWidget->setDeadlockedPids(deadlocked);
    m_graphWidget->setDeadlockCycles(cycles);
    m_graphWidget->updateGraph(&m_rm);

    if (!deadlocked.isEmpty()) {
        QString names;
        for (int pid : deadlocked)
            names += m_rm.processes().value(pid).name + " ";

        m_deadlockBanner->setText(
            "  ⚠  DEADLOCK DETECTED: " + names.trimmed() + "  ⚠");
        m_deadlockBanner->setVisible(true);

        // Show cycle info
        QString cycleStr = "Deadlock Cycles Detected:\n";
        for (auto &cycle : cycles) {
            QStringList names2;
            for (int i = 0; i < cycle.size(); i++)
                names2 << m_rm.processes().value(cycle[i]).name;
            cycleStr += "  → " + names2.join(" → ") + "\n";
        }
        QMessageBox::warning(this, "Deadlock Detected", cycleStr);
    } else {
        m_deadlockBanner->setVisible(false);
        QMessageBox::information(this, "No Deadlock", "✓ System is deadlock-free.");
    }

    refreshProcessList();
    refreshLog();
}


void MainWindow::onAutoRecover()
{
    QVector<int> deadlocked = m_rm.detectDeadlock();
    if (deadlocked.isEmpty()) {
        QMessageBox::information(this, "Recovery", "No deadlock found — nothing to recover.");
        return;
    }


    int rounds = 0;
    while (!deadlocked.isEmpty() && rounds++ < 20) {
        int victim = m_rm.chooseBestVictim(deadlocked);
        if (victim < 0) break;
        m_rm.terminateProcess(victim);
        deadlocked = m_rm.detectDeadlock();
    }

    m_graphWidget->setDeadlockedPids({});
    m_graphWidget->setDeadlockCycles({});
    m_deadlockBanner->setVisible(false);
    refreshAll();
    QMessageBox::information(this, "Recovery Complete",
                             "Deadlock resolved. " + QString::number(rounds) + " process(es) terminated.");
}

void MainWindow::onManualTerminate()
{
    int pid = m_terminateProcCombo->currentData().toInt();
    if (!m_rm.processExists(pid)) {
        QMessageBox::warning(this, "Error", "Invalid process selected."); return;
    }
    QString name = m_rm.processes().value(pid).name;
    auto res = QMessageBox::question(this, "Confirm",
                                     "Terminate process '" + name + "'?\nAll its resources will be released.");
    if (res == QMessageBox::Yes) {
        m_rm.terminateProcess(pid);
        m_rm.detectDeadlock();
        m_graphWidget->setDeadlockedPids({});
        m_graphWidget->setDeadlockCycles({});
        m_deadlockBanner->setVisible(false);
        refreshAll();
    }
}


void MainWindow::onTogglePeriodicDetection(bool checked)
{
    if (checked)
        m_periodicTimer->start(3000);
    else
        m_periodicTimer->stop();
}

void MainWindow::periodicDetect()
{
    QVector<int> dl = m_rm.detectDeadlock();
    QVector<QVector<int>> cycles = m_rm.getAllCycles();
    m_graphWidget->setDeadlockedPids(dl);
    m_graphWidget->setDeadlockCycles(cycles);
    m_graphWidget->updateGraph(&m_rm);
    m_deadlockBanner->setVisible(!dl.isEmpty());
    refreshProcessList();
    refreshLog();
}


void MainWindow::onResetSystem()
{
    auto res = QMessageBox::question(this, "Reset",
                                     "Reset the entire system? All processes and resources will be cleared.");
    if (res == QMessageBox::Yes) {
        m_rm.reset();
        m_graphWidget->setDeadlockedPids({});
        m_graphWidget->setDeadlockCycles({});
        m_deadlockBanner->setVisible(false);
        m_simSteps.clear();
        m_simStep = 0;
        refreshAll();
    }
}


void MainWindow::onStepSimulation()
{
    if (m_simSteps.isEmpty()) {
        QMessageBox::information(this, "Simulation",
                                 "Load a test case first, then use Step to walk through it.");
        return;
    }
    if (m_simStep < m_simSteps.size()) {
        m_simSteps[m_simStep]();
        m_simStep++;
        refreshAll();
        if (m_simStep >= m_simSteps.size())
            QMessageBox::information(this, "Simulation",
                                     "Simulation complete. Click Detect Deadlock to check.");
    } else {
        QMessageBox::information(this, "Simulation", "All steps executed.");
    }
}


void MainWindow::onLoadTestCase(int index)
{
    m_rm.reset();
    m_graphWidget->setDeadlockedPids({});
    m_graphWidget->setDeadlockCycles({});
    m_deadlockBanner->setVisible(false);
    m_simSteps.clear();
    m_simStep = 0;

    switch (index) {
    case 0: runTestCase0(); break;
    case 1: runTestCase1(); break;
    case 2: runTestCase2(); break;
    case 3: runTestCase3(); break;
    case 4: runTestCase4(); break;
    }
    refreshAll();
}


void MainWindow::runTestCase0()
{
    // Setup steps
    m_simSteps = {
        [this]{ m_rm.addProcess("P1"); m_rm.addProcess("P2"); },
        [this]{ m_rm.addResource("R1"); m_rm.addResource("R2"); },
        [this]{ m_rm.allocateResource(0, 0); }, // P1 gets R1
        [this]{ m_rm.allocateResource(1, 1); }, // P2 gets R2
    };
    // Run all
    for (auto &s : m_simSteps) s();
    m_simStep = m_simSteps.size();
    QMessageBox::information(this, "TC0 Loaded",
                             "TC0: No Deadlock\n\n"
                             "P1 holds R1, P2 holds R2.\n"
                             "Neither is waiting → No deadlock.\n\n"
                             "Click 'Detect Deadlock' to verify.");
}


void MainWindow::runTestCase1()
{
    m_simSteps = {
        [this]{ m_rm.addProcess("P1"); m_rm.addProcess("P2"); },
        [this]{ m_rm.addResource("R1"); m_rm.addResource("R2"); },
        [this]{ m_rm.allocateResource(0, 0); },
        [this]{ m_rm.allocateResource(1, 1); },
        [this]{ m_rm.requestResource(0, 1); },
        [this]{ m_rm.requestResource(1, 0); },
    };
    for (auto &s : m_simSteps) s();
    m_simStep = m_simSteps.size();
    QMessageBox::warning(this, "TC1 Loaded",
                         "TC1: Single Deadlock (2 processes)\n\n"
                         "P1 holds R1, waiting for R2\n"
                         "P2 holds R2, waiting for R1\n\n"
                         "→ Circular wait: P1 ↔ P2\n\n"
                         "Click 'Detect Deadlock' to visualize.");
}


void MainWindow::runTestCase2()
{
    m_simSteps = {
        [this]{ m_rm.addProcess("P1"); m_rm.addProcess("P2"); m_rm.addProcess("P3"); },
        [this]{ m_rm.addResource("R1"); m_rm.addResource("R2"); m_rm.addResource("R3"); },
        [this]{ m_rm.allocateResource(0, 0); },
        [this]{ m_rm.allocateResource(1, 1); },
        [this]{ m_rm.allocateResource(2, 2); },
        [this]{ m_rm.requestResource(0, 1); },
        [this]{ m_rm.requestResource(1, 2); },
        [this]{ m_rm.requestResource(2, 0); },
    };
    for (auto &s : m_simSteps) s();
    m_simStep = m_simSteps.size();
    QMessageBox::warning(this, "TC2 Loaded",
                         "TC2: Multi-process Deadlock (3 processes)\n\n"
                         "P1 holds R1, waiting R2\n"
                         "P2 holds R2, waiting R3\n"
                         "P3 holds R3, waiting R1\n\n"
                         "→ Cycle: P1 → P2 → P3 → P1\n\n"
                         "Click 'Detect Deadlock' to visualize.");
}


void MainWindow::runTestCase3()
{
    m_simSteps = {
        [this]{
            m_rm.addProcess("P1"); m_rm.addProcess("P2");
            m_rm.addProcess("P3"); m_rm.addProcess("P4");
            m_rm.addProcess("P5");
        },
        [this]{
            m_rm.addResource("R1"); m_rm.addResource("R2");
            m_rm.addResource("R3"); m_rm.addResource("R4");
        },
        [this]{

            m_rm.allocateResource(0, 0);
            m_rm.allocateResource(1, 1);
            m_rm.requestResource(0, 1);
            m_rm.requestResource(1, 0);
        },
        [this]{

            m_rm.allocateResource(2, 2);
            m_rm.allocateResource(3, 3);
            m_rm.requestResource(2, 3);
            m_rm.requestResource(3, 2);
        },
        [this]{

        }
    };
    for (auto &s : m_simSteps) s();
    m_simStep = m_simSteps.size();
    QMessageBox::warning(this, "TC3 Loaded",
                         "TC3: Multiple Deadlocks\n\n"
                         "Deadlock #1: P1 ↔ P2\n"
                         "Deadlock #2: P3 ↔ P4\n"
                         "P5: running normally (no deadlock)\n\n"
                         "Click 'Detect Deadlock' to visualize.");
}


void MainWindow::runTestCase4()
{
    m_simSteps = {
        [this]{
            for (int i = 1; i <= 5; i++)
                m_rm.addProcess("P" + QString::number(i));
            for (int i = 1; i <= 5; i++)
                m_rm.addResource("R" + QString::number(i));
        },
        [this]{

            for (int i = 0; i < 5; i++)
                m_rm.allocateResource(i, i);
        },
        [this]{

            m_rm.requestResource(0, 1);
            m_rm.requestResource(1, 2);
            m_rm.requestResource(2, 3);
            m_rm.requestResource(3, 4);
            m_rm.requestResource(4, 0);
        }
    };
    for (auto &s : m_simSteps) s();
    m_simStep = m_simSteps.size();
    QMessageBox::warning(this, "TC4 Loaded",
                         "TC4: Stress Test (5 processes)\n\n"
                         "All 5 processes form a single deadlock ring:\n"
                         "P1→P2→P3→P4→P5→P1\n\n"
                         "Tests DFS performance on larger graphs.\n"
                         "Click 'Detect Deadlock' to visualize.");
}
