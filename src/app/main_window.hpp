#ifndef __MAIN_WINDOW_HPP__
#define __MAIN_WINDOW_HPP__

#include <QMainWindow>

class QLabel;
class QPushButton;

#ifdef Q_OS_WIN
#include "resource_monitor_windows.hpp"
#define RESOURCE_MONITOR ResourceMonitorWindows
#endif

class MainWindow : public QMainWindow {
private:
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onButtonClicked();

private:
    QLabel* m_label;
    QPushButton* m_button;
    RESOURCE_MONITOR* m_resourceMonitor;
};

#endif
