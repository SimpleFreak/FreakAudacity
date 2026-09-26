#include "main_window.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Главное окно Qt");
    resize(400, 300);

    auto* central = new QWidget(this);
    auto* layout  = new QVBoxLayout(central);

    m_label  = new QLabel("Привет, Qt!", central);
    m_label->setAlignment(Qt::AlignCenter);

    m_button = new QPushButton("Нажми меня", central);

    connect(m_button, &QPushButton::clicked,
        this, &MainWindow::onButtonClicked);

    layout->addWidget(m_label);
    layout->addWidget(m_button);

    setCentralWidget(central);

    m_resourceMonitor = new RESOURCE_MONITOR(this);
}

void MainWindow::onButtonClicked() {
    m_label->setText("Кнопка нажата!");
}
