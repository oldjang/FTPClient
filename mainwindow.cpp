#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

bool MainWindow::controlConnect(){
    return true;
}

void MainWindow::on_clicked_connectButton()
{
    controlConnect();
}

MainWindow::~MainWindow()
{
    delete ui;
}

