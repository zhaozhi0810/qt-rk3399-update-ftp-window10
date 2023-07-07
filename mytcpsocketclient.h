#ifndef MYTCPSOCKETCLIENT_H
#define MYTCPSOCKETCLIENT_H

#include <QWidget>
#include <QAbstractSocket>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QMetaType>
#include <QString>
#include <QStandardPaths>
#include <QTcpSocket>



namespace Ui {
class MytcpSocketClient;
}

class MytcpSocketClient : public QWidget
{
    Q_OBJECT

public:
    explicit MytcpSocketClient(QWidget *parent = 0);
    ~MytcpSocketClient();
    void sendMessage(QString objName,QString Message_str);
    void setui_label_ftp_stat(bool stat);
signals:
    void newMessage(QByteArray);
    void connected();
    void disconnected_server();
    void readytoconnect(void);   //准备连接
private slots:
    void readSocket();
    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

//    void displayMessage(QByteArray str);
//    void on_pushButton_sendMessage_clicked();
//    void on_pushButton_sendAttachment_clicked();
    void on_pushButton_connect_clicked();
    void readytoconnect_tohost(void);



    void on_pushButton_exit_clicked();

private:
    Ui::MytcpSocketClient *ui;

    QTcpSocket* socket;

};

#endif // MYTCPSOCKETCLIENT_H
