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
#include <QCryptographicHash>



const char* g_build_time_str = "Buildtime :" __DATE__ " " __TIME__ ;   //获得编译时间
QStringList g_show_title;   //获得编译时间
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define PAGES 9  //显示总页数



static Qt::Key keys_value[] = {Qt::Key_F1/*L1*/,Qt::Key_F2/*R1*/,Qt::Key_F3,Qt::Key_F4,Qt::Key_F5,Qt::Key_F6,Qt::Key_F7,Qt::Key_F8,
                              Qt::Key_F9,Qt::Key_F10,Qt::Key_Z/*L6*/,Qt::Key_X/*R6*/,Qt::Key_F11/*inc*/,Qt::Key_F12/*ext*/,
                               Qt::Key_0,Qt::Key_1,Qt::Key_2,Qt::Key_3,Qt::Key_4,Qt::Key_5,Qt::Key_6,
                              Qt::Key_7,Qt::Key_8,Qt::Key_9,Qt::Key_Asterisk/*\**/,Qt::Key_Slash/*#*/,
                                Qt::Key_C/*tel*/,Qt::Key_Control/*test*/,Qt::Key_Plus/*V+*/,Qt::Key_Minus/*V-*/,
                               Qt::Key_Up/*up*/,Qt::Key_Down/*down*/,Qt::Key_P/*Ptt*/
                               , Qt::Key_Left/*left*/,Qt::Key_Right/*riht*/,Qt::Key_Return /*ok*/   /*20230504 多功能增加3个按键，左，右，ok，切换（与测试键值相同）*/
                              };
#ifdef RK_3399_PLATFORM
static int led_key_map[] = {
 1,2,3,4,5,6,7,8,9,10,44,45,11,12,27,18,19,20,21,22,23,24,25,26,28,29,13,14,35,36,30,31,17,32,33,34,37,38,39    /*20230504 多功能增加3个按键，左32，右33，ok34，切换（与测试键值相同）37,38,39是三色灯*/
};
#endif

QTextBrowser * g_help_brower[PAGES];



struct system_config
{
    int is_cpu_stress_start;   //启动开始cpu压力测试？0表示不开启，1开启测试
    int is_gpio_flow_start;   //启动开启gpio流水灯吗？
    int is_key_lights_start;  //启动开启键灯吗？
    int is_cpu_test_checked;
    int is_mem_test_checked;
    int default_show_page;    //启动默认显示页面,默认是第一页
    int cpu_test_core_num;    //cpu的测试核心数
    int mem_test_usage;       //内存测试的百分比
    int ip1;
    int ip2;
    int ip3;
}g_sys_conf;

void ReadConfigFile();
void SetConfigFile(void);



const QString &myftp_userName = "ftp_hnhtjc";//getRandomString(3);
const QString &myftp_password = "123456";//getRandomString(3);
const QString &myftp_rootPath = "./";//QDir::currentPath();



Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    int i;
    ui->setupUi(this);

    mysocket = new MytcpSocketClient();
    mysocket->show();
    connect(mysocket, &MytcpSocketClient::connected, this, &Widget::conneted_to_server);  //
    connect(mysocket, &MytcpSocketClient::disconnected_server, this, &Widget::hide);  //disconnected_server
    connect(mysocket, &MytcpSocketClient::newMessage, this, &Widget::displayMessage);

//    version_store_string.clear();  //清空

    ui->label->setText(g_build_time_str);
    ui->label_2->setText("Create by dazhi-2023");


    g_show_title<<"1.键盘测试"<<"2.键灯测试"<<"3.LCD测试"<<"4.网络测试"<<"5.音频测试"<<"6.IIC/SPI/UART"<<"7.其他配置"<<"8.系统信息"<<"9.软件版本信息";

    g_help_brower[0] = ui->textBrowser_help_page1;
    g_help_brower[1] = ui->textBrowser_help_page2;
    g_help_brower[2] = ui->textBrowser_help_page3;
    g_help_brower[3] = ui->textBrowser_help_page4;
    g_help_brower[4] = ui->textBrowser_help_page5;
    g_help_brower[5] = ui->textBrowser_help_page6;
    g_help_brower[6] = ui->textBrowser_help_page7;
    g_help_brower[7] = ui->textBrowser_help_page8;
    g_help_brower[8] = ui->textBrowser_help_page9;

    for(i=0;i<PAGES;i++)
    {
        g_help_brower[i]->lower();
        g_help_brower[i]->setVisible(false);
    }


    ui->label_color->setVisible(false);
    ui->pushButton_lcd_last_color->setVisible(false);
    ui->pushButton_lcd_next_color->setVisible(false);
    ui->radioButton_micpanel->setEnabled(false);
    ui->radioButton_michand->setEnabled(false);

    buttonFrame = new QFrame;
    page2_show_color = 0;
