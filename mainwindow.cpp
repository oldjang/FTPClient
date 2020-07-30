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
    ftpBasic.getInformationText(ui->informationText);
}

/*
控制连接服务器
*/
bool MainWindow::controlConnect(){
    WSADATA wsaData;
    ::WSAStartup( MAKEWORD(2, 2), &wsaData);

    /*
     * 连接信息
     */
    addrMessage = ui->addressText->text();//找出ip地址

    socketControl = ftpBasic.createSocket(addrMessage,CONNECTPORT);

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


bool MainWindow::turnToPasvMode()
{
    ui->informationText->append("PASV....");

    //PORT Request
    if(!ftpBasic.sendCMD(socketControl,"PASV\r\n"))
    {
        qDebug("PASV Request Error");
        ui->informationText->append("PORT Request Error");
        return false;
    }

    QString lastResponse;

    if(!ftpBasic.readResponse(socketControl,&lastResponse))
    {
        qDebug("receive portmessage error");
        ui->informationText->append("receive portmessage error");
        return false;
    }
    lastResponse.remove(QChar(')'), Qt::CaseInsensitive);
    QStringList resps = lastResponse.split(',');
    bool ok;
    int dataPort = resps[4].toInt(&ok,10) * 256 + resps[5].toInt(&ok,10);
    qDebug("%s",qPrintable(lastResponse));
    socketData = ftpBasic.createSocket(addrMessage,dataPort);
    return true;
}

/*
 * 获取列表
 */

bool MainWindow::list()
{
    int nRead;
    char data_buf[DATA_BUFSIZE];

    turnToPasvMode();

    //构建命令报文并发送至服务器
    if(!ftpBasic.sendCMD(socketControl,"LIST\r\n"))
    {
        ui->informationText->append("list error");
        return false;
    }

    QString tmp("..\n");
    //每次读到多少数据就显示多少，直到数据连接断开
    while (true)
    {
        nRead = recv(socketData, data_buf, DATA_BUFSIZE - 1, 0);
        if (nRead == SOCKET_ERROR)
        {
            ui->informationText->append("list error");
            closesocket(socketData);
            exit(1);
        }

        if (nRead == 0)//数据读取结束
            break;
        tmp.append(data_buf);
    }

    listMessage = tmp;

    ui->fileList->setText(tmp);

    closesocket(socketData);


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

bool MainWindow::download()
{
    if(!listMessage.contains(ui->downloadFilenamText->text(),Qt::CaseSensitive))
    {
        QMessageBox::warning(NULL, "error", "文件路径错误",QMessageBox::Yes);
        //报错
        return false;
    }
    if(ui->downloadFilenamText->text() != NULL) {

        turnToPasvMode();

        //RETR
        if(!ftpBasic.sendCMD(socketControl,"RETR "+ui->downloadFilenamText->text()+"\r\n")) {
            ui->informationText->append("send retr Request error");
            return false;
        }
        ui->informationText->append("send RETR Request success");

        if(!ftpBasic.readResponse(socketControl)) {
            ui->informationText->append("socket RETR receive error");
            return false;
        }
        ui->informationText->append(Respond);


        char src[256];

        std::string filename=("E:\\file\\"+ui->downloadFilenamText->text()).toStdString();

        FILE *fd=fopen(filename.data(),"wb");

        int cnt;
        while ((cnt = recv(socketData, src, 256, 0)) > 0)
            fwrite(src, sizeof(char), 256, fd);
        QMessageBox::information(NULL, "success", "file success", QMessageBox::Yes);
        closesocket(socketData);
        //file.close();

    }
    else {
        QMessageBox::warning(NULL, "error", "请输入文件名",QMessageBox::Yes);
    }
    return true;
}


bool MainWindow::upload(char *srcPath)
{
    char buf[MAXSIZE];
    int count;
    FILE *fd;
    char *name;

    QFileInfo fileinfo(srcPath);
    if(!fileinfo.isFile())//判断文件路径是否 正确
    {
        QMessageBox::warning(NULL, "error", "文件路径错误",QMessageBox::Yes);
        //报错
        return false;
    }
    QByteArray ba=fileinfo.fileName().toLocal8Bit();
    name=ba.data();

    fd=fopen(srcPath,"rb");
    if(fd==NULL)
    {
        QMessageBox::warning(NULL, "error", "无法访问文件",QMessageBox::Yes);
        return false;
    }
    turnToPasvMode();
    if(!ftpBasic.sendCMD(socketControl,"STOR "+fileinfo.fileName()+"\r\n")) {
        ui->informationText->append("send stor Request error");
        return false;
    }
    ui->informationText->append("send STOR Request success");
    if(!ftpBasic.readResponse(socketControl)) {
        ui->informationText->append("socket STOR receive error");
        return false;
    }
    ui->informationText->append(Respond);

    memset(buf,0,sizeof(buf));

    while(true)
    {
        count=fread(buf,sizeof(char),256,fd);
        send(socketData,buf,count,0);
        if(count<256)
            break;
    }
    fclose(fd);
    closesocket(socketData);
    return true;
}

bool MainWindow::cd(QString name)
{
    if((!listMessage.contains(name,Qt::CaseSensitive))&&(name!=".."))
    {
        QMessageBox::warning(NULL, "error", "文件路径错误",QMessageBox::Yes);
        //报错
        return false;
    }
    if(!ftpBasic.sendCMD(socketControl,"CWD "+name+"\r\n"))
    {
        ui->informationText->append("send cd Request error");
        return false;
    }
    ui->informationText->append("send cd Request success");

    if(!ftpBasic.readResponse(socketControl)) {
        ui->informationText->append("socket cd receive error");
        return false;
    }

    list();
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
    QMessageBox::information(NULL, "login", "login success\r\nmodal:主动模式(PORT)", QMessageBox::Yes);

}

void MainWindow::on_listButton_clicked()
{
    list();
}

void MainWindow::on_quitButton_clicked()
{
    QUITRequest();
}

void MainWindow::on_downloadButton_clicked()
{
    download();
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

    if(upload(path)==true)
    {
        ui->informationText->append("upload complete");
    }
}

void MainWindow::on_cdButton_clicked()
{
    cd(ui->folderText->text());
}

void MainWindow::on_returnButton_clicked()
{
    cd("..");
}

MainWindow::~MainWindow()
{
    delete ui;
}
