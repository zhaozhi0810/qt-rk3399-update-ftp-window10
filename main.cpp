#include "widget.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget wiget;

    wiget.setFixedSize(366,645);
#if 1
    //wiget.show();
#else
    QGraphicsScene *scene = new QGraphicsScene;
    QGraphicsProxyWidget *w = scene->addWidget(&wiget);
    w->setRotation(-90);

    QGraphicsView *view = new QGraphicsView(scene);
    view->resize(1280, 720);
    view->show();
#endif

//    const QString &userName = "ftp_hnhtjc";//getRandomString(3);
//    const QString &password = "123456";//getRandomString(3);
//    const QString &rootPath = "d:/mcu_update/";//QDir::currentPath();

//    // *TODO: Allow using port 0.
//    FtpServer server(&a, rootPath, 21, userName, password, false, false);
//    if (server.isListening()) {
//        qDebug() << QString("Listening at %1:21").arg(FtpServer::lanIp()).toStdString().c_str();
//        qDebug() << QString("User: %1").arg(userName).toStdString().c_str();
//        qDebug() << QString("Password: %1").arg(password).toStdString().c_str();
//        return a.exec();
//    } else {
//        qDebug() << QString("Failed to start").toStdString().c_str();
//        return 1;
//    }

    return a.exec();
}
