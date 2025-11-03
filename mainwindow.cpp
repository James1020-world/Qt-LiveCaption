#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <windows.h>
#include <psapi.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_liveCaptionsProcess(nullptr)
    , m_captureTimer(nullptr)
    , m_captionWindow(nullptr)
    , m_isRunning(false)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    
    // Initialize language mapping
    m_languageMap["English"] = "en-US";
    m_languageMap["Spanish"] = "es-ES";
    m_languageMap["French"] = "fr-FR";
    m_languageMap["German"] = "de-DE";
    m_languageMap["Japanese"] = "ja-JP";
    m_languageMap["Chinese"] = "zh-CN";
    m_languageMap["Korean"] = "ko-KR";
    
    // Find Live Captions executable
    if (!findLiveCaptionsPath()) {
        QMessageBox::warning(this, "Warning", 
            "LiveCaptions.exe not found. Some features may not work properly.");
    }
}

MainWindow::~MainWindow()
{
    stopLiveCaptionsProcess();
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("Live Captions Controller - Qt Demo");
    setMinimumSize(800, 600);
    
    // Populate language combo
    ui->languageCombo->addItems(m_languageMap.keys());
    
    // Populate position combo
    ui->positionCombo->addItem("Top Left", 1);
    ui->positionCombo->addItem("Top Right", 2);
    ui->positionCombo->addItem("Bottom Left", 3);
    ui->positionCombo->addItem("Bottom Right", 4);
    ui->positionCombo->addItem("Center Top", 5);
    ui->positionCombo->addItem("Center Bottom", 6);
    
    // Set initial values
    ui->opacitySlider->setValue(80);
    ui->stopButton->setEnabled(false);
}

void MainWindow::setupConnections()
{
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::on_stopButton_clicked);
    connect(ui->languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::on_languageCombo_currentIndexChanged);
    connect(ui->positionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::on_positionCombo_currentIndexChanged);
    connect(ui->opacitySlider, &QSlider::valueChanged, 
            this, &MainWindow::on_opacitySlider_valueChanged);
}

bool MainWindow::findLiveCaptionsPath()
{
    // Common paths where LiveCaptions.exe might be located
    QStringList possiblePaths = {
        "C:/Windows/System32/LiveCaptions.exe",
        "C:/Windows/SysWOW64/LiveCaptions.exe",
        QDir::homePath() + "/AppData/Local/Microsoft/WindowsApps/LiveCaptions.exe",
        "C:/Program Files/WindowsApps/Microsoft.Windows.LiveCaptions_*/LiveCaptions.exe"
    };
    
    for (const QString &path : possiblePaths) {
        if (QFile::exists(path)) {
            m_liveCaptionsPath = path;
            qDebug() << "Found LiveCaptions.exe at:" << path;
            return true;
        }
    }
    
    // Try to find in WindowsApps (might require wildcard expansion)
    QDir windowsAppsDir(QDir::homePath() + "/AppData/Local/Microsoft/WindowsApps");
    if (windowsAppsDir.exists("LiveCaptions.exe")) {
        m_liveCaptionsPath = windowsAppsDir.absoluteFilePath("LiveCaptions.exe");
        return true;
    }
    
    return false;
}

void MainWindow::on_startButton_clicked()
{
    startLiveCaptionsProcess();
}

void MainWindow::on_stopButton_clicked()
{
    stopLiveCaptionsProcess();
}

void MainWindow::startLiveCaptionsProcess()
{
    if (m_isRunning) {
        return;
    }
    
    if (m_liveCaptionsPath.isEmpty() && !findLiveCaptionsPath()) {
        QMessageBox::critical(this, "Error", "Could not find LiveCaptions.exe");
        return;
    }
    
    // Kill any existing Live Captions process
    stopLiveCaptionsProcess();
    
    // Start new process
    m_liveCaptionsProcess = new QProcess(this);
    m_liveCaptionsProcess->setProgram(m_liveCaptionsPath);
    
    // Connect signals
    connect(m_liveCaptionsProcess, &QProcess::readyReadStandardOutput, 
            this, &MainWindow::readLiveCaptionsOutput);
    connect(m_liveCaptionsProcess, &QProcess::readyReadStandardError, 
            this, &MainWindow::readLiveCaptionsOutput);
    connect(m_liveCaptionsProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                qDebug() << "LiveCaptions process finished with code:" << exitCode;
                m_isRunning = false;
                ui->startButton->setEnabled(true);
                ui->stopButton->setEnabled(false);
                if (m_captureTimer) {
                    m_captureTimer->stop();
                }
            });
    
    // Start the process
    m_liveCaptionsProcess->start();
    
    if (m_liveCaptionsProcess->waitForStarted(3000)) {
        m_isRunning = true;
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->statusLabel->setText("Live Captions Running");
        
        // Start timer to capture caption window
        if (!m_captureTimer) {
            m_captureTimer = new QTimer(this);
            connect(m_captureTimer, &QTimer::timeout, this, &MainWindow::on_captureTimer_timeout);
        }
        m_captureTimer->start(500); // Check every 500ms
        
        qDebug() << "LiveCaptions started successfully";
    } else {
        QMessageBox::critical(this, "Error", "Failed to start LiveCaptions.exe");
        delete m_liveCaptionsProcess;
        m_liveCaptionsProcess = nullptr;
    }
}

