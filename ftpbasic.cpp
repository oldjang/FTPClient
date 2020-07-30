#include "mainwindow.h"

FTPBasic::FTPBasic()
{

}
bool FTPBasic::readResponse(SOCKET fd)
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
    informationText->append("Send command successfully");
    informationText->append("--------------------------");
    return true;
}


SOCKET FTPBasic::createSocket()
{
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in dataAddr;
    dataAddr.sin_family=AF_INET;
    dataAddr.sin_port = htons(p1*256+p2);
    dataAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    memset(&(dataAddr.sin_zero), 0, sizeof(dataAddr.sin_zero));

    //绑定
    if (bind(sockfd, (struct sockaddr *)&dataAddr, sizeof(struct sockaddr)) == SOCKET_ERROR)
    {
        informationText->append("bind error");
        closesocket(sockfd);
        WSACleanup();
    }
    int nREUSEADDR = 1;

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&nREUSEADDR,sizeof(int));

    //侦听数据连接请求
    if (listen(sockfd, 3) == SOCKET_ERROR)
    {
        informationText->append("listen error");
        closesocket(sockfd);
        WSACleanup();
    }
    return sockfd;
}

void FTPBasic::getInformationText(QTextBrowser *InformationText)
{
    informationText = InformationText;
}