//    check_version_wait = 0;   //获取版本的时候时间比较长，防止多次按钮
    ui->pushButton_update->setEnabled(false);  //版本对比后决定是否升级
    //ui->pushButton_version_compare->setEnabled(true);
    update_command_wait = 0;  //升级，首先需要打包，需要一些时间等待，防止多次按钮
    is_test_press = 0;     //测试键没有按下
//    key_light_connect = 1;  //键灯关联

    intValidator = new QIntValidator;
    intValidator->setRange(1,999999);
    ui->lineEdit_interval->setValidator(intValidator);

    QRegExp rx("^([1-9]|[1-9]\\d|(1[0-9]\\d)|(2[0-4]\\d)|(2[5][0-4]))$");//输入范围为【1-254】
    pReg = new QRegExpValidator(rx, this);
    ui->lineEdit_ip1->setValidator(pReg);
    ui->lineEdit_ip2->setValidator(pReg);
    ui->lineEdit_ip3->setValidator(pReg);
    ui->textBrowser_ifconfig->setVisible(false);//显示ip信息的暂时不可见


    //lightpwm = 100;

    ui->label_ping_reson1->setText("");
    ui->label_ping_reson2->setText("");
    ui->label_ping_reson3->setText("");

    //lcdPwm = 90;

//    iicspi_connect = 0;

    ui->toolButton_left->setVisible(false);
    ui->toolButton_right->setVisible(false);
    ui->toolButton_ok->setVisible(false);
    ui->label_ftp_stat->setText("");

    myftp_server = new FtpServer(this, myftp_rootPath, 21, myftp_userName, myftp_password, false, false);
    if (myftp_server->isListening()) {
        qDebug() << QString("Listening at %1:21").arg(FtpServer::lanIp()).toStdString().c_str();
        qDebug() << QString("User: %1").arg(myftp_userName).toStdString().c_str();
        qDebug() << QString("Password: %1").arg(myftp_password).toStdString().c_str();
        ui->label_ftp_stat->setStyleSheet("QLabel{background-color:#00ff00;border-radius:5px;}");
        mysocket->setui_label_ftp_stat(true);
    } else {
        qDebug() << QString("Failed to start").toStdString().c_str();
        ui->label_ftp_stat->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;}");
        mysocket->setui_label_ftp_stat(false);
    }
}






Widget::~Widget()
{
    delete myftp_server;

    delete ui;
}









void Widget::Palette_button(int ison,int keyval)
{
    //注意与keys_value数组对应
    QToolButton* key_buttons[] = {ui->toolButton_L1,ui->toolButton_R1,ui->toolButton_L2,ui->toolButton_R2,ui->toolButton_L3,ui->toolButton_R3,ui->toolButton_L4,
                                   ui->toolButton_R4,ui->toolButton_L5,ui->toolButton_R5,ui->toolButton_L6,ui->toolButton_R6,ui->toolButton_Inc,ui->toolButton_Ext,
                                   ui->toolButton_0,ui->toolButton_1,ui->toolButton_2,ui->toolButton_3,ui->toolButton_4,ui->toolButton_5,ui->toolButton_6,
                                   ui->toolButton_7,ui->toolButton_8,ui->toolButton_9,ui->toolButton_11,ui->toolButton_12,ui->toolButton_Tell,ui->toolButton_Test,
                                   ui->toolButton_V1,ui->toolButton_V2,ui->toolButton_Up,ui->toolButton_Down,ui->toolButton_Ptt,ui->toolButton_left,ui->toolButton_right,ui->toolButton_ok
                                   };
//    qDebug()<<"Palette_button ison="<<ison <<" key" << keyval;
    QPalette p = buttonFrame->palette();
    QString color = "background-color:white;";
    if(ison)
    {
        //p.setColor(QPalette::Button,Qt::green);
       color = "background-color:green;";
    }
    else
    {
        if(keyval == 0)
        {
            //p.setColor(QPalette::Button,QColor(1, 1, 1, 1));//Qt::white);

            for(unsigned int i=0;i<ARRAY_SIZE(keys_value);i++)
            {
                //key_buttons[i]->setPalette(p);
                key_buttons[i]->setStyleSheet("QPushButton{background-color:QColor(1, 1, 1, 20);font: 20pt \"Ubuntu\"}");
            }
            return;
        }
        else
        {
            p.setColor(QPalette::Button,Qt::white);
            color = "background-color:white;";
        }
    }
    if(is_test_press == 0)  //判断是否有组合键功能
    {
        for(unsigned int i=0;i<ARRAY_SIZE(keys_value);i++)
        {
            if(keyval == keys_value[i])
            {
                key_buttons[i]->setStyleSheet(color + "font: 15pt \"Ubuntu\"");  //Ping_stat[i]->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 20pt \"Ubuntu\";}");
               // key_buttons[i]->setf//setFlat(true);
               // key_buttons[i]->setPalette(p);
                //key_buttons[i]->setAutoFillBackground(true);
                //进行两项设置


           //     qDebug()<<"setPalette i=" << i << key_buttons[i];

                break;
            }
        }
    }
}










