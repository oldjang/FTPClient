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
 * 获取列表
 */

bool MainWindow::list()
{
    int sin_size;
    int nRead;
    SOCKET newsockfd, data_sockfd;
    struct sockaddr_in their_add;
    char data_buf[DATA_BUFSIZE];

    //创建数据连接
    newsockfd = ftpBasic.createSocket();
    //构建命令报文并发送至服务器
    if(!ftpBasic.sendCMD(socketControl,"LIST\r\n"))
    {
        ui->informationText->append("list error");
        closesocket(newsockfd);
        return false;
    }
    sin_size = sizeof(struct sockaddr_in);
    //接受服务器的数据连接请求
    if ((data_sockfd = accept(newsockfd, (struct sockaddr*)&their_add, &sin_size)) == INVALID_SOCKET)
    {
        ui->informationText->append("list error");
        closesocket(newsockfd);
        return false;
    }

    QString tmp("..\n");
    //每次读到多少数据就显示多少，直到数据连接断开
    while (true)
    {
        nRead = recv(data_sockfd, data_buf, DATA_BUFSIZE - 1, 0);
        if (nRead == SOCKET_ERROR)
        {
            ui->informationText->append("list error");
            closesocket(data_sockfd);
            closesocket(newsockfd);
            exit(1);
        }

        if (nRead == 0)//数据读取结束
            break;
        tmp.append(data_buf);
    }

    QStringList strList=tmp.split('\n');
    int nCount = strList.size();
    ItemModel = new QStandardItemModel(this);

    for(int i = 0; i < nCount; i++)
    {
        QString string = static_cast<QString>(strList.at(i));
        QStandardItem *item = new QStandardItem(string);
        ItemModel->appendRow(item);
    }
    ui->fileList->setModel(ItemModel);
    ui->fileList->adjustSize();
    connect(ui->fileList,SIGNAL(clicked(QModelIndex)),this,SLOT(showClick(QModelIndex)));

    linger m_sLinger;

    m_sLinger.l_onoff = 1;  // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)

    m_sLinger.l_linger = 0; // (容许逗留的时间为0秒)

    setsockopt(data_sockfd,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(linger));
    setsockopt(newsockfd,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(linger));


    closesocket(data_sockfd);
    closesocket(newsockfd);


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
    if(ui->downloadFilenamText->text() != NULL) {


        int sin_size;
        SOCKET newsockfd, data_sockfd;
        struct sockaddr_in their_add;

        newsockfd = ftpBasic.createSocket();

        //PORT Request
        memset(Sendbuf,0,sizeof(Sendbuf));
        memcpy(Sendbuf, "PORT 127,0,0,1,20,80\r\n", strlen("PORT 127,0,0,1,20,80\r\n"));

        if(send(socketControl, Sendbuf, strlen(Sendbuf),0)<0) {
            qDebug("PORT Request Error");
            ui->informationText->append("PORT Request Error");
            exit(0);
        }
        ui->informationText->append(Sendbuf);
        memset(Respond, 0, sizeof(Respond));
        if(recv(socketControl, Respond, MAXSIZE, 0) < 0) {
            qDebug("receive poertmessage error");
            exit(0);
        }
        ui->informationText->append(Respond);
        ui->informationText->append("-----------------------------------");
        //TYPE
        if(!ftpBasic.sendCMD(socketControl,"TYPE I\r\n")) {
            qDebug("send List Request error");
            ui->informationText->append("send TYPE Request error");
            return false;
        }
        ui->informationText->append("send TYPE Request success");
        if(!ftpBasic.readResponse(socketControl)) {
            qDebug("socket TYPE receive error");
            ui->informationText->append("socket TYPE receive error");
            return false;
        }

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



        sin_size = sizeof(struct sockaddr_in);
        //接受服务器的数据连接请求
        if ((data_sockfd = accept(newsockfd, (struct sockaddr*)&their_add, &sin_size)) == INVALID_SOCKET)
        {
            ui->informationText->append("RETR error");
            closesocket(newsockfd);
            return false;
        }

        char src[256];

        std::string filename=("E:\\file\\"+ui->downloadFilenamText->text()).toStdString();

        qDebug(filename.c_str());

        std::ofstream file(filename.c_str(), std::ios::binary);
//        throw std::system_error(errno, std::system_category(), "fail to open")
        try
        {
            throw std::system_error(errno, std::system_category(), "fail to open")

        } catch(std::system_error &e)
        {
            qDebug() << e.what() << ' ' << e.code().value() << ' ' << e.code().message();
        }
        ;
        if(file.good())
            qDebug("good");
        else
            qDebug("bad");

        memset(src, 0, sizeof(src));
        int cnt;
        while((cnt = recv(data_sockfd, src, 256, 0)) > 0)
        {
            qDebug("hhh");
            file.write(src,cnt);
            qDebug(src);
            memset(src, 0, sizeof(src));
        }
        QMessageBox::information(NULL, "success", "file success", QMessageBox::Yes);
        file.close();

    }
    else {
        QMessageBox::warning(NULL, "error", "请输入文件名",QMessageBox::Yes);
    }
    return true;
}

void MainWindow::showClick(QModelIndex index)
{
    QString strTemp;
    strTemp = index.data().toString();

    QStringList listTemp=strTemp.split(' ');

    QString fileName=listTemp[listTemp.size()-1];

    if(fileName!=".."&&strTemp[0]!='d')
        ui->downloadFilenamText->setText(fileName);
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

MainWindow::~MainWindow()
{
    delete ui;
}



