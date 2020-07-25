#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

bool MainWindow::controlConnect(){
    WSADATA wsaData;
    ::WSAStartup( MAKEWORD(2, 2), &wsaData);
    socketControl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(socketControl== INVALID_SOCKET){
           infoMessage = &"creatsocket error:"[errno];
           qDebug("creatsocket error ");
           ui->informationText->append("creat socket error");
           return true;
        }

    /*
    连接信息
    */
    addrMessage = ui->addressText->text();//找出ip地址
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(CONNECTPORT);
    serverAddr.sin_addr.s_addr=inet_addr(addrMessage.toStdString().data());

    if(inet_addr(addrMessage.toStdString().data())==INADDR_NONE){
            //测试ip地址是否正确
            ui->informationText->append("inet_pton error");
            qDebug("inet_pton error");
            return false;
        }

    ui->informationText->append("connect to server...");

    if(::connect(socketControl,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR)) == SOCKET_ERROR){
            //连接服务器
            qDebug("connect error: ");
            ui->informationText->append(&"connect error: "[errno]);
            return false;
        }
    ui->informationText->append("connect success");

    if(recv(socketControl,Respond,MAXSIZE,0) == SOCKET_ERROR){
        qDebug("receive error");
        ui->informationText->append("receive error");
        exit(0);
    }
    ui->informationText->append("receive success");
    ui->informationText->append(Respond);
    ui->informationText->append("--------------------------");
    QMessageBox::information(NULL,"info","contorlConnect to ftpserver\r\n success",QMessageBox::Yes);

    return true;
}

void MainWindow::on_connectButton_clicked()
{
    controlConnect();
}

MainWindow::~MainWindow()
{
    delete ui;
}

