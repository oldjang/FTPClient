#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextCodec>
#include <QFileDialog>

#pragma execution_character_set(“utf-8”)

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
//作用: send发送命令，并返回recv结果
//参数: 命令，命令返回码-命令返回描述，命令返回字节数
//返回值: 0 表示发送成功  -1表示发送失败
int MainWindow::sendcmd_re(char *cmd,char *re_buf, SSIZE_T *len)
{
    char buf[MAXSIZE];
    SSIZE_T r_len;
    if(send(socketControl,cmd,strlen(cmd),0)<0)
    {
        qDebug("cmd send error!");
        return -1;
    }
    r_len=recv(socketControl,buf,MAXSIZE,0);
    if(r_len<1)
    {
        qDebug("No response!");
        ui->informationText->append("recv error");
        ui->informationText->append(buf);
        return -1;
    }
    buf[r_len]=0;
    if(NULL!=len)
        *len=r_len;
    if(NULL!=re_buf)
        sprintf(re_buf,"%s",buf);
    return 0;
}

int MainWindow::sendcmd(char *cmd)
{
    char buf[MAXSIZE];
    int result;
    SSIZE_T len;
    ui->informationText->append("FTP Client: ");
    ui->informationText->append(cmd);
    result=sendcmd_re(cmd,buf,&len);
    ui->informationText->append("FTP Server: ");
    ui->informationText->append(buf);
    if(0==result)
    {
        sscanf_s(buf,"%d",&result);
    }
    return result;
}


MainWindow::~MainWindow()
{
    delete ui;
}
//设置ftp传输类型
int MainWindow::ftp_transtype(char mode)
{
    char buf[MAXSIZE];
    sprintf(buf,"TYPE %c\r\n",mode);
    if(sendcmd(buf)!=0)
    {
        return -1;
    }
    else return 0;
}

int MainWindow::ftp_upload(char *srcPath)
{
    char buf[MAXSIZE];
    int result;
    int send_res;
    int count;
    struct sockaddr_in their_addr;
    FILE *fd;

    fd=fopen(srcPath,"rb");
    if(fd==NULL)
    {
        ui->informationText->append("open file error");
        return -1;
    }

    if(!portRequest())
    {
        ui->informationText->append("portRequest error");
        return -1;
    }
    ftp_transtype('I');
    ui->informationText->append("TYPE I changed");
    memset(buf,0,sizeof(buf));
    sprintf(buf,"STOR %s\r\n",srcPath);
    send_res=sendcmd(buf);
    if(send_res!=0)
    {
        ui->informationText->append("STOR error");
        return -1;
    }
    if(!dataConnect())
    {
        ui->informationText->append("bind error");
        return -1;
    }
    int sin_size=sizeof(struct sockaddr_in);
    if(socketData=accept(socketConnect,(struct sockaddr *)&their_addr,&sin_size)==INVALID_SOCKET)
    {
        ui->informationText->append("dataConnect error");
        closesocket(socketConnect);
        return -1;
    }
    memset(buf,0,sizeof(buf));
    while(true)
    {
        count=fread(buf,sizeof(char),MAXSIZE,fd);
        send(socketData,buf,count,0);
        if(count<MAXSIZE)
            break;
    }
    fclose(fd);
    return 0;
}

void MainWindow::on_listButton_clicked()
{

}

void MainWindow::on_fileChooseButton_clicked()
{
    QString srcPath=ui->downloadFileText->text();
    ui->downloadFileText->setText(QFileDialog::getOpenFileName(this,srcPath));

}

void MainWindow::on_uploadButton_clicked()
{

     QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
     char *path;
     QString srcPath=ui->downloadFileText->text();
     QByteArray ba=srcPath.toLocal8Bit();
     path=ba.data();
     QFileInfo fileinfo(srcPath);
     if(!fileinfo.isFile())//判断文件路径是否 正确
     {
         ui->informationText->append("the path is error");
         //报错
         return;
     }
     if(ftp_upload(path)==0)
     {
         ui->informationText->append("upload complete");
     }
}
