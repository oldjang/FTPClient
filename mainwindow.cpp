#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

/*
控制连接服务器
*/
bool MainWindow::controlConnect(){
    WSADATA wsaData;
    ::WSAStartup( MAKEWORD(2, 2), &wsaData);
    socketControl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    /*
     * 连接信息
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

/*
 * 用户登录
 */

bool MainWindow::FTPLogin()
{
    usernameMessage = ui->usernameText->text();
    passwordMessage = ui->passwordText->text();
    memset(&Sendbuf,0,sizeof(Sendbuf));
    memset(&Respond,0,sizeof(Respond));
    memcpy(Sendbuf,"USER ",strlen("USER "));
    memcpy(Sendbuf+strlen("USER "),usernameMessage.toStdString().data(),strlen(usernameMessage.toStdString().data()));
    memcpy(Sendbuf+strlen("USER ")+strlen(usernameMessage.toStdString().data()),"\r\n",2);
    if(send(socketControl,Sendbuf,strlen(Sendbuf),0) == SOCKET_ERROR){
        qDebug("socket username send error");
        ui->informationText->append(&"socket username send error "[errno]);
        return false;
    }

    ui->informationText->append("socket username send success");
    if(recv(socketControl,Respond,MAXSIZE,0) == SOCKET_ERROR){
        qDebug("socket username receive error");
        ui->informationText->append("socket username receive error");
    }
    ui->informationText->append(Respond);

    ui->informationText->append("password login...");
    memset(&Sendbuf,0,sizeof(Sendbuf));
    memset(&Respond,0,sizeof(Respond));
    memcpy(Sendbuf,"PASS ",strlen("PASS "));
    memcpy(Sendbuf+strlen("PASS "),passwordMessage.toStdString().data(),strlen(passwordMessage.toStdString().data()));
    memcpy(Sendbuf+strlen("PASS ")+strlen(passwordMessage.toStdString().data()),"\r\n",2);
    if(send(socketControl,Sendbuf,strlen(Sendbuf),0)<0){
        qDebug("socket password send error");
         ui->informationText->append("socket password send error");
    }
    ui->informationText->append("socket password send success ");
    if(recv(socketControl,Respond,MAXSIZE,0)<0){
        qDebug("socket password receive error");
        ui->informationText->append("socket password receive error");
    }
    ui->informationText->append(Respond);

    return true;
}

/*
 * 数据连接
 */

bool MainWindow::dataConnect(){
    socketConnect = socket(AF_INET,SOCK_STREAM,0);

    socketData = socket(AF_INET,SOCK_STREAM,0);

    dataAddr.sin_family=AF_INET;
    dataAddr.sin_port = htons(p1*256+p2);
    dataAddr.sin_addr.s_addr=inet_addr(addrMessage.toStdString().data());

    //bind
    if(bind(socketConnect,(struct sockaddr*)&dataAddr,sizeof(dataAddr)) == SOCKET_ERROR){
        qDebug("bind error");
        ui->informationText->append("bind error");
        return false;
    }
    ui->informationText->append("bind client dataAddr success");
    if(listen(socketConnect,3) < 0){
        qDebug("listen error");
        ui->informationText->append("listen error");
        return false;
    }
    return true;
}


bool MainWindow::portRequest()
{
    ui->informationText->append("PORT....");

   //PORT Request
   memset(Sendbuf,0,sizeof(Sendbuf));
   memcpy(Sendbuf,"PORT 127,0,0,1,20,80\r\n",strlen("PORT 127,0,0,1,20,80\r\n"));

   if(send(socketControl,Sendbuf,strlen(Sendbuf),0)<0){
       qDebug("PORT Request Error");
       ui->informationText->append("PORT Request Error");
       return true;
   }
   ui->informationText->append(Sendbuf);
   memset(&Sendbuf,0,sizeof(Sendbuf));
   memset(&Respond,0,sizeof(Respond));
   if(recv(socketControl,Respond,MAXSIZE,0) == SOCKET_ERROR){
       qDebug("receive portmessage error");
       ui->informationText->append("receive portmessage error");
       return false;
   }
   ui->informationText->append(Respond);
   return true;
}


void MainWindow::on_connectButton_clicked()
{
    controlConnect();
}

void MainWindow::on_loginButton_clicked()
{
    FTPLogin();
    ui->informationText->append("---------------------------");
    dataConnect();
    portRequest();
    QMessageBox::information(NULL, "login", "login success\r\nmodal:主动模式(PORT)", QMessageBox::Yes);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_quitButton_clicked()
{
     QUITRequest();
}

/*
 * 断开连接
 */
bool MainWindow::QUITRequest(){

   memset(Sendbuf,0,MAXSIZE);
   memset(Respond,0,MAXSIZE);
   memcpy(Sendbuf,"QUIT\r\n",strlen("QUIT\r\n"));

      //返回值< 0 -> 客户端quit请求发送失败
  if(send(socketControl,Sendbuf,strlen(Sendbuf),0)<0){
      qDebug("quit ftp request error");
       exit(0);
   }
   ui->informationText->append("quit ftp send success");

    //返回值< 0 -> ftp服务器响应接受失败
    if(recv(socketControl,Respond,MAXSIZE,0)<0){
         qDebug("quit ftp receive error");
    }
     ui->informationText->append("quit success");
     ui->informationText->append(Respond);
    if(QMessageBox::information(NULL,"info","quit ftpserver\r\n success",QMessageBox::Yes)){
       this->close();

    }
   return true;
}
