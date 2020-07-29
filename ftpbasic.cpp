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


//SOCKET FTPBasic::createSocket()
//{
//    SOCKET sockfd;
//    struct sockaddr_in dataAddr;
//    dataAddr.sin_family=AF_INET;
//    dataAddr.sin_port = htons(p1*256+p2);
//    dataAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
//    memset(&(dataAddr.sin_zero), 0, sizeof(dataAddr.sin_zero));

//    //绑定
//    if (bind(sockfd, (struct sockaddr *)&dataAddr, sizeof(struct sockaddr)) == SOCKET_ERROR)
//    {
//        int err = WSAGetLastError();
//        printf("绑定地址失败，错误代码：%d\n", err);
//        closesocket(sockfd);
//        WSACleanup();
//    }

//    //侦听数据连接请求
//    if (listen(sockfd, 1) == SOCKET_ERROR)
//    {
//        printf("监听数据连接失败！\n");
//        closesocket(sockfd);
//        WSACleanup();
//    }
//    return sockfd;
//}

void FTPBasic::getInformationText(QTextBrowser *InformationText)
{
    informationText = InformationText;
}
