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
    void on_languageCombo_currentIndexChanged(int index);
    void on_positionCombo_currentIndexChanged(int index);
    void on_opacitySlider_valueChanged(int value);
    void on_captureTimer_timeout();
    void readLiveCaptionsOutput();

private:
    Ui::MainWindow *ui;
    
    void setupUI();
    void setupConnections();
    bool findLiveCaptionsPath();
    void startLiveCaptionsProcess();
    void stopLiveCaptionsProcess();
    void captureCaptionWindow();
    QString getWindowText(HWND hwnd);
    
    QProcess *m_liveCaptionsProcess;
    QTimer *m_captureTimer;
    QString m_liveCaptionsPath;
    HWND m_captionWindow;
    bool m_isRunning;
    
    // Language codes for Live Captions
    QMap<QString, QString> m_languageMap;
};
#endif // MAINWINDOW_H