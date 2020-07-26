#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <errno.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string.h>
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
    void on_loginButton_clicked();

private:
    Ui::MainWindow *ui;

    QString infoMessage;//信息
    QString addrMessage;//地址
    QString usernameMessage;//用户名
    QString passwordMessage;//密码

    SOCKET socketData;//数据传输socket
    SOCKET socketConnect;//连接socket
    SOCKET socketControl;//控制传输socket
    struct sockaddr_in serverAddr;//服务器地址
    struct sockaddr_in dataAddr;//数据地址

    char Respond[MAXSIZE];//接收缓存
    char Sendbuf[MAXSIZE];//发送缓存

    bool controlConnect();//控制连接
    bool FTPLogin();//登录
    bool dataConnect();//数据连接
    bool portRequest();
};
#endif // MAINWINDOW_H
