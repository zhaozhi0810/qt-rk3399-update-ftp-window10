#include "mytcpsocketclient.h"
#include "ui_mytcpsocketclient.h"

MytcpSocketClient::MytcpSocketClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MytcpSocketClient)
{
    ui->setupUi(this);

    loadSettings();

    socket = new QTcpSocket(this);

//    connect(this, &MytcpSocketClient::newMessage, this, &MytcpSocketClient::displayMessage);
    connect(socket, &QTcpSocket::readyRead, this, &MytcpSocketClient::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MytcpSocketClient::discardSocket);
    //connect(this, &MytcpSocketClient::readytoconnect, this, &MytcpSocketClient::readytoconnect_tohost);
//    connect(socket, &QAbstractSocket::errorOccurred, this, &MytcpSocketClient::displayError);

    ui->label_ftp_stat1->setText("");

}

MytcpSocketClient::~MytcpSocketClient()
{
    saveSettings();

    if(socket->isOpen())
            socket->close();
    delete ui;
}



void MytcpSocketClient::setui_label_ftp_stat(bool stat)
{
    if(stat)
    {
        ui->label_ftp_stat1->setStyleSheet("QLabel{background-color:#00ff00;border-radius:5px;}");
    }
    else
    {
        ui->label_ftp_stat1->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;}");
    }
}



void MytcpSocketClient::readSocket()
{
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_7);

    while(1)
    {
        socketStream.startTransaction();
        socketStream >> buffer;

        if(!socketStream.commitTransaction())
        {
        //    QString message = QString("%1 :: Waiting for more data to come..").arg(socket->socketDescriptor());
            //emit newMessage(message);
        //    qDebug() << message;
            return;
        }

        emit newMessage(buffer);
    }
    return ;

#if 0
    QString header = buffer.mid(0,128);
    QString fileType = header.split(",")[0].split(":")[1];

    buffer = buffer.mid(128);

    if(fileType=="attachment"){
        QString fileName = header.split(",")[1].split(":")[1];
        QString ext = fileName.split(".")[1];
        QString size = header.split(",")[2].split(":")[1].split(";")[0];

        if (QMessageBox::Yes == QMessageBox::question(this, "QTCPServer", QString("You are receiving an attachment from sd:%1 of size: %2 bytes, called %3. Do you want to accept it?").arg(socket->socketDescriptor()).arg(size).arg(fileName)))
        {
            QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+fileName, QString("File (*.%1)").arg(ext));

            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly)){
                file.write(buffer);
                QString message = QString("INFO :: Attachment from sd:%1 successfully stored on disk under the path %2").arg(socket->socketDescriptor()).arg(QString(filePath));
                emit newMessage(message);
            }else
                QMessageBox::critical(this,"QTCPServer", "An error occurred while trying to write the attachment.");
        }else{
            QString message = QString("INFO :: Attachment from sd:%1 discarded").arg(socket->socketDescriptor());
            emit newMessage(message);
        }
    }else if(fileType=="message"){
        QString message = QString("%1 :: %2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message);
    }
#endif
}

void MytcpSocketClient::discardSocket()
{
//    socket->deleteLater();
//    socket=nullptr;
    qDebug()<<"discardSocket";
    this->show();
    emit disconnected_server();
//    ui->statusBar->showMessage("Disconnected!");
}

void MytcpSocketClient::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPClient", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPClient", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QMessageBox::information(this, "QTCPClient", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}



#if 0
void MytcpSocketClient::on_pushButton_sendMessage_clicked()
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString str ;//= ui->lineEdit_message->text();

            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_7);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str.toUtf8();
            byteArray.prepend(header);

            socketStream << byteArray;

        //    ui->lineEdit_message->clear();
        }
        else
            QMessageBox::critical(this,"QTCPClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPClient","Not connected");
}