void MainWindow::stopLiveCaptionsProcess()
{
    if (m_liveCaptionsProcess && m_liveCaptionsProcess->state() == QProcess::Running) {
        m_liveCaptionsProcess->terminate();
        if (!m_liveCaptionsProcess->waitForFinished(3000)) {
            m_liveCaptionsProcess->kill();
        }
    }
    
    if (m_captureTimer) {
        m_captureTimer->stop();
    }
    
    m_isRunning = false;
    m_captionWindow = nullptr;
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->statusLabel->setText("Live Captions Stopped");
}

void MainWindow::on_captureTimer_timeout()
{
    if (!m_isRunning) return;
    
    captureCaptionWindow();
}

void MainWindow::captureCaptionWindow()
{
    // Find the Live Captions window
    if (!m_captionWindow) {
        m_captionWindow = FindWindowW(nullptr, L"Live Captions");
        if (!m_captionWindow) {
            m_captionWindow = FindWindowW(nullptr, L"Live captions");
        }
        if (m_captionWindow) {
            qDebug() << "Found Live Captions window";
        }
    }
    
    if (m_captionWindow) {
        // Get window text (captions)
        QString captionText = getWindowText(m_captionWindow);
        if (!captionText.isEmpty() && captionText != ui->captionDisplay->toPlainText()) {
            ui->captionDisplay->setPlainText(captionText);
        }
        
        // Apply window position and style
        applyWindowSettings();
    }
}

QString MainWindow::getWindowText(HWND hwnd)
{
    if (!hwnd) return "";
    
    // Get text from the main window
    wchar_t buffer[1024];
    int length = GetWindowTextW(hwnd, buffer, 1024);
    
    if (length > 0) {
        return QString::fromWCharArray(buffer);
    }
    
    // Try to get text from child windows (the actual caption text might be in a child control)
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child) {
        length = GetWindowTextW(child, buffer, 1024);
        if (length > 0) {
            QString text = QString::fromWCharArray(buffer);
            // Filter out UI text and look for actual captions
            if (!text.contains("Live Captions") && text.length() > 10) {
                return text;
            }
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
    
    return "";
}

void MainWindow::applyWindowSettings()
{
    if (!m_captionWindow) return;
    
    // Get screen dimensions
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    // Window size (approximate Live Captions window size)
    int width = 500;
    int height = 150;
    
    // Calculate position based on selection
    int position = ui->positionCombo->currentData().toInt();
    int x = 0, y = 0;
    
    switch (position) {
    case 1: // Top Left
        x = 10;
        y = 10;
        break;
    case 2: // Top Right
        x = screenGeometry.width() - width - 10;
        y = 10;
        break;
    case 3: // Bottom Left
        x = 10;
        y = screenGeometry.height() - height - 50;
        break;
    case 4: // Bottom Right
        x = screenGeometry.width() - width - 10;
        y = screenGeometry.height() - height - 50;
        break;
    case 5: // Center Top
        x = (screenGeometry.width() - width) / 2;
        y = 10;
        break;
    case 6: // Center Bottom
        x = (screenGeometry.width() - width) / 2;
        y = screenGeometry.height() - height - 50;
        break;
    }
    
    // Set window position
    SetWindowPos(m_captionWindow, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
    
    // Set opacity
    int opacity = ui->opacitySlider->value();
    LONG windowLong = GetWindowLongW(m_captionWindow, GWL_EXSTYLE);
    SetWindowLongW(m_captionWindow, GWL_EXSTYLE, windowLong | WS_EX_LAYERED);
    SetLayeredWindowAttributes(m_captionWindow, 0, (255 * opacity) / 100, LWA_ALPHA);
}

void MainWindow::on_languageCombo_currentIndexChanged(int index)
{
    if (!m_isRunning) return;
    
    QString language = m_languageMap[ui->languageCombo->currentText()];
    qDebug() << "Language changed to:" << language;
    
    // Note: Changing language might require restarting Live Captions
    // or sending specific commands to the process
}

void MainWindow::on_positionCombo_currentIndexChanged(int index)
{
    if (m_isRunning && m_captionWindow) {
        applyWindowSettings();
    }
}

void MainWindow::on_opacitySlider_valueChanged(int value)
{
    ui->opacityLabel->setText(QString("Opacity: %1%").arg(value));
    
    if (m_isRunning && m_captionWindow) {
        applyWindowSettings();
    }
}

void MainWindow::readLiveCaptionsOutput()
{
    if (m_liveCaptionsProcess) {
        QByteArray output = m_liveCaptionsProcess->readAllStandardOutput();
        QByteArray error = m_liveCaptionsProcess->readAllStandardError();
        
        if (!output.isEmpty()) {
            qDebug() << "LiveCaptions stdout:" << output;
        }
        if (!error.isEmpty()) {
            qDebug() << "LiveCaptions stderr:" << error;
        }
    }
}