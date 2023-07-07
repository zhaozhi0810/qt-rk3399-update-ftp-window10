
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QFrame>
#include <QColorDialog>
#include <QtNetwork/QNetworkInterface>



//连接上服务器之后需要一些初始化
void Widget::conneted_to_server(void)
{
    this->show();   //显示这个界面

    //获取一些初始化的信息，比如当前显示的页面，键盘类型，lcd是否已连接，网卡的信息，哪些是开机默认启动的，配置文件中的信息，
    mysocket->sendMessage("connetct_init","1");  //一次性全部读过来。


}



void Widget::displayMessage(QByteArray buffer)
{
    qDebug() <<"displayMessage:" << buffer;

    QString header = buffer.mid(0,64);
    QString objName = header.split(",")[0].split(":")[1];

    buffer = buffer.mid(64);

    if(check_version_wait)  //版本查询的时间比较长，需要防止重复触发
        check_version_wait = 0;   //获得回复就允许按钮

//    ui->pushButton_update->setEnabled(true);  //升级按钮，等待一下
    ui->pushButton_version_compare->setEnabled(true);   //没有受到数据就不进行版本对比了

    //ui->pushButton_version_info->setEnabled(true);

    if(objName=="pushButton_ifconfig")
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

//        if(message == "check")
//        {

//        }
        if(message == "off")
        {
            ui->textBrowser_ifconfig->setVisible(false);
            ui->pushButton_ifconfig->setText("查看rk3399主板IP");
        }
    //    on_pushButton_ifconfig_clicked();
    }
    else if(objName=="textBrowser_ifconfig")
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        ui->textBrowser_ifconfig->setText(message);
        ui->textBrowser_ifconfig->setVisible(true);
        ui->textBrowser_ifconfig->raise();
        ui->textBrowser_ifconfig->setFocus();
        ui->textBrowser_ifconfig->setEnabled(true);
        ui->pushButton_ifconfig->setText("关闭查看rk3399IP");
    //    on_pushButton_ifconfig_clicked();
    }
    else if(objName=="show_page_index")  //显示第几页
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        stackedWidget_page_show(message.toInt());


    }
    else if(objName=="pushButton_6")  //键灯全部点亮熄灭控制
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        if(message == "1")
        {
            ui->pushButton_6->setText("结束");
            ui->pushButton_7->setEnabled(false);
            ui->pushButton->setEnabled(false);
            ui->lineEdit_interval->clearFocus();
            ui->pushButton_6->setStyleSheet("QPushButton{background-color:#ff0000;font: 10pt \"Ubuntu\";}");
        }
        else
        {
            ui->pushButton_6->setStyleSheet("QPushButton{background-color:#00ff00;font: 10pt \"Ubuntu\";}");
            ui->pushButton_6->setText("1.键灯全部点亮熄灭控制");
            ui->pushButton_7->setEnabled(true);
            ui->pushButton->setEnabled(true);
            ui->lineEdit_interval->clearFocus();
        }
    }
    else if(objName=="pushButton_7")  //键灯流水灯控制
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        if(message == "1")
        {
            ui->pushButton_7->setStyleSheet("QPushButton{background-color:#ff0000;font: 10pt \"Ubuntu\";}");
            ui->pushButton_7->setText("结束");
            ui->pushButton_6->setEnabled(false);
            ui->pushButton->setEnabled(false);
            ui->lineEdit_interval->clearFocus();
        }
        else
        {
            ui->pushButton_7->setText("2.键灯流水灯控制");
            ui->pushButton_7->setStyleSheet("QPushButton{background-color:#00ff00;font: 10pt \"Ubuntu\";}");
            ui->pushButton_6->setEnabled(true);
            ui->pushButton->setEnabled(true);
            ui->lineEdit_interval->clearFocus();
        }
    }
    else if(objName=="pushButton")  //键灯全部点亮熄灭控制
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        if(message == "1")
        {
            ui->pushButton->setText("3.键灯全部熄灭");
            ui->lineEdit_interval->clearFocus();
            ui->pushButton->setStyleSheet("QPushButton{background-color:#ff0000;font: 10pt \"Ubuntu\";}");
        }
        else
        {
            ui->pushButton->setStyleSheet("QPushButton{background-color:#00ff00;font: 10pt \"Ubuntu\";}");
            ui->lineEdit_interval->clearFocus();
            ui->pushButton->setText("3.键灯全部点亮");
        }
    }
    else if(objName=="pushButton_FlowLEDS")  //工装板流水灯
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        if(message == "1")
        {
            ui->pushButton_FlowLEDS->setText("4.工装板流水灯结束");
            ui->lineEdit_interval->clearFocus();
            ui->pushButton_FlowLEDS->setStyleSheet("QPushButton{background-color:#ff0000;font: 10pt \"Ubuntu\";}");
        }
        else
        {
            ui->lineEdit_interval->clearFocus();
            ui->pushButton_FlowLEDS->setText("4.工装板流水灯");
            ui->pushButton_FlowLEDS->setStyleSheet("QPushButton{background-color:#00ff00;font: 10pt \"Ubuntu\";}");
        }
    }
    else if("connetct_init" == objName)
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        qDebug()<<"connetct_init="<<message;
        QStringList mylist = message.split(",");  //使用逗号分割
        int i;

        for(i = 0;i<mylist.length();i++)
        {
            QStringList list1 = mylist[i].split(":");   //用冒号隔开
            if(list1.length() > 1)
            {
                qDebug()<<"list[0]=" << list1[0]<<" list1[1]="<<list1[1];
                if(list1[0] == "index")   //当前显示的页号
                {
                    stackedWidget_page_show(list1[1].toInt());
                }
                else if(list1[0] == "keyboardtype")
                {
                    show_boardtype_info(list1[1].toInt());
                }
                else if(list1[0] == "enp1_dev")
                {
                    ui->label_netdev1->setEnabled(true);
                    ui->label_netduan1->setEnabled(true);
                    ui->label_Net_Stat1->setEnabled(true);
                    ui->lineEdit_ip1->setEnabled(true);
                }
                else if(list1[0] == "enp2_dev")
                {
                    ui->label_netdev2->setEnabled(true);
                    ui->label_netduan2->setEnabled(true);
                    ui->label_Net_Stat2->setEnabled(true);
                    ui->lineEdit_ip2->setEnabled(true);
                }
                else if(list1[0] == "eth2_dev")
                {
                    ui->label_netdev3->setEnabled(true);
                    ui->label_netduan3->setEnabled(true);
                    ui->label_Net_Stat3->setEnabled(true);
                    ui->lineEdit_ip3->setEnabled(true);
                }
                else if(list1[0] == "eth0_dev")
                {
                    ui->label_netdev3->setEnabled(true);
                    ui->label_netduan3->setEnabled(true);
                    ui->label_Net_Stat3->setEnabled(true);
                    ui->lineEdit_ip3->setEnabled(true);
                    ui->label_netdev3->setText("eth0");
                    ui->pushButton_5->setText("ping eth0");
                }
                else if(list1[0] == "ip1")
                {
                    ui->lineEdit_ip1->setText(list1[1]);
                }
                else if(list1[0] == "ip2")
                {
                    ui->lineEdit_ip2->setText(list1[1]);
                }
                else if(list1[0] == "ip3")
                {
                    ui->lineEdit_ip3->setText(list1[1]);
                }
                else if(list1[0] == "default_show_page")    //启动默认显示页面,默认是第一页
                {
                    ui->comboBox->setCurrentIndex(list1[1].toInt());
                }
                else if(list1[0] == "is_cpu_stress_start")   //启动开始cpu压力测试？0表示不开启，1开启测试
                {
                    ui->checkBox_cpu_stress->setChecked( list1[1].toInt()==1);
                }
                else if(list1[0] == "is_gpio_flow_start")   //启动开启gpio流水灯吗？
                {
                    ui->checkBox_gpio_flow->setChecked( list1[1].toInt()==1);
                }
                else if(list1[0] == "is_key_lights_start")   //启动开启键灯吗？
                {
                    ui->checkBox_keyLights->setChecked( list1[1].toInt()==1);
                }
                else if(list1[0] == "is_cpu_test_checked")
                {
                    ui->checkBox_cpu_n->setChecked( list1[1].toInt()==1);
                }
                else if(list1[0] == "is_mem_test_checked")
                {
                    ui->checkBox_mem_n->setChecked( list1[1].toInt()==1);
                }
                else if(list1[0] == "cpu_test_core_num")   //cpu的测试核心数
                {
                    ui->comboBox_cpu->setCurrentIndex(list1[1].toInt());
                }
                else if(list1[0] == "mem_test_usage")    //内存测试的百分比
                {
                    ui->comboBox_memory->setCurrentIndex(list1[1].toInt());
                }
            }
        }

    }
    else if("verticalScrollBar_lightpwm2" == objName)  //键灯测试页：键灯亮度滑条调节
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        ui->verticalScrollBar_lightpwm2->setValue(message.toInt());   //这个....
    }
    else if(objName=="Palette_button")  //上一页
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        qDebug()<<"drvLightLED="<<message;
        QStringList mylist = message.split(",");  //使用逗号分割
        int ison,keyval;

        QStringList list1;
        if(mylist.length() > 1)
        {
            list1 = mylist[0].split(":");
            ison = list1[1].toInt();

            list1 = mylist[1].split(":");
            keyval =  list1[1].toInt();

            qDebug()<<"ison="<<ison << " keyval=" << keyval;

            Palette_button(ison,keyval);
        }
    }
    else if(objName=="checkBox")  //
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        bool checked = message.toInt();
        ui->checkBox->setChecked(checked);   //这个....
    }
    else if(objName=="pushButton_start_color_test")  //
    {
        ui->label_color->setVisible(true);
        ui->pushButton_lcd_last_color->setVisible(true);
        ui->pushButton_lcd_next_color->setVisible(true);
        ui->label_color->setStyleSheet("background-color:rgb(255,0,0)");
        ui->label_color->raise();
    }
    else if(objName=="page2_show_color")  // lcd颜色测试
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        int page2_show_color1 = message.toInt();
        if(page2_show_color1 > 4)
        {
            ui->label_color->setVisible(false);
            ui->pushButton_lcd_last_color->setVisible(false);
            ui->pushButton_lcd_next_color->setVisible(false);
            ui->label_color->lower();
        }
        else if(page2_show_color1 == 0)
        {
            ui->label_color->setStyleSheet("background-color:rgb(255,0,0)");
        }
        else if(page2_show_color1 == 1)
        {
            ui->label_color->setStyleSheet("background-color:rgb(0,255,0)");

        }
        else if(page2_show_color1 == 2)
        {
            ui->label_color->setStyleSheet("background-color:rgb(0,0,255)");
        }
        else if(page2_show_color1 == 3)
        {
            ui->label_color->setStyleSheet("background-color:rgb(255,255,255)");
        }
        else if(page2_show_color1 == 4)
        {
            ui->label_color->setStyleSheet("background-color:rgb(0,0,0)");
        }
    }
    else if(objName=="horizontalScrollBar_light")  //
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        ui->horizontalScrollBar_light->setValue(message.toInt());
    }
    else if("cpu_mem_info" == objName)
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        qDebug()<<"connetct_init="<<message;
        QStringList mylist = message.split(",");  //使用逗号分割
        int i;

        for(i = 0;i<mylist.length();i++)
        {
            QStringList list1 = mylist[i].split(":");   //用冒号隔开
            if(list1.length() > 1)
            {
                qDebug()<<"list[0]=" << list1[0]<<" list1[1]="<<list1[1];
                if(list1[0] == "mem_total")   //当前显示的页号
                {
                    ui->label_mem_total->setText(list1[1]);
                }
                else if(list1[0] == "mem_usage")
                {
                    ui->label_mem_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu_temp")
                {
                    ui->label_cpu_temp->setText(list1[1]);
                }
                else if(list1[0] == "gpu_temp")
                {
                    ui->label_gpu_temp->setText(list1[1]);
                }
                else if(list1[0] == "cpu_usage")
                {
                    ui->label_cpu_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu0_usage")
                {
                    ui->label_cpu0_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu1_usage")
                {
                    ui->label_cpu1_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu2_usage")
                {
                    ui->label_cpu2_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu3_usage")
                {
                    ui->label_cpu3_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu4_usage")    //启动默认显示页面,默认是第一页
                {
                    ui->label_cpu4_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu5_usage")   //启动开始cpu压力测试？0表示不开启，1开启测试
                {
                    ui->label_cpu5_usage->setText(list1[1]);
                }
                else if(list1[0] == "cpu0_freq")   //启动开启gpio流水灯吗？
                {
                    ui->label_cpu0_freq->setText(list1[1]);
                }
                else if(list1[0] == "cpu1_freq")   //启动开启键灯吗？
                {
                    ui->label_cpu1_freq->setText(list1[1]);
                }
                else if(list1[0] == "cpu2_freq")
                {
                    ui->label_cpu2_freq->setText(list1[1]);
                }
                else if(list1[0] == "cpu3_freq")
                {
                    ui->label_cpu3_freq->setText(list1[1]);
                }
                else if(list1[0] == "cpu4_freq")   //cpu的测试核心数
                {
                    ui->label_cpu4_freq->setText(list1[1]);
                }
                else if(list1[0] == "cpu5_freq")    //内存测试的百分比
                {
                    ui->label_cpu5_freq->setText(list1[1]);
                }
                else if(list1[0] == "start_time")   //cpu的测试核心数
                {
                    ui->label_start_time->setText(list1[1]+":"+list1[2]+":"+list1[3]);
                }
                else if(list1[0] == "has_run_time")    //内存测试的百分比
                {
                    QString temp = list1[1];
                    if(list1.length()>2)
                    {
                        temp += ":";
                        temp += list1[2];
                    }

                    ui->label_has_run_time->setText(temp);
                }
            }
        }

    }
    else if(objName=="pushButton_start_cpustress")  //
    {
        QString size = header.split(",")[1].split(":")[1].split(";")[0];
        qDebug() << "size=" << size;

        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));

        if(message == "1")
        {
            ui->pushButton_start_cpustress->setText("结束压力测试");
            ui->pushButton_start_cpustress->setStyleSheet("QPushButton{background-color:#ff0000;font: 10pt \"Ubuntu\";}");
        }
        else
        {
            ui->pushButton_start_cpustress->setText("开始压力测试");
            ui->pushButton_start_cpustress->setStyleSheet("QPushButton{background-color:#00ff00;font: 10pt \"Ubuntu\";}");
        }
    }
    else if(objName=="textBrowser_disk_info")  //
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        ui->textBrowser_disk_info->setText(message);
    }
    else if(objName=="textBrowser_system_info")  //
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        ui->textBrowser_system_info->setText(message);
    }
    else if(objName=="textBrowser_version")  //
    {
//        QFile qfile("mysoft_version.txt");
        version_store_string = QString("%1").arg(QString::fromStdString(buffer.toStdString()));  //用一个变量存起来，方便之后版本对比
        qDebug() << version_store_string;
        ui->textBrowser_version_info->setText(version_store_string);

        //收到数据后，进行版本比对
        softwareversion_version_compare(version_store_string);

    }
    else if(objName=="pushButton_update")
    {
        QString message = QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        ui->textBrowser_version_info->setText(message);
    }
}