//下一项
void Widget::stackedWidget_page_show(int index)
{
    if(index < g_show_title.count())
        ui->label_Page_title->setText(g_show_title.at(index));

    if(index == 3){  //进入网络测试页，开启定时器
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
    }

    ui->stackedWidget->setCurrentIndex(index);

    if(index == 0)
    {
        ui->pushButton_Last_page->setEnabled(false);
        ui->pushButton_Next_page->setEnabled(true);

        Palette_button(0,0);//键恢复为灰色 按键框怎么不见了！！！！

    }
    else if(index == PAGES-1)
    {
        ui->pushButton_Next_page->setEnabled(false);
        ui->pushButton_Last_page->setEnabled(true);
    }
    else
    {
        ui->pushButton_Next_page->setEnabled(true);
        ui->pushButton_Last_page->setEnabled(true);
    }
}





//下一项
void Widget::next_func_page_show()
{
    int index = ui->stackedWidget->currentIndex();

    if(g_help_brower[index]->isVisible())
    {
        g_help_brower[index]->setVisible(false);
        g_help_brower[index]->lower();
    }

    if(index < (PAGES-1) )
        index ++;

    stackedWidget_page_show(index);


}
//上一项
void Widget::last_func_page_show()
{
    int index = ui->stackedWidget->currentIndex();

    if(g_help_brower[index]->isVisible())
    {
        g_help_brower[index]->setVisible(false);
        g_help_brower[index]->lower();
    }


    if(index > 0 )
        index --;

    if(index == 0){
        Palette_button(0,0);//键恢复为灰色
    }
    stackedWidget_page_show(index);
}



void Widget::closeEvent(QCloseEvent *event)
{
    event->accept();
    qApp->quit();
}




//page2 start test lcd color
void Widget::on_pushButton_start_color_test_clicked()
{
    if(ui->label_color->isVisible())
        return;

    ui->label_color->setVisible(true);
    ui->pushButton_lcd_last_color->setVisible(true);
    ui->pushButton_lcd_next_color->setVisible(true);

    ui->label_color->setStyleSheet("background-color:rgb(255,0,0)");
    ui->label_color->raise();

    mysocket->sendMessage("pushButton_start_color_test","1");
}



//page3 left
void Widget::last_color_page_show()
{
    if(page2_show_color <= 0)
    {
        page2_show_color = 0;
        return;
    }
    else if(page2_show_color == 1)
    {
        page2_show_color = 0;
        ui->label_color->setStyleSheet("background-color:rgb(255,0,0)");
    }
    else if(page2_show_color == 2)
    {
        page2_show_color = 1;
        ui->label_color->setStyleSheet("background-color:rgb(0,255,0)");

    }
    else if(page2_show_color == 3)
    {
        page2_show_color = 2;
        ui->label_color->setStyleSheet("background-color:rgb(0,0,255)");
    }
    else if(page2_show_color == 4)
    {
        page2_show_color = 3;
        ui->label_color->setStyleSheet("background-color:rgb(255,255,255)");
    }
}


////page3 right
void Widget::next_color_page_show()
{
    if(page2_show_color >= 4)
    {
        ui->label_color->setVisible(false);
        ui->pushButton_lcd_last_color->setVisible(false);
        ui->pushButton_lcd_next_color->setVisible(false);
        ui->label_color->lower();

        page2_show_color = 0;
        return;
    }
    else if(page2_show_color == 0)
    {
        page2_show_color = 1;
        ui->label_color->setStyleSheet("background-color:rgb(0,255,0)");
    }
    else if(page2_show_color == 1)
    {
        page2_show_color = 2;
        ui->label_color->setStyleSheet("background-color:rgb(0,0,255)");

    }
    else if(page2_show_color == 2)
    {
        page2_show_color = 3;
        ui->label_color->setStyleSheet("background-color:rgb(255,255,255)");
    }
    else if(page2_show_color == 3)
    {
        page2_show_color = 4;
        ui->label_color->setStyleSheet("background-color:rgb(0,0,0)");
    }
}


