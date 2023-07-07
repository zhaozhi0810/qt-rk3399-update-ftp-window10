#include "ftpstorcommand.h"
#include <QFile>
#include <QSslSocket>
#include <QPainter>
#include <QPainter>
//#include <QMatrix>

FtpStorCommand::FtpStorCommand(QObject *parent, const QString &fileName, bool appendMode, qint64 seekTo) :
    FtpCommand(parent)
{
    this->fileName = fileName;
    this->appendMode = appendMode;
    file = 0;
    this->seekTo = seekTo;
    success = false;
}

FtpStorCommand::~FtpStorCommand()
{
    if (started) {
        if (success) {
            emit reply("226 Closing data connection.");
            if(fileName.contains("full",Qt::CaseSensitive)){
                this->makePic();
            }
        } else {
            emit reply("451 Requested action aborted: local error in processing.");
        }
    }
}

void FtpStorCommand::startImplementation()
{
    file = new QFile(fileName, this);
    qDebug() << "fileName" << fileName;
    if (!file->open(appendMode ? QIODevice::Append : QIODevice::WriteOnly)) {
        deleteLater();
        return;
    }
    success = true;
    emit reply("150 File status okay; about to open data connection.");
    if (seekTo) {
        file->seek(seekTo);
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(acceptNextBlock()));


}

void FtpStorCommand::acceptNextBlock()
{
    const QByteArray &bytes = socket->readAll();

    int bytesWritten = file->write(bytes);

    if (bytesWritten != bytes.size()) {
        emit reply("451 Requested action aborted. Could not write data to file.");
        deleteLater();
    }

}

void FtpStorCommand::makePic()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString text = current_date_time.toString("yyyy.MM.dd hh:mm:ss");

    QPixmap pm;
    pm.load(this->fileName);
    QPainter painter(&pm);
    // 绘制文字
    int fontSize = 18, spacing = 10;
    QFont font(QStringLiteral("宋体"), fontSize, QFont::Bold);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    painter.setFont(font);
    painter.setPen(QColor(250, 250, 250));

    painter.drawText(20, 80,text);
    QString texts = "FZ7000主驾";
    painter.drawText(pm.width()-250, pm.height()-50,texts);
    pm.save(this->fileName);

}


