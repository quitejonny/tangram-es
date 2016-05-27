#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform_qt.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == TANGRAM_REQ_RENDER_EVENT_TYPE) {
        QCoreApplication::postEvent(ui->openGLWidget, new QEvent(event->type()));
        return true;
    }
    return QMainWindow::event(event);
}
