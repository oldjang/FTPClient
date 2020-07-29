#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ftpBasic.getInformationText(ui->informationText);
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

    if(!ftpBasic.readResponse(socketControl))
        return false;

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

    if(ftpBasic.sendCMD(socketControl,"USER "+usernameMessage+"\r\n"))
        ui->informationText->append("socket username send success");
    else{
        ui->informationText->append("socket username send error ");
        return false;
    }
    if(!ftpBasic.readResponse(socketControl)){
        qDebug("socket username receive error");
        ui->informationText->append("socket username receive error");
        return false;
    }

    ui->informationText->append("password login...");
    if(ftpBasic.sendCMD(socketControl,"PASS "+passwordMessage+"\r\n"))
        ui->informationText->append("socket password send success ");
    else{
        ui->informationText->append("socket password send error ");
        return false;
    }
    if(!ftpBasic.readResponse(socketControl)){
        qDebug("socket password receive error");
        ui->informationText->append("socket passworde receive error");
        return false;
    }

    return true;
}

/*
 * 发送端口
 */


bool MainWindow::portRequest()
{
    ui->informationText->append("PORT....");

    //PORT Request
    if(!ftpBasic.sendCMD(socketControl,"PORT 127,0,0,1,20,80\r\n"))
    {
        qDebug("PORT Request Error");
        ui->informationText->append("PORT Request Error");
        return false;
    }

    if(!ftpBasic.readResponse(socketControl))
    {
        qDebug("receive portmessage error");
        ui->informationText->append("receive portmessage error");
        return false;
    }
    return true;
}


/*
 * 断开连接
 */
bool MainWindow::QUITRequest(){

    if(!ftpBasic.sendCMD(socketControl,"QUIT\r\n"))
    {
        qDebug("quit ftp request error");
        ui->informationText->append("quit ftp request error");
        return false;
    }
    ui->informationText->append("quit ftp send success");

    //返回值< 0 -> ftp服务器响应接受失败
    if(!ftpBasic.readResponse(socketControl)){
        qDebug("quit ftp receive error");
        ui->informationText->append("quit ftp receive error");
        return false;
    }
    ui->informationText->append("quit success");
    if(QMessageBox::information(NULL,"info","quit ftpserver\r\n success",QMessageBox::Yes)){
        this->close();

    }
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
    portRequest();
    QMessageBox::information(NULL, "login", "login success\r\nmodal:主动模式(PORT)", QMessageBox::Yes);

}

void MainWindow::on_quitButton_clicked()
{
    QUITRequest();
}


MainWindow::~MainWindow()
{
    delete ui;
}



