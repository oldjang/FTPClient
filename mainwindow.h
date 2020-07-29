#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <errno.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <QtWidgets/QMessageBox>
#include <QTextBrowser>


#define MAXSIZE 1024
#define CONNECTPORT 21
#define p1 20
#define p2 80

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FTPBasic
{
public:
    FTPBasic();
    bool readResponse(SOCKET fd);
    bool sendCMD(SOCKET fd,QString sendData);
    SOCKET createSocket();
    void getInformationText(QTextBrowser* InformationText);

private:
    char sendBuf[MAXSIZE];
    char responseBuf[MAXSIZE];
    QTextBrowser* informationText;

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_loginButton_clicked();

    void on_quitButton_clicked();

private:
    Ui::MainWindow *ui;

    FTPBasic ftpBasic;

    QString infoMessage;//信息
    QString addrMessage;//地址
    QString usernameMessage;//用户名
    QString passwordMessage;//密码

    SOCKET socketControl;//控制传输socket
    struct sockaddr_in serverAddr;//服务器地址
    struct sockaddr_in dataAddr;//数据地址

    char Respond[MAXSIZE];//接收缓存
    char Sendbuf[MAXSIZE];//发送缓存

    bool controlConnect();//控制连接
    bool FTPLogin();//登录
    bool portRequest();
    bool QUITRequest();
};
#endif // MAINWINDOW_H