//page 0 : 键灯全部点亮
void Widget::on_pushButton_clicked()
{
    mysocket->sendMessage("pushButton","1");
}



void Widget::on_pushButton_FlowLEDS_clicked()
{
    mysocket->sendMessage("pushButton_FlowLEDS","1");
}





//音频测试页：播放按键
void Widget::on_pushButton_Play_clicked()
{
    mysocket->sendMessage("pushButton_Play","1");
}


//键灯全部点亮熄灭控制
void Widget::on_pushButton_6_clicked()
{
    mysocket->sendMessage("pushButton_6","1");

}

//音频测试页：键灯流水灯控制
void Widget::on_pushButton_7_clicked()
{
    mysocket->sendMessage("pushButton_7","1");

}



void Widget::ping_info_show(QString &strMsg,int ping_num)
{
    //QString strMsg = myprocess_ping1->readAllStandardOutput();
    QLabel* Ping_stat[3] = {ui->label_ping_stat1,ui->label_ping_stat2,ui->label_ping_stat3};
    QLabel* ping_err[3] = {ui->label_ping_err1,ui->label_ping_err2,ui->label_ping_err3};
    QLabel* timeval[3] = {ui->label_timeval1,ui->label_timeval2,ui->label_timeval3};
    QLabel* icmpseq[3] ={ui->label_icmpseq1,ui->label_icmpseq2,ui->label_icmpseq3};

    QStringList myList,message_List ;
//    qDebug() << strMsg;
    int len,i;

    if(ping_num < 0 || ping_num > 2)
        return;

    if(strMsg.endsWith("data.\n") || strMsg.startsWith("ping",Qt::CaseInsensitive)  || strMsg.contains("root", Qt::CaseInsensitive))
    {
        return;
    }

    if(strMsg.contains("timed out", Qt::CaseInsensitive))
    {
        error_count[ping_num] ++;
        ping_err[ping_num]->setText(QString::number(error_count[ping_num]));
        return;
    }

    message_List= strMsg.split('\n');
    len = message_List.length();
    for(i=0;i<len;i++)
    {
        if(message_List[i].contains("Host Unreachable", Qt::CaseInsensitive))
        {
            if(Ping_stat[ping_num]->text() != "异常")
            {
                Ping_stat[ping_num]->setText("异常");
                Ping_stat[ping_num]->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 10pt \"Ubuntu\";}");
            }

            //if(myList[i].contains("Host Unreachable", Qt::CaseInsensitive))
            error_count[ping_num] ++;
            ping_err[ping_num]->setText(QString::number(error_count[ping_num]));
        }

        else
        {
            myList = message_List[i].split(' ');

            if(myList.length()>6)
            {
            //    qDebug()<< myList[6];    //time
                QStringList myList1 = myList[6].split('=');
                if(myList1.length()>1)
                {
                    timeval[ping_num]->setText(myList1[1]);
                    Ping_stat[ping_num]->setText("正常");
                    Ping_stat[ping_num]->setStyleSheet("QLabel{background-color:#00ff00;border-radius:5px;font: 10pt \"Ubuntu\";}");
                }
                else
                {
                    Ping_stat[ping_num]->setText("异常");
                    Ping_stat[ping_num]->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 10pt \"Ubuntu\";}");
                    error_count[ping_num] ++;
                    ping_err[ping_num]->setText(QString::number(error_count[0]));
                }
                myList1 = myList[4].split('=');   //icmp_seq
                if(myList1.length()>1)
                {
                    icmp_cur[ping_num] = myList1[1].toInt();
                    icmpseq[ping_num]->setText(myList1[1]);
                }
            }
        }
    }
}





//网络测试页 ： ping enp1s0f0
void Widget::on_pushButton_2_clicked()
{
    mysocket->sendMessage("pushButton_2","1");
}


//网络测试页 ： ping enp1s0f1
void Widget::on_pushButton_4_clicked()
{
    mysocket->sendMessage("pushButton_4","1");
}


//网络测试页 ： ping eth2
void Widget::on_pushButton_5_clicked()
{
    mysocket->sendMessage("pushButton_5","1");
}



void Widget::on_horizontalScrollBar_SpeakVol_sliderMoved(int position)
{
    mysocket->sendMessage("horizontalScrollBar_SpeakVol",QString::number(position));   //把这个值发送过去
}



//音频测试页： 手柄音量调整滑动
void Widget::on_horizontalScrollBar_HandVol_valueChanged(int value)
{
    value = value;
//    qDebug()<<"手柄音量" << value  ;

}

