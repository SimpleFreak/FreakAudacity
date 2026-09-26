// resource_monitor_windows.hpp
#ifndef RESOURCE_MONITOR_WINDOWS_HPP
#define RESOURCE_MONITOR_WINDOWS_HPP

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <chrono>

class ResourceMonitorWindows : public QObject {
    Q_OBJECT
public:
    explicit ResourceMonitorWindows(QObject* parent = nullptr) : QObject(parent) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ResourceMonitorWindows::update);
        update();
        m_timer->start(1000);
    }

private slots:
    void update() {
        double cpuUsage = getCpuUsage();
        long memoryUsage = getMemoryUsage();
        int threadCount = getThreadCount();

        qDebug() << "=== Monitoring resources (Windows) ===";
        qDebug() << "Load CPU:" << cpuUsage << "%";
        qDebug() << "Usage memory (Working Set):" << memoryUsage << "KB";
        qDebug() << "Count threads:" << threadCount;
        qDebug() << "-------------------------------------";

        QThread::sleep(1);
    }

private:
    QTimer* m_timer;
    ULONGLONG m_prevProcessTime = 0;
    ULONGLONG m_prevSystemTime = 0;

    double getCpuUsage() {
        FILETIME createTime, exitTime, kernelTime, userTime;
        if (!GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime)) {
            return 0.0;
        }

        ULARGE_INTEGER kTime, uTime;
        kTime.LowPart = kernelTime.dwLowDateTime;
        kTime.HighPart = kernelTime.dwHighDateTime;
        uTime.LowPart = userTime.dwLowDateTime;
        uTime.HighPart = userTime.dwHighDateTime;

        ULONGLONG processTime = kTime.QuadPart + uTime.QuadPart;
        ULONGLONG systemTime = GetTickCount64();

        if (m_prevProcessTime == 0) {
            m_prevProcessTime = processTime;
            m_prevSystemTime = systemTime;
            return 0.0;
        }

        ULONGLONG processDelta = processTime - m_prevProcessTime;
        ULONGLONG systemDelta = systemTime - m_prevSystemTime;

        m_prevProcessTime = processTime;
        m_prevSystemTime = systemTime;

        if (systemDelta == 0) return 0.0;

        int numCores = QThread::idealThreadCount();
        double usage = (static_cast<double>(processDelta) / (systemDelta * 10000.0)) * numCores * 100.0;
        return usage;
    }

    long getMemoryUsage() {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize / 1024;
        }
        return 0;
    }

    int getThreadCount() {
        DWORD processId = GetCurrentProcessId();
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        int count = 0;

        if (Thread32First(snapshot, &te)) {
            do {
                if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
                    if (te.th32OwnerProcessID == processId) {
                        count++;
                    }
                }
            } while (Thread32Next(snapshot, &te));
        }
        CloseHandle(snapshot);
        return count;
    }
};

#endif
