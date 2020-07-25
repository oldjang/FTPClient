#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#define MAXSIZE 1024
#define CONNECTPORT 21
#define p1 20
#define p2 80

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
    void on_clicked_connectButton();

private:
    Ui::MainWindow *ui;
    bool controlConnect();
};
#endif // MAINWINDOW_H
