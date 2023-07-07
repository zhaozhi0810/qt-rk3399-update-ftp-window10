#ifndef MYTCPSOCKET_CLIENT_H
#define MYTCPSOCKET_CLIENT_H


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



class mytcpsocket_client
{
public:
    mytcpsocket_client();
    ~mytcpsocket_client();
signals:
    void newMessage(QString);
private slots:
    void readSocket();
    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);
    void on_pushButton_sendMessage_clicked();
    void on_pushButton_sendAttachment_clicked();
private:

    QTcpSocket* socket;
};

#endif // MYTCPSOCKET_CLIENT_H
