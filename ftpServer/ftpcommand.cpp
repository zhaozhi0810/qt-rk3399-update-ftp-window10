#include "ftpcommand.h"
#include <QTcpSocket>

FtpCommand::FtpCommand(QObject *parent) :
    QObject(parent)
{
    started = false;
}

void FtpCommand::start(QTcpSocket *socket)
{
    started = true;
    this->socket = socket;
    socket->setParent(this);
    connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    startImplementation();
}