//音频测试页： 耳机音量调整滑动
void Widget::on_horizontalScrollBar_EarphVol_valueChanged(int value)
{
    value = value;
//    qDebug()<<"耳机音量" << value  ;

}




//键灯测试页：键灯亮度滑条调节
void Widget::on_verticalScrollBar_lightpwm2_valueChanged(int value)
{
    ui->label_light_value_2->setText(QString::number(value));
}

void Widget::on_verticalScrollBar_lightpwm2_sliderMoved(int position)
{
    mysocket->sendMessage("verticalScrollBar_lightpwm2",QString::number(position));   //把这个值发送过去
}



//键测试页:键灯关联
void Widget::on_checkBox_toggled(bool checked)
{
    //key_light_connect = checked;
    mysocket->sendMessage("checkBox",QString::number(checked));   //把这个值发送过去
}



//网络测试页，配置3399网卡ip
void Widget::on_pushButton_3_clicked()
{
    mysocket->sendMessage("pushButton_3","1");   //把这个值发送过去
}


void Widget::on_pushButton_ifconfig_clicked()
{
    mysocket->sendMessage("pushButton_ifconfig","check");
}

void Widget::on_radioButton_loop_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(true);
        ui->radioButton_micpanel->setEnabled(true);
    }
    mysocket->sendMessage("radioButton_loop",QString::number(checked));
}

void Widget::on_radioButton_playmusic_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(false);
        ui->radioButton_micpanel->setEnabled(false);
    }
    mysocket->sendMessage("radioButton_playmusic",QString::number(checked));
}

void Widget::on_radioButton_playrec_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(false);
        ui->radioButton_micpanel->setEnabled(false);
    }

    mysocket->sendMessage("radioButton_playrec",QString::number(checked));
}

void Widget::on_radioButton_rec_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(true);
        ui->radioButton_micpanel->setEnabled(true);
    }
    mysocket->sendMessage("radioButton_rec",QString::number(checked));
}





void Widget::on_horizontalScrollBar_light_valueChanged(int value)
{
    ui->label_light_val->setText(QString::number(value));
}

void Widget::on_horizontalScrollBar_light_sliderMoved(int position)
{
    mysocket->sendMessage("horizontalScrollBar_light",QString::number(position));   //把这个值发送过去
}






//IIC、SPI数据读取
void Widget::on_pushButton_8_clicked()
{
    mysocket->sendMessage("pushButton_8","1");   //把这个值发送过去
}



//IIC、SPII数据写入
void Widget::on_pushButton_9_clicked()
{
    mysocket->sendMessage("pushButton_9","1");   //把这个值发送过去
}




//IIC、SPI数据擦除
void Widget::on_pushButton_10_clicked()
{
    mysocket->sendMessage("pushButton_10","1");   //把这个值发送过去
}




void Widget::on_radioButton_micpanel_clicked()
{

}

void Widget::on_radioButton_michand_clicked()
{

}




//void Widget::on_radioButton_HandVol_toggled(bool checked)
//{

//}



void Widget::on_pushButton_start_cpustress_clicked()
{
    mysocket->sendMessage("pushButton_start_cpustress","1");   //把这个值发送过去
}




//开机自动启动配置，记录到文件中
void Widget::on_checkBox_cpu_stress_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_cpu_stress",QString::number(checked));   //把这个值发送过去
}


//开机自动启动配置，记录到文件中
void Widget::on_checkBox_gpio_flow_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_gpio_flow",QString::number(checked));   //把这个值发送过去
}


//开机自动启动配置，记录到文件中
void Widget::on_checkBox_keyLights_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_keyLights",QString::number(checked));   //把这个值发送过去
}



void Widget::on_comboBox_memory_currentIndexChanged(int index)
{
    mysocket->sendMessage("comboBox_memory",QString::number(index));   //把这个值发送过去
}

void Widget::on_comboBox_cpu_currentIndexChanged(int index)
{
    mysocket->sendMessage("comboBox_cpu",QString::number(index));   //把这个值发送过去
}






void Widget::on_checkBox_cpu_n_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_cpu_n",QString::number(checked));   //把这个值发送过去
}

void Widget::on_checkBox_mem_n_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_mem_n",QString::number(checked));   //把这个值发送过去
}

void Widget::on_comboBox_currentIndexChanged(int index)
{
    mysocket->sendMessage("comboBox",QString::number(index));   //把这个值发送过去
}



