#include "mainwindow.h"

FTPBasic::FTPBasic()
{

}
bool FTPBasic::readResponse(SOCKET fd,QString * message)
{
    memset(responseBuf,0,MAXSIZE);
    if(recv(fd,responseBuf,MAXSIZE,0) == SOCKET_ERROR){
        qDebug("receive error");
        informationText->append("receive error");
        closesocket(fd);
        return false;
    }
    informationText->append("Receive successfully");
    informationText->append(responseBuf);
    informationText->append("--------------------------");
    if(message!=nullptr)
        *message=QLatin1String(responseBuf);
    return true;
}



bool FTPBasic::sendCMD(SOCKET fd,QString sendData)
{
    memset(sendBuf,0,MAXSIZE);
    memcpy(sendBuf,sendData.toStdString().data(),strlen(sendData.toStdString().data()));
    if(send(fd,sendBuf,strlen(sendBuf),0) == SOCKET_ERROR){
        qDebug("socket username send error");
        informationText->append(&"socket username send error "[errno]);
        closesocket(fd);
        return false;
    }
    qDebug("%s",sendData.toStdString().data());
    informationText->append("Send command successfully");
    informationText->append("--------------------------");
    return true;
}


SOCKET FTPBasic::createSocket(QString addr,int port)
{
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in Addr;
    memset(&Addr,0,sizeof(Addr));
    Addr.sin_family = AF_INET;
    Addr.sin_port = htons(port);
    Addr.sin_addr.s_addr=inet_addr(addr.toStdString().data());

    if(inet_addr(addr.toStdString().data())==INADDR_NONE){
        //测试ip地址是否正确
        informationText->append("inet_pton error");
        qDebug("inet_pton error");
        return false;
    }

    informationText->append("connect to server...");

    if(::connect(sockfd,(SOCKADDR*)&Addr,sizeof(SOCKADDR)) == SOCKET_ERROR){
        //连接服务器
        qDebug("connect error: ");
        informationText->append(&"connect error: "[errno]);
        return false;
    }
    informationText->append("connect success");

    return sockfd;
}

void FTPBasic::getInformationText(QTextBrowser *InformationText)
{
    informationText = InformationText;
}
