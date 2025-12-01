#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <windows.h>
#include <tlhelp32.h>
#include <comdef.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_captureTimer(nullptr)
    , m_captionWindow(nullptr)
    , m_isRunning(false)
    , m_automation(nullptr)
    , m_captionElement(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, 
                                  CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), 
                                  (void**)&m_automation);
    
    if (FAILED(hr)) {
        qWarning() << "Failed to initialize UI Automation";
    }
    
    if (isLiveCaptionsRunning()) {
        ui->statusLabel->setText("Live Captions is already running");
        ui->startButton->setText("Connect to Live Captions");
    }
}

MainWindow::~MainWindow()
{
    if (m_captureTimer) {
        m_captureTimer->stop();
    }
    
    if (m_captionElement) {
        m_captionElement->Release();
    }
    
    if (m_automation) {
        m_automation->Release();
    }
    
    CoUninitialize();
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("Live Captions Controller - Professional Edition");
    setMinimumSize(800, 600);
    
    ui->positionCombo->addItem("Bottom Center (Default)", 0);
    ui->positionCombo->addItem("Top Center", 1);
    ui->positionCombo->addItem("Bottom Left", 2);
    ui->positionCombo->addItem("Bottom Right", 3);
    
    ui->opacitySlider->setValue(100);
    ui->stopButton->setEnabled(false);
    
    ui->captionDisplay->setReadOnly(true);
}

void MainWindow::setupConnections()
{
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::on_stopButton_clicked);
    connect(ui->positionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::on_positionCombo_currentIndexChanged);
    connect(ui->opacitySlider, &QSlider::valueChanged, 
            this, &MainWindow::on_opacitySlider_valueChanged);
    connect(ui->openSettingsButton, &QPushButton::clicked, this, &MainWindow::on_openSettingsButton_clicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::on_clearButton_clicked);
}

bool MainWindow::isLiveCaptionsRunning()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);
    
    bool found = false;
    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            QString processName = QString::fromWCharArray(processEntry.szExeFile);
            if (processName.contains("LiveCaptions", Qt::CaseInsensitive)) {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &processEntry));
    }
    
    CloseHandle(snapshot);
    return found;
}

HWND MainWindow::findLiveCaptionsWindow()
{
    HWND hwnd = FindWindowW(nullptr, L"Live Captions");
    if (!hwnd) {
        hwnd = FindWindowW(nullptr, L"Live captions");
    }
    if (!hwnd) {
        hwnd = FindWindowW(L"Windows.UI.Core.CoreWindow", nullptr);
        if (hwnd) {
            wchar_t title[256];
            GetWindowTextW(hwnd, title, 256);
            QString titleStr = QString::fromWCharArray(title);
            if (!titleStr.contains("caption", Qt::CaseInsensitive)) {
                hwnd = nullptr;
            }
        }
    }
    return hwnd;
}

void MainWindow::on_startButton_clicked()
{
    if (m_isRunning) {
        return;
    }
    
    if (isLiveCaptionsRunning()) {
        ui->statusLabel->setText("Connecting to Live Captions...");
    } else {
        launchLiveCaptions();
        ui->statusLabel->setText("Launching Live Captions...");
        QThread::msleep(2000); // Wait for launch
    }
    
    m_isRunning = true;
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    
    if (!m_captureTimer) {
        m_captureTimer = new QTimer(this);
        connect(m_captureTimer, &QTimer::timeout, this, &MainWindow::on_captureTimer_timeout);
    }
    m_captureTimer->start(300); // Check every 300ms for smooth updates
    
    ui->statusLabel->setText("Live Captions Active - Monitoring");
}

void MainWindow::on_stopButton_clicked()
{
    if (m_captureTimer) {
        m_captureTimer->stop();
    }
    
    m_isRunning = false;
    m_captionWindow = nullptr;
    
    if (m_captionElement) {
        m_captionElement->Release();
        m_captionElement = nullptr;
    }
    
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->statusLabel->setText("Monitoring Stopped (Live Captions still running)");
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Close Live Captions",
        "Do you want to close Live Captions application?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        closeLiveCaptions();
        ui->statusLabel->setText("Live Captions Closed");
    }
}