void Widget::show_boardtype_info(int keyboard_type)
{
//    int ret = -1;
    QStringList boardtype_list;
    boardtype_list << "嵌1" << "嵌2" << "嵌3" << "壁挂2" << "防风雨" << "多功能";
    ui->label_keyboard_type->setText("未知");
 //   if(had_keyboard)
    {
        if((keyboard_type >= 0) && (keyboard_type < 6))
        {
            ui->label_keyboard_type->setText(boardtype_list.at(keyboard_type));

            if(keyboard_type == 5)
            {
                ui->toolButton_left->setVisible(true);
                ui->toolButton_right->setVisible(true);
                ui->toolButton_ok->setVisible(true);
            }
        }
    }

}





void Widget::on_pushButton_disk_info_clicked()
{
    mysocket->sendMessage("pushButton_disk_info","1");   //把这个值发送过去
}





//串口测试的选择，checked表示是否选择
void Widget::on_radioButton_Uarttest_toggled(bool checked)
{
    mysocket->sendMessage("radioButton_Uarttest",QString::number(checked));   //把这个值发送过去

    if(checked)
    {
        ui->pushButton_8->setEnabled(false);
        ui->pushButton_9->setEnabled(false);
        ui->pushButton_10->setEnabled(false);
        ui->textBrowser_IICSPI->clear();
        ui->textBrowser_IICSPI->setFocus();
    }
    else
    {
        ui->pushButton_8->setEnabled(true);
        ui->pushButton_9->setEnabled(true);
        ui->pushButton_10->setEnabled(true);
    }
}

void Widget::on_pushButton_clear_display_clicked()
{
    ui->textBrowser_IICSPI->clear();
    mysocket->sendMessage("pushButton_clear_display","1");   //把这个值发送过去
}





void Widget::on_pushButton_Last_page_clicked()
{
    mysocket->sendMessage("pushButton_Last_page","1");
}

void Widget::on_pushButton_Next_page_clicked()
{
    mysocket->sendMessage("pushButton_Next_page","1");
}

void Widget::on_pushButton_Help_clicked()
{

    DebugLogDialog *dlg = new DebugLogDialog;
    dlg->setAttribute( Qt::WA_DeleteOnClose, true );
    dlg->setModal(true);   //模态对话框，该窗口不关闭，其他窗口不能运行！！！
    dlg->showExpanded();


//    int i;

//    i = ui->stackedWidget->currentIndex();

//    if(g_help_brower[i]->isVisible())
//    {
//        g_help_brower[i]->setVisible(false);
//        g_help_brower[i]->lower();
//    }
//    else
//    {
//        g_help_brower[i]->setVisible(true);
//        g_help_brower[i]->raise();
//    }
}







//闪烁间隔时间调整
void Widget::on_lineEdit_interval_textEdited(const QString &arg1)
{
    mysocket->sendMessage("lineEdit_interval",arg1);
}



void Widget::on_pushButton_lcd_last_color_clicked()
{
    if(ui->label_color->isVisible())
        mysocket->sendMessage("pushButton_lcd_last_color","1");
}

void Widget::on_pushButton_lcd_next_color_clicked()
{
    if(ui->label_color->isVisible())
        mysocket->sendMessage("pushButton_lcd_next_color","1");
}



void Widget::get_softwareversion_info()
{
//    ui->pushButton_version_info->setEnabled(false);
    ui->pushButton_version_compare->setEnabled(false);   //没有受到数据就不进行版本对比了

    mysocket->sendMessage("pushButton_version_info","1");

}



#define TEXT_COLOR_RED(STRING)         "<font color=red>" STRING "</font>" "<font color=black> </font>"
#define TEXT_COLOR_BLUE(STRING)        "<font color=blue>" STRING "</font>" "<font color=black> </font>"
#define TEXT_COLOR_GREEN(STRING)        "<font color=green>" STRING "</font>" "<font color=black> </font>"