void MytcpSocketClient::on_pushButton_sendAttachment_clicked()
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString filePath = QFileDialog::getOpenFileName(this, ("Select an attachment"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), ("File (*.json *.txt *.png *.jpg *.jpeg)"));

            if(filePath.isEmpty()){
                QMessageBox::critical(this,"QTCPClient","You haven't selected any attachment!");
                return;
            }

            QFile m_file(filePath);
            if(m_file.open(QIODevice::ReadOnly)){

                QFileInfo fileInfo(m_file.fileName());
                QString fileName(fileInfo.fileName());

                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_5_7);

                QByteArray header;
                header.prepend(QString("fileType:attachment,fileName:%1,fileSize:%2;").arg(fileName).arg(m_file.size()).toUtf8());
                header.resize(128);

                QByteArray byteArray = m_file.readAll();
                byteArray.prepend(header);

                socketStream.setVersion(QDataStream::Qt_5_7);
                socketStream << byteArray;
            }else
                QMessageBox::critical(this,"QTCPClient","Attachment is not readable!");
        }
        else
            QMessageBox::critical(this,"QTCPClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPClient","Not connected");
}
#endif




//void MytcpSocketClient::displayMessage(const QString& str)
//{
////    ui->textBrowser_receivedMessages->append(str);
//    qDebug() << str;
//}


void MytcpSocketClient::readytoconnect_tohost(void)
{
//    socket->connectToHost(ui->lineEdit->text(),9876);

//    if(socket->waitForConnected())
//    {
//    //    ui->pushButton_connect->setEnabled(true);
//        this->hide();
//        emit connected();
//   //     ui->statusBar->showMessage("Connected to Server");
//    }
//    else{
//        QMessageBox::critical(this,"QTCPClient", QString("The following error occurred: %1.").arg(socket->errorString()));
//    //    exit(EXIT_FAILURE);   //不退出，还可以继续
//    }
//    ui->pushButton_connect->setEnabled(true);
//    ui->pushButton_exit->setEnabled(true);
//    ui->pushButton_connect->setText("连接");
}

void MytcpSocketClient::on_pushButton_connect_clicked()
{
    socket->connectToHost(ui->lineEdit->text(),9876);

    if(socket->waitForConnected())
    {
    //    ui->pushButton_connect->setEnabled(true);
        this->hide();
        emit connected();
   //     ui->statusBar->showMessage("Connected to Server");
    }
    else{
        QMessageBox::critical(this,"QTCPClient", QString("The following error occurred: %1.").arg(socket->errorString()));
    //    exit(EXIT_FAILURE);   //不退出，还可以继续
    }
    ui->pushButton_connect->setEnabled(true);
    ui->pushButton_exit->setEnabled(true);
    ui->pushButton_connect->setText("连接");
}



void MytcpSocketClient::sendMessage(QString objName,QString Message_str)
{
//    foreach (QTcpSocket* socket,connection_set)
    {
        if(socket)
        {
            if(socket->isOpen())
            {
             //   QString Message_str ;//= ui->lineEdit_message->text();

                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_5_7);

                QByteArray header;
                QString tmp = objName+";";
                //header.prepend(QString("objName:%1,fileSize:%2;").arg(objName).arg(Message_str.size()).toUtf8());
                //header.resize(64);
                header.prepend(tmp.toUtf8());

                QByteArray byteArray = Message_str.toUtf8();
                byteArray.prepend(header);

                socketStream.setVersion(QDataStream::Qt_5_7);
                socketStream << byteArray;
            }
            else
                QMessageBox::critical(this,"QTCPServer","Socket doesn't seem to be opened");
        }
        else
            QMessageBox::critical(this,"QTCPServer","Not connected");

    }
}



void MytcpSocketClient::on_pushButton_exit_clicked()
{
    this->close();
}




void MytcpSocketClient::loadSettings()
{
    // UNIX-derived systems such as Linux and Android don't allow access to
    // port 21 for non-root programs, so we will use port 2121 instead.


    QSettings settings;
    ui->lineEdit->setText(settings.value("settings/ip", "192.168.0.100").toString());
}

void MytcpSocketClient::saveSettings()
{
    QSettings settings;
    settings.setValue("settings/ip", ui->lineEdit->text());
}




void MytcpSocketClient::on_lineEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    saveSettings();
}