void MainWindow::launchLiveCaptions()
{
    QProcess::startDetached("powershell.exe", 
        QStringList() << "-Command" << "Start-Process" << "ms-settings:easeofaccess-closedcaptioning");
    
    ShellExecuteW(nullptr, L"open", L"ms-settings:easeofaccess-closedcaptioning", 
                  nullptr, nullptr, SW_SHOWNORMAL);
}

void MainWindow::closeLiveCaptions()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }
    
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            QString processName = QString::fromWCharArray(processEntry.szExeFile);
            if (processName.contains("LiveCaptions", Qt::CaseInsensitive)) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
                break;
            }
        } while (Process32NextW(snapshot, &processEntry));
    }
    
    CloseHandle(snapshot);
}

void MainWindow::on_captureTimer_timeout()
{
    if (!m_isRunning) return;
    
    captureCaptionWindow();
}

void MainWindow::captureCaptionWindow()
{
    if (!m_captionWindow) {
        m_captionWindow = findLiveCaptionsWindow();
        if (m_captionWindow) {
            qDebug() << "Found Live Captions window";
            ui->statusLabel->setText("Live Captions Active - Capturing Text");
        } else {
            return;
        }
    }
    
    QString captionText = getCaptionTextViaUIA();
    
    if (!captionText.isEmpty()) {
        updateCaptionDisplay(captionText);
    }
}

QString MainWindow::getCaptionTextViaUIA()
{
    if (!m_automation || !m_captionWindow) {
        return QString();
    }
    
    IUIAutomationElement *rootElement = nullptr;
    HRESULT hr = m_automation->ElementFromHandle(m_captionWindow, &rootElement);
    
    if (FAILED(hr) || !rootElement) {
        return QString();
    }
    
    BSTR name = nullptr;
    rootElement->get_CurrentName(&name);
    
    QString result;
    if (name) {
        result = QString::fromWCharArray(name);
        SysFreeString(name);
    }
    
    if (result.isEmpty()) {
        IUIAutomationTextPattern *textPattern = nullptr;
        hr = rootElement->GetCurrentPatternAs(UIA_TextPatternId, __uuidof(IUIAutomationTextPattern), 
                                               (void**)&textPattern);
        if (SUCCEEDED(hr) && textPattern) {
            IUIAutomationTextRange *textRange = nullptr;
            hr = textPattern->get_DocumentRange(&textRange);
            if (SUCCEEDED(hr) && textRange) {
                BSTR text = nullptr;
                textRange->GetText(-1, &text);
                if (text) {
                    result = QString::fromWCharArray(text);
                    SysFreeString(text);
                }
                textRange->Release();
            }
            textPattern->Release();
        }
    }
    
    rootElement->Release();
    return result;
}

void MainWindow::updateCaptionDisplay(const QString &text)
{
    if (text == m_lastCaptionText) {
        return;
    }
    
    m_lastCaptionText = text;
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedText = QString("[%1] %2\n").arg(timestamp, text);
    
    ui->captionDisplay->append(formattedText);
    
    QTextCursor cursor = ui->captionDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->captionDisplay->setTextCursor(cursor);
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
void
 MainWindow::on_positionCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->statusLabel->setText("Position: Change via Windows Settings");
}

void MainWindow::on_opacitySlider_valueChanged(int value)
{
    ui->opacityLabel->setText(QString("Opacity: %1%").arg(value));
}

void MainWindow::on_openSettingsButton_clicked()
{
    ShellExecuteW(nullptr, L"open", L"ms-settings:easeofaccess-closedcaptioning", 
                  nullptr, nullptr, SW_SHOWNORMAL);
}

void MainWindow::on_clearButton_clicked()
{
    ui->captionDisplay->clear();
    m_lastCaptionText.clear();
    ui->statusLabel->setText("Caption history cleared");
}
