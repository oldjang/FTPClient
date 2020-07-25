#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <errno.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <QtWidgets/QMessageBox>

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
    void on_connectButton_clicked();

private:
    Ui::MainWindow *ui;

    QString infoMessage;//信息
    QString addrMessage;//地址

    SOCKET socketControl;//控制传输socket
    struct sockaddr_in serverAddr;//服务器地址

    char Respond[MAXSIZE];//接收缓存
    char Sendbuf[MAXSIZE];//发送缓存

    bool controlConnect();
};
#endif // MAINWINDOW_H