//改为由socket信息处理去调用了
void Widget::softwareversion_version_compare(QString version_tmp)
{
    //QString version_tmp = message;   //临时保存一下
    QString message;
    message.clear();    //用于发送给设备

    if(version_tmp.isEmpty())
        return;
    //计算本地目录中的镜像的md5，并得出结论
    QStringList mylist = version_tmp.split("\n");  //使用逗号分割
    int i;
    int need_update = 0;   //需要升级吗？0表示不需要，1表示需要

    mylist.removeAll(QString(""));

    ui->textBrowser_version_info->clear();  //清除显示

    for(i=0;i<mylist.length();i+=2)
    {
        qDebug()<< mylist[i];
        int bpos = mylist[i].indexOf(".");//从前面开始查找

        QString name = mylist[i].mid(bpos+1);  //保留点后面的这一段
        qDebug()<< "name = " << name;
        name.replace(":","");    //末尾的冒号去掉
        if(name.endsWith(".bin"))  //bin结尾的文件是单片机的，不需要计算md5，需要读取出来
        {
            int offset = 0;
            qDebug() <<"mcu:"<< name;
            if(name.contains("lcd"))
                offset = 0x6000 - 512;   //lcd单片机的偏移
            else if(name.contains("dg_keyboard"))
                offset = 0x5c00 - 512;   //按键的偏移
            else
                offset = 0x6000 - 512 + 8;   //话音接口板单片机，这个单片机的偏移多了8个字节

            QFile theFile("update/"+name);   //打开这个文件
            if(theFile.open(QIODevice::ReadOnly))
            {
                if(theFile.seek(offset))
                {
                    QString md5_read = theFile.read(32);   //读取32个字节
                    qDebug() <<"md5_read=" <<md5_read;

                    if(md5_read == mylist[i+1])  //对比md5
                    {
                        qDebug() << "md5一致";
                        ui->textBrowser_version_info->append(TEXT_COLOR_BLUE("*."+name+":md5一致,不需要升级"));
                        ui->textBrowser_version_info->append(md5_read+"\n");
                        message.append(TEXT_COLOR_BLUE("*."+name+":md5一致,不需要升级"));
                        message.append(md5_read+"\n");
                    }
                    else
                    {
                        qDebug() << "md5 不同 xxxx";
                        ui->textBrowser_version_info->append(TEXT_COLOR_GREEN("++."+name+":md5不同,可升级"));
                        ui->textBrowser_version_info->append("used:"+mylist[i+1]);
                        ui->textBrowser_version_info->append("update:"+md5_read+"\n");
                        need_update = 1;
                        message.append(TEXT_COLOR_GREEN("++."+name+":md5不同,可升级"));
                        message.append("used:"+mylist[i+1]);
                        message.append("update:"+md5_read+"\n");
                    }


                }
                else
                    qDebug() <<"seek error";

                theFile.close();
            }
        }
        else
        {
            //计算md5
            //name = "update/"+name;
            //qDebug() << name;
            QFile theFile("update/"+name);   //打开这个文件
            if(theFile.open(QIODevice::ReadOnly))
            {
                QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
                theFile.close();
                QString md5_calc = ba.toHex().constData();
                qDebug() <<"md5_calc ="<< md5_calc;
                qDebug() <<"mylist[i+1] ="<< mylist[i+1];
                if(md5_calc == mylist[i+1])  //对比md5
                {
                    qDebug() << "md5一致";
                    ui->textBrowser_version_info->append(TEXT_COLOR_BLUE("*."+name+":md5一致,不需要升级"));
                    ui->textBrowser_version_info->append(md5_calc+"\n");
                    message.append(TEXT_COLOR_BLUE("*."+name+":md5一致,不需要升级"));
                    message.append(md5_calc+"\n");
                }
                else
                {
                    qDebug() << "md5 不同 xxxx";
                    ui->textBrowser_version_info->append(TEXT_COLOR_GREEN("++."+name+":md5不同,可升级"));
                    ui->textBrowser_version_info->append("used:"+mylist[i+1]);
                    ui->textBrowser_version_info->append("update:"+md5_calc+"\n");
                    need_update = 1;
                    message.append(TEXT_COLOR_GREEN("++."+name+":md5不同,可升级"));
                    message.append("used:"+mylist[i+1]);
                    message.append("update:"+md5_calc+"\n");
                }
            }
            else
            {
                qDebug() << "升级文件不存在";
                ui->textBrowser_version_info->append(TEXT_COLOR_RED("--."+name+":升级文件不存在"));
                ui->textBrowser_version_info->append(mylist[i+1]+"\n");
                message.append(TEXT_COLOR_RED("--."+name+":升级文件不存在"));
                message.append(mylist[i+1]+"\n");
            }
        }

    }

    mysocket->sendMessage("version_compare_info",message);

    if(need_update)
    {
        //版本对比之后决定是否需要升级
        ui->pushButton_update->setEnabled(true);  //不需要升级
    }

}



//版本对比
void Widget::on_pushButton_version_compare_clicked()
{
    //1.查看是否有update的目录，目录中是否有文件
    QDir dir("./update");
    if(!dir.exists())   //目录不存在
    {
        qDebug() << "update 目录不存在";
        return ;
    }

    //没有目录，或者文件为空，不执行对比命令
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();

    if (list.count() <= 0)
    {
        qDebug() << "update 文件夹为空";
        return;
    }

    //存在update目录，并且里面有文件，则执行
    get_softwareversion_info();  //发出版本查询的指令
}




