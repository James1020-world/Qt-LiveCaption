#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QProcess>
#include <QTimer>
#include <windows.h>
#include <UIAutomation.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_positionCombo_currentIndexChanged(int index);
    void on_opacitySlider_valueChanged(int value);
    void on_captureTimer_timeout();
    void on_openSettingsButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::MainWindow *ui;
    
    void setupUI();
    void setupConnections();
    void launchLiveCaptions();
    void closeLiveCaptions();
    void captureCaptionWindow();
    QString getCaptionTextViaUIA();
    HWND findLiveCaptionsWindow();
    bool isLiveCaptionsRunning();
    void updateCaptionDisplay(const QString &text);
    
    QTimer *m_captureTimer;
    HWND m_captionWindow;
    bool m_isRunning;
    QString m_lastCaptionText;
    
    IUIAutomation *m_automation;
    IUIAutomationElement *m_captionElement;
};
#endif // MAINWINDOW_H