void Widget::software_packet()
{
//system("7za.exe a -ttar -so update.tar update/* | 7za.exe a -si update.tar.gz");
    QDir dir("./update");
    if(!dir.exists())   //目录不存在
    {
        qDebug() << "update 目录不存在";
        return ;
    }

    //没有目录，或者文件为空，不执行对比命令
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();

    if (list.count() <= 0)
    {
        qDebug() << "update 文件夹为空";
        return;
    }

    QProcess *process = NULL;

    QString  cmd_format = "7za.exe a -ttar update.tar \""; //生成归档文件

    cmd_format += "update/*";  //目录下的所有文件
    qDebug()<<"cmd = "<<cmd_format;
    process = new QProcess(this);

    process->start(cmd_format);  //压缩为service.tar
    process->waitForFinished(); //等待执行完成
    qDebug()<<"Result:"<<process->readAll();

    cmd_format = "7za.exe a -tgzip ";  //生成gz文件

    cmd_format += "./update.tar.gz";
    cmd_format += " update.tar";


    process->start(cmd_format);
    process->waitForFinished(); //等待执行完成
//    qDebug()<<"Result:"<<process->readAll();
    process->deleteLater();

    QFile::remove("update.tar"); //删除中间归档文件


    //计算md5
    QFile theFile("update.tar.gz");
    theFile.open(QIODevice::ReadOnly);
    QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
    theFile.close();
    qDebug() << ba.toHex().constData();

    QFile file("update.tar.gz.md5");
    //进行写文件
    file.open(QIODevice::WriteOnly); //用追加方式进行写
    file.write(ba.toHex().constData());
    file.close();
}



//发起升级的命令
void Widget::on_pushButton_update_clicked()
{
    if(update_command_wait)
        return;

    update_command_wait = 1;

    //完成软件打包工作。
    software_packet();

    ui->pushButton_update->setEnabled(false);
    mysocket->sendMessage("pushButton_update","1");  //发送命令
}




void Widget::on_radioButton_Spitest_clicked(bool checked)
{
    mysocket->sendMessage("radioButton_Spitest",QString::number(checked));   //把这个值发送过去
}



void Widget::on_radioButton_IICtest_clicked(bool checked)
{
    mysocket->sendMessage("radioButton_IICtest",QString::number(checked));   //把这个值发送过去
}



//只有被修改才会触发，在程序中setText修改不回触发
void Widget::on_lineEdit_ip1_textEdited(const QString &arg1)
{
    mysocket->sendMessage("lineEdit_ip1",arg1);   //把这个值发送过去
}

void Widget::on_lineEdit_ip2_textEdited(const QString &arg1)
{
    mysocket->sendMessage("lineEdit_ip2",arg1);   //把这个值发送过去
}

void Widget::on_lineEdit_ip3_textEdited(const QString &arg1)
{
    mysocket->sendMessage("lineEdit_ip3",arg1);   //把这个值发送过去
}




//
void Widget::on_checkBox_bigpack1_clicked(bool checked)
{
    mysocket->sendMessage("checkBox_bigpack1",QString::number(checked));   //把这个值发送过去
}

void Widget::on_checkBox_adap1_clicked(bool checked)
{
    mysocket->sendMessage("checkBox_adap1",QString::number(checked));   //把这个值发送过去
}

void Widget::on_checkBox_bigpack2_clicked(bool checked)
{
    mysocket->sendMessage("checkBox_bigpack2",QString::number(checked));   //把这个值发送过去
}

void Widget::on_checkBox_adap2_clicked(bool checked)
{
    mysocket->sendMessage("checkBox_adap2",QString::number(checked));   //把这个值发送过去
}

void Widget::on_checkBox_bigpack3_clicked(bool checked)
{
    mysocket->sendMessage("checkBox_bigpack3",QString::number(checked));   //把这个值发送过去
}

void Widget::on_checkBox_adap3_clicked(bool checked)
{
    mysocket->sendMessage("checkBox_adap3",QString::number(checked));   //把这个值发送过去
}

void Widget::on_radioButton_micpanel_clicked(bool checked)
{
    mysocket->sendMessage("radioButton_micpanel",QString::number(checked));   //把这个值发送过去
}

void Widget::on_radioButton_michand_clicked(bool checked)
{
    mysocket->sendMessage("radioButton_michand",QString::number(checked));   //把这个值发送过去
}



void Widget::on_radioButton_SpeakVol_clicked(bool checked)
{
    mysocket->sendMessage("radioButton_SpeakVol",QString::number(checked));   //把这个值发送过去
}

