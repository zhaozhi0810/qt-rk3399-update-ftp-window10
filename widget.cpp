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

    version_store_string.clear();  //清空

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

    buttonFrame = new QFrame;
    page2_show_color = 0;
    check_version_wait = 0;   //获取版本的时候时间比较长，防止多次按钮
    ui->pushButton_update->setEnabled(false);  //版本对比后决定是否升级
    //ui->pushButton_version_compare->setEnabled(true);
    update_command_wait = 0;  //升级，首先需要打包，需要一些时间等待，防止多次按钮
    is_test_press = 0;     //测试键没有按下
    key_light_connect = 1;  //键灯关联

    intValidator = new QIntValidator;
    intValidator->setRange(1,999999);
    ui->lineEdit_interval->setValidator(intValidator);

    QRegExp rx("^([1-9]|[1-9]\\d|(1[0-9]\\d)|(2[0-4]\\d)|(2[5][0-4]))$");//输入范围为【1-254】
    pReg = new QRegExpValidator(rx, this);
    ui->lineEdit_ip1->setValidator(pReg);
    ui->lineEdit_ip2->setValidator(pReg);
    ui->lineEdit_ip3->setValidator(pReg);
    ui->textBrowser_ifconfig->setVisible(false);//显示ip信息的暂时不可见


    lightpwm = 100;

    ui->label_ping_reson1->setText("");
    ui->label_ping_reson2->setText("");
    ui->label_ping_reson3->setText("");

    lcdPwm = 90;

    iicspi_connect = 0;

    ui->toolButton_left->setVisible(false);
    ui->toolButton_right->setVisible(false);
    ui->toolButton_ok->setVisible(false);
    ui->label_ftp_stat->setText("");
    //mysocket->ui->label_ftp_stat1->setText("");
    // *TODO: Allow using port 0.
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
//    delete myftp_server;

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
    qDebug()<<"Palette_button ison="<<ison <<" key" << keyval;
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


                qDebug()<<"setPalette i=" << i << key_buttons[i];
#ifdef RK_3399_PLATFORM
//                qDebug() << " i = " <<i;
//                qDebug() << " key_map i = " << led_key_map[i];
//                if(ison && key_light_connect)
//                    drvLightLED(led_key_map[i]);
//                else
//                    drvDimLED(led_key_map[i]);
#endif
                break;
            }
        }
    }


}



#if 0
//网络测试页，定时检测网络状态,500ms
void Widget::timer_net_stat_slot_Function()
{
    int i;
//    QLabel* Ping_stat[3] = {ui->label_ping_stat1,ui->label_ping_stat2,ui->label_ping_stat3};
    //getNetDeviceStats();

    for(i=0;i<3;i++)
    {
        if(ping_status[i]) //已经开始ping
        {
            if(icmp_saved[i] != icmp_cur[i])
            {
                icmp_saved[i] = icmp_cur[i];
            }
            else
            {
//                if(Ping_stat[i]->text() != "异常")
//                {
//                    Ping_stat[i]->setText("异常");
//                    Ping_stat[i]->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 20pt \"Ubuntu\";}");
//                }
            }
        }
    }
}

#endif



//label_color
bool Widget::eventFilter(QObject *obj, QEvent *event)
{
#if 1
    obj = obj;

    if(event->type() == QEvent::MouseButtonRelease)
    {
        if((ui->stackedWidget->currentIndex() == 2))
        {
            QMouseEvent *e  = (QMouseEvent*)(event);
            QPoint sPoint2= e->pos();
     //       qDebug() << "sPoint2: " <<sPoint2.rx();
            if(sPoint2.rx() < 350)
            {
                last_color_page_show();
            }
            else if(sPoint2.rx() > 370)
            {
                next_color_page_show();
            }
        }
        return true;
    }
    else if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *KeyEvent = static_cast<QKeyEvent*>(event);


        if(ui->stackedWidget->currentIndex() == 0) //第一页，按键测试页
        {
            Palette_button(1,KeyEvent->key());
        }
 #if 0
        else if(ui->stackedWidget->currentIndex() == 2)//lcd颜色测试页，按键变换颜色
        {
            if(KeyEvent->key() == Qt::Key_Up)
            {
                if(ui->label_color->isVisible())
                {
                    last_color_page_show();
                }
                else
                {
                    lcdPwm = ui->horizontalScrollBar_light->value();
                    if(lcdPwm < 100)
                    {
                        lcdPwm += 5;
                        if(lcdPwm > 100)
                            lcdPwm = 100;
                        ui->horizontalScrollBar_light->setValue(lcdPwm);
                     //   ui->label_light_val->setText(QString::number(lcdPwm));
                    //    qDebug() << "lcdPwm = " << lcdPwm;
                    }
                }
            }
            else if(KeyEvent->key() == Qt::Key_Down)
            {
                if(ui->label_color->isVisible())
                {
                    next_color_page_show();
                }
                else
                {
                    lcdPwm = ui->horizontalScrollBar_light->value();
                    if(lcdPwm > 0)
                    {
                        lcdPwm -= 5;
                        if(lcdPwm < 0)
                            lcdPwm = 0;
                        ui->horizontalScrollBar_light->setValue(lcdPwm);
                     //   ui->label_light_val->setText(QString::number(lcdPwm));
                     //   qDebug() << "lcdPwm = " << lcdPwm;
                    }
                }
            }
            else if(KeyEvent->key() == Qt::Key_C)
            {
                on_pushButton_start_color_test_clicked();
            }
        }
        else if(ui->stackedWidget->currentIndex() == 1)  //键灯测试页
        {
            if(KeyEvent->key() == Qt::Key_F1)  // L1    Qt::Key_F1
            {
                ui->lineEdit_interval->backspace();
            }
            else if(KeyEvent->key() == Qt::Key_F3)  // L2    Qt::Key_F3
            {
                on_pushButton_6_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F5)  // L3    Qt::Key_F5
            {
                on_pushButton_7_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F7)  // L4    Qt::Key_F7
            {
                on_pushButton_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_Up)
            {
                if(lightpwm < 100){
                    lightpwm +=5;
                    if(lightpwm > 100)
                        lightpwm = 100;
                 }
                ui->verticalScrollBar_lightpwm2->setValue(lightpwm);

            }
            else if(KeyEvent->key() == Qt::Key_Down)
            {
                if(lightpwm > 0){
                    lightpwm -=5;
                    if(lightpwm < 0 )
                        lightpwm = 0;
                 }
                ui->verticalScrollBar_lightpwm2->setValue(lightpwm);//drvSetLedBrt(lightpwm);
            }
        }

        else if(ui->stackedWidget->currentIndex() == 3)  //网络测试页
        {
            if(KeyEvent->key() == Qt::Key_F1)  // L1    Qt::Key_F1
            {
                if(ui->lineEdit_ip1 == qobject_cast<QLineEdit*>(ui->stackedWidget->focusWidget()))
                {
                    ui->lineEdit_ip1->backspace();
                }
                else if(ui->lineEdit_ip2 == qobject_cast<QLineEdit*>(ui->stackedWidget->focusWidget()))
                {
                    ui->lineEdit_ip2->backspace();
                }
                else if(ui->lineEdit_ip3 == qobject_cast<QLineEdit*>(ui->stackedWidget->focusWidget()))
                {
                    ui->lineEdit_ip3->backspace();
                }

            }
            else if(KeyEvent->key() == Qt::Key_F5)
            {
                if(ui->pushButton_2->isEnabled())
                    on_pushButton_2_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F7)
            {
                if(ui->pushButton_4->isEnabled())
                    on_pushButton_4_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F9)
            {
                if(ui->pushButton_5->isEnabled())
                    on_pushButton_5_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F11)   //内通按键 配置ip
            {
                on_pushButton_3_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F12)    //外通按键，查看ip
            {
                on_pushButton_ifconfig_clicked();
            }
            else if(KeyEvent->key() == Qt::Key_F2)
            {
                if(ui->lineEdit_ip1 == qobject_cast<QLineEdit*>(ui->stackedWidget->focusWidget()))
                {
                    ui->lineEdit_ip2->setFocus();
                }
                else if(ui->lineEdit_ip2 == qobject_cast<QLineEdit*>(ui->stackedWidget->focusWidget()))
                {
                    ui->lineEdit_ip3->setFocus();
                }
                else if(ui->lineEdit_ip3 == qobject_cast<QLineEdit*>(ui->stackedWidget->focusWidget()))
                {
                    ui->lineEdit_ip3->clearFocus();
                }
                else
                {
                    ui->lineEdit_ip3->setFocus();
                }

            }
            else if(KeyEvent->key() == Qt::Key_F6)    //调整大包，自适应
            {
                if(!ping_status[0])
                {
                    if(!ui->checkBox_bigpack1->isChecked() && !ui->checkBox_adap1->isChecked())
                        ui->checkBox_bigpack1->setChecked(true);
                    else if(ui->checkBox_bigpack1->isChecked() && !ui->checkBox_adap1->isChecked())
                        ui->checkBox_adap1->setChecked(true);
                    else if(ui->checkBox_bigpack1->isChecked() && ui->checkBox_adap1->isChecked())
                        ui->checkBox_bigpack1->setChecked(false);
                    else if(!ui->checkBox_bigpack1->isChecked() && ui->checkBox_adap1->isChecked())
                        ui->checkBox_adap1->setChecked(false);
                }
            }
            else if(KeyEvent->key() == Qt::Key_F8)    //调整大包，自适应
            {
                if(!ping_status[1])
                {
                    if(!ui->checkBox_bigpack2->isChecked() && !ui->checkBox_adap2->isChecked())
                        ui->checkBox_bigpack2->setChecked(true);
                    else if(ui->checkBox_bigpack2->isChecked() && !ui->checkBox_adap2->isChecked())
                        ui->checkBox_adap2->setChecked(true);
                    else if(ui->checkBox_bigpack2->isChecked() && ui->checkBox_adap2->isChecked())
                        ui->checkBox_bigpack2->setChecked(false);
                    else if(!ui->checkBox_bigpack2->isChecked() && ui->checkBox_adap2->isChecked())
                        ui->checkBox_adap2->setChecked(false);
                }
            }
            else if(KeyEvent->key() == Qt::Key_F10)    //调整大包，自适应
            {
                if(!ping_status[2])
                {
                    if(!ui->checkBox_bigpack3->isChecked() && !ui->checkBox_adap3->isChecked())
                        ui->checkBox_bigpack3->setChecked(true);
                    else if(ui->checkBox_bigpack3->isChecked() && !ui->checkBox_adap3->isChecked())
                        ui->checkBox_adap3->setChecked(true);
                    else if(ui->checkBox_bigpack3->isChecked() && ui->checkBox_adap3->isChecked())
                        ui->checkBox_bigpack3->setChecked(false);
                    else if(!ui->checkBox_bigpack3->isChecked() && ui->checkBox_adap3->isChecked())
                        ui->checkBox_adap3->setChecked(false);
                }
            }
        }
        else if(ui->stackedWidget->currentIndex() == 4)//音频测试页
        {
            int val;
            if(KeyEvent->key() == Qt::Key_F1)
            {
                if(is_test_press == 1)
                {
                    if(ui->radioButton_SpeakVol->isChecked())
                    {
                        ui->radioButton_SpeakVol->setChecked(false);
#ifdef RK_3399_PLATFORM
                        //drvSetSpeakVolume(value);
#endif
                    }
                    else
                    {
                        ui->radioButton_SpeakVol->setChecked(true);
#ifdef RK_3399_PLATFORM
                        //drvSetSpeakVolume(value);
#endif
                    }
                }
                else
                {
                    val = ui->horizontalScrollBar_SpeakVol->value();
                    if(val > 0)
                    {
                        val -= 5;
                        if(val < 0)
                            val = 0;
                    }
                    ui->horizontalScrollBar_SpeakVol->setValue(val);
                }

            }
            else if(KeyEvent->key() == Qt::Key_F3)
            {
                if(is_test_press == 1)
                {
                    if(ui->radioButton_HandVol->isChecked())
                    {
                        ui->radioButton_HandVol->setChecked(false);
#ifdef RK_3399_PLATFORM
                        drvEnableHandout();
#endif
                    }
                    else
                    {
                        ui->radioButton_HandVol->setChecked(true);
#ifdef RK_3399_PLATFORM
                        drvDisableHandout();
#endif
                    }
                }
                else
                {
                    val = ui->horizontalScrollBar_HandVol->value();
                    if(val > 0)
                    {
                        val -= 5;
                        if(val < 0)
                            val = 0;
                    }
                    ui->horizontalScrollBar_HandVol->setValue(val);
                }
            }
            else if(KeyEvent->key() == Qt::Key_F5)
            {
                if(is_test_press == 1)
                {
                    if(ui->radioButton_EarphVol->isChecked())
                    {
                        ui->radioButton_EarphVol->setChecked(false);
#ifdef RK_3399_PLATFORM
                        drvEnableEarphout();
#endif
                    }
                    else
                    {
                        ui->radioButton_EarphVol->setChecked(true);
#ifdef RK_3399_PLATFORM
                        drvDisableEarphout();
#endif
                    }
                }
                else
                {
                    val = ui->horizontalScrollBar_EarphVol->value();
                    if(val > 0)
                    {
                        val -= 5;
                        if(val < 0)
                            val = 0;
                    }
                    ui->horizontalScrollBar_EarphVol->setValue(val);
                }
            }
            else if(KeyEvent->key() == Qt::Key_F2)
            {
                val = ui->horizontalScrollBar_SpeakVol->value();
                if(val < 100)
                {
                    val += 5;
                    if(val > 100)
                        val = 100;
                }
                ui->horizontalScrollBar_SpeakVol->setValue(val);

            }
            else if(KeyEvent->key() == Qt::Key_F4)
            {
                val = ui->horizontalScrollBar_HandVol->value();
                if(val < 100)
                {
                    val += 5;
                    if(val > 100)
                        val = 100;
                }
                ui->horizontalScrollBar_HandVol->setValue(val);
            }
            else if(KeyEvent->key() == Qt::Key_F6)
            {
                val = ui->horizontalScrollBar_EarphVol->value();
                if(val < 100)
                {
                    val += 5;
                    if(val > 100)
                        val = 100;
                }
                ui->horizontalScrollBar_EarphVol->setValue(val);
            }
            else if(KeyEvent->key() == Qt::Key_F9)
            {
                if(ui->radioButton_rec->isChecked())
                {
                    ui->radioButton_loop->setChecked(true);
                    ui->radioButton_rec->setChecked(false);

                }
                else if(ui->radioButton_loop->isChecked())
                {
                    ui->radioButton_loop->setChecked(false);
                    ui->radioButton_playrec->setChecked(true);
                    ui->radioButton_michand->setEnabled(false);
                    ui->radioButton_micpanel->setEnabled(false);
                }
                else if(ui->radioButton_playrec->isChecked())
                {
                    ui->radioButton_playrec->setChecked(false);
                    ui->radioButton_playmusic->setChecked(true);

                }
                else if(ui->radioButton_playmusic->isChecked())
                {
                    ui->radioButton_playmusic->setChecked(false);
                    ui->radioButton_rec->setChecked(true);
                    ui->radioButton_michand->setEnabled(true);
                    ui->radioButton_micpanel->setEnabled(true);
                }
            }
            else if(KeyEvent->key() == Qt::Key_F10)
            {
                if(ui->radioButton_rec->isChecked() || ui->radioButton_loop->isChecked())
                {
                    if(ui->radioButton_michand->isChecked())
                    {
                        ui->radioButton_michand->setChecked(false);
                        ui->radioButton_micpanel->setChecked(true);
#ifdef RK_3399_PLATFORM
                        drvSelectHandFreeMic();
#endif
                    }
                    else if(ui->radioButton_micpanel->isChecked())
                    {
                        ui->radioButton_micpanel->setChecked(false);
                        ui->radioButton_michand->setChecked(true);
#ifdef RK_3399_PLATFORM
                        drvSelectHandMic();
#endif
                    }
                }
            }
            else if(KeyEvent->key() == Qt::Key_C)
            {
                on_pushButton_Play_clicked();
            }

        }
//        else if(ui->stackedWidget->currentIndex() == 5)//触摸屏测试页
//        {
//            if(KeyEvent->key() == Qt::Key_C) //拨号/电话键
//            {
//                on_pushButton_start_lcd_touch_clicked();
//            }
//        }



        if(is_test_press == 1)  //判断是否有组合键功能
        {
            if(KeyEvent->key() >= Qt::Key_1 && KeyEvent->key() <= Qt::Key_9)
            {
                stackedWidget_page_show(KeyEvent->key() - Qt::Key_1);
            }
            else if(KeyEvent->key() == Qt::Key_Up)
            {
                 last_func_page_show();
            }
            else if(KeyEvent->key() == Qt::Key_Down)
            {
                next_func_page_show();
            }
            else if(KeyEvent->key() == Qt::Key_Slash)   //测试 + #
            {
                if(ui->stackedWidget->currentIndex() == 0)
                {
                    if(ui->checkBox->isChecked())
                    {
                        ui->checkBox->setChecked(false);
                    }
                    else
                        ui->checkBox->setChecked(true);
                }
            }
            else if(KeyEvent->key() == Qt::Key_P)   //测试 + ptt
            {
                system("reboot");
            }
        }


        if(KeyEvent->key() == Qt::Key_Control)
            is_test_press = 1;     //测试键按下
#endif
        return true;
    }
    else if(event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *KeyEvent = static_cast<QKeyEvent*>(event);
        if(KeyEvent->key() == Qt::Key_Control)
            is_test_press = 0;     //测试键没有按下

        if((ui->stackedWidget->currentIndex() == 0))
        {
 //           Palette_button(0,KeyEvent->key());
        }
        return true;\
    }
    return false;
#endif
}



//下一项
void Widget::stackedWidget_page_show(int index)
{
#ifdef RK_3399_PLATFORM
//    if(index == 0)
//        drvDimAllLED();
#endif
    //qDebug() << "enter stackedWidget_page_show = " << index;
    if(index < g_show_title.count())
        ui->label_Page_title->setText(g_show_title.at(index));

    if(index == 3){  //进入网络测试页，开启定时器
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
    }


    if(index == 7)
    {
        page9_info_show();
    }

    if(index != 5)
    {

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
    //qDebug() << "go out stackedWidget_page_show = " << index;
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
//    qDebug() << "on_pushButton_P3_l_clicked";
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


//page3 right
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








//开始触摸测试
void Widget::on_pushButton_start_lcd_touch_clicked()
{
//    lcd_touch_ui = new fingerpaint(this);
    lcd_touch_ui->show();
}


void Widget::play_finished_slot(int ret)
{
    Q_UNUSED(ret);

//    qDebug()  << "play_finished_slot";

    ui->radioButton_rec->setEnabled(true);
    ui->radioButton_loop->setEnabled(true);
    ui->radioButton_playrec->setEnabled(true);
    ui->radioButton_playmusic->setEnabled(true);
    if(ui->radioButton_rec->isChecked() || ui->radioButton_loop->isChecked() )
    {
        ui->radioButton_michand->setEnabled(true);
        ui->radioButton_micpanel->setEnabled(true);
    }
    ui->pushButton_Play->setText("开始(拨号键)");

}




//音频测试页：播放按键
void Widget::on_pushButton_Play_clicked()
{
    static int loop_flag = 0;
    QString cmd;

    if(ui->pushButton_Play->text() == "开始(拨号键)"){
        ui->pushButton_Play->setStyleSheet("QPushButton{background-color:#ff0000;font: 20pt \"Ubuntu\";}");
        if(ui->radioButton_rec->isChecked())
        {
            cmd = "arecord -f cd  /home/deepin/test.wav";
            qDebug()  << "录音测试" ;
            loop_flag = 0;
        }
        else if(ui->radioButton_loop->isChecked())
        {
#ifdef RK_3399_PLATFORM
#if 1
            myprocess_play1[0]->start("arecord  -f cd");
            myprocess_play1[1]->start("aplay");
#else
            myprocess_play->start("i2cset -f -y 4 0x10 39 0x40");
            myprocess_play->waitForFinished();
#endif
#endif
            loop_flag = 1;
            qDebug()  << "回环测试" ;
        }
        else if(ui->radioButton_playrec->isChecked())
        {
            cmd = "aplay /home/deepin/test.wav";
            qDebug()  << "播放录音" ;
            loop_flag = 0;
        }
        else if(ui->radioButton_playmusic->isChecked())
        {
            cmd = "aplay /home/deepin/123.wav";
            qDebug()  << "播放音乐" ;
            loop_flag = 0;
        }

#ifdef RK_3399_PLATFORM
        qDebug()<<"cmd = " << cmd;
        if(!loop_flag)
        {           
            myprocess_play->start(cmd,QIODevice::ReadOnly);//ReadOnly,ReadWrite
        }

#endif
        ui->radioButton_rec->setEnabled(false);
        ui->radioButton_loop->setEnabled(false);
        ui->radioButton_playrec->setEnabled(false);
        ui->radioButton_playmusic->setEnabled(false);
        ui->radioButton_michand->setEnabled(false);
        ui->radioButton_micpanel->setEnabled(false);
        ui->pushButton_Play->setText("结束");
    }
    else{
        ui->pushButton_Play->setStyleSheet("QPushButton{background-color:#00ff00;font: 20pt \"Ubuntu\";}");
#ifdef RK_3399_PLATFORM
        if(myprocess_play->state()==QProcess::Running)
            myprocess_play->kill();
#else
        play_finished_slot(0);
#endif
        if(loop_flag)//子进程杀不死，暂时这么处理吧
        {
#ifdef RK_3399_PLATFORM
#if 1
            if(myprocess_play1[0]->state()==QProcess::Running)
                myprocess_play1[0]->kill();
            if(myprocess_play1[1]->state()==QProcess::Running)
                myprocess_play1[1]->kill();
#else
            myprocess_play->start("i2cset -f -y 4 0x10 39 0x80");
            myprocess_play->waitForFinished();
#endif
#endif
            loop_flag = 0;
        }

    }
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





static bool isipAddr_sameSegment(const QString & ip1,const QString & ip2)
{
    int i,j;

    if (ip1.isEmpty() || ip1.isEmpty())
    {
        return false;
    }

    i = ip1.lastIndexOf('.');
    j = ip2.lastIndexOf('.');

    if(i == j)  //长度是否相同
    {
        QString str1 = ip1.mid(0,i);
        QString str2 = ip2.mid(0,i);

        if(str1 == str2)  //相等吗
        {
            return true;
        }

    }

}


#if 0
void Widget::getNetDeviceStats()
{
    QList<QNetworkInterface> list;
//    QList<QNetworkAddressEntry> list_addrs;
    QNetworkInterface intf;
    list = QNetworkInterface::allInterfaces(); //获取系统里所有的网卡对象



    for (int i = 0; i < list.size(); i++)
    {
        intf = list.at(i);
        if(intf.name() == "lo")
            continue;
        if(intf.flags() & intf.IsRunning){
            if(enp1_dev && intf.name() == "enp1s0f0"){
                ui->label_Net_Stat1->setText("已连接");
                ui->label_Net_Stat1->setStyleSheet("QLabel{background-color:#00ff00;border-radius:5px;font: 20pt \"Ubuntu\";}");
                if(isipAddr_sameSegment(intf.addressEntries().at(0).ip().toString(),"192.168.0.200"))
                {
                    ui->pushButton_2->setEnabled(true);
                    ui->label_ping_reson1->setText("");
                }
                else
                {                    ui->pushButton_2->setEnabled(false);
                    ui->label_ping_reson1->setText("设备ip与配测计算机网段不同，请点击\"配置rk3399主板IP\"按钮");
                }
            }
            else if(enp2_dev && intf.name() == "enp1s0f1"){
                ui->label_Net_Stat2->setText("已连接");
                ui->label_Net_Stat2->setStyleSheet("QLabel{background-color:#00ff00;border-radius:5px;font: 20pt \"Ubuntu\";}");
                if(isipAddr_sameSegment(intf.addressEntries().at(0).ip().toString(),"192.168.1.200"))
                {
                    ui->pushButton_4->setEnabled(true);
                    ui->label_ping_reson2->setText("");
                }
                else
                {
                    ui->pushButton_4->setEnabled(false);
                    ui->label_ping_reson2->setText("设备ip与配测计算机网段不同，请点击\"配置rk3399主板IP\"按钮");
                }
            }
            else if((eth2_dev && intf.name() == "eth2") || (eth0_dev && intf.name() == "eth0")){
                ui->label_Net_Stat3->setText("已连接");
                ui->label_Net_Stat3->setStyleSheet("QLabel{background-color:#00ff00;border-radius:5px;font: 20pt \"Ubuntu\";}");
                if(isipAddr_sameSegment(intf.addressEntries().at(0).ip().toString(),"192.168.2.200"))
                {
                    ui->pushButton_5->setEnabled(true);
                    ui->label_ping_reson3->setText("");
                }
                else
                {
                    ui->pushButton_5->setEnabled(false);
                    ui->label_ping_reson3->setText("设备ip与配测计算机网段不同，请点击\"配置rk3399主板IP\"按钮");
                }
            }
        }
        else
        {
            if(enp1_dev && intf.name() == "enp1s0f0"){
                if(ui->label_Net_Stat1->text() != "已断开")
                {
                    ui->label_Net_Stat1->setText("已断开");
                    ui->label_Net_Stat1->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 20pt \"Ubuntu\";}");
                    ui->pushButton_2->setEnabled(false);
                    ui->label_ping_reson1->setText("网线已断开，请连接网线");
                    if(ping_status[0])
                    {
                        terminate_ping1();
                    }
                }
            }
            else if(enp2_dev && intf.name() == "enp1s0f1"){
                if(ui->label_Net_Stat2->text() != "已断开")
                {
                    ui->label_Net_Stat2->setText("已断开");
                    ui->label_Net_Stat2->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 20pt \"Ubuntu\";}");
                    ui->pushButton_4->setEnabled(false);
                    ui->label_ping_reson2->setText("网线已断开，请连接网线");
                    if(ping_status[1])
                    {
                        terminate_ping2();
                    }
                }
            }
            else if((eth2_dev && intf.name() == "eth2") || (eth0_dev && intf.name() == "eth0")){
                if(ui->label_Net_Stat3->text() != "已断开")
                {
                    ui->label_Net_Stat3->setText("已断开");
                    ui->label_Net_Stat3->setStyleSheet("QLabel{background-color:#ff0000;border-radius:5px;font: 20pt \"Ubuntu\";}");
                    ui->pushButton_5->setEnabled(false);
                    ui->label_ping_reson3->setText("网线已断开，请连接网线");
                    if(ping_status[2])
                    {
                        terminate_ping3();
                    }
                }
            }
        }
    }
}




void Widget::terminate_ping1()
{
//    if(myprocess_ping[0]->state()==QProcess::Running)
//        myprocess_ping[0]->kill();

    ui->pushButton_2->setText("ping enp1s0f0");
    ui->checkBox_bigpack1->setEnabled(true);
    ui->checkBox_adap1->setEnabled(true);
    ping_status[0] = false;
}

void Widget::terminate_ping2()
{
//    if(myprocess_ping[1]->state()==QProcess::Running)
//        myprocess_ping[1]->kill();

    ui->pushButton_4->setText("ping enp1s0f1");
    ui->checkBox_bigpack2->setEnabled(true);
    ui->checkBox_adap2->setEnabled(true);
    ping_status[1] = false;

}
void Widget::terminate_ping3()
{
//    if(myprocess_ping[2]->state()==QProcess::Running)
//        myprocess_ping[2]->kill();

    if(eth2_dev)
        ui->pushButton_5->setText("ping eth2");
    else if(eth0_dev)
        ui->pushButton_5->setText("ping eth0");
    ui->checkBox_bigpack3->setEnabled(true);
    ui->checkBox_adap3->setEnabled(true);
    ping_status[2] = false;

}
#endif



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

            if(myList[i].contains("Host Unreachable", Qt::CaseInsensitive))
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




void Widget::ping1_info_show()
{
//    QString strMsg = myprocess_ping[0]->readAllStandardOutput();
//    ping_info_show(strMsg,0);
}


void Widget::ping2_info_show()
{
//    QString strMsg = myprocess_ping[1]->readAllStandardOutput();
//    ping_info_show(strMsg,1);
}


void Widget::ping3_info_show()
{
//    QString strMsg = myprocess_ping[2]->readAllStandardOutput();
//    ping_info_show(strMsg,2);
}


//void Widget::ifconfig_errinfo_show()
//{
//    QString strMsg = myprocess_ping1->readAllStandardError();
//    qDebug() <<"error: "<< strMsg;
//}



//ping 进程结束
void Widget::ping1_finished_slot(int ret)
{
    Q_UNUSED(ret);
//    if(myprocess_ping[0] != nullptr){
//        delete myprocess_ping[0];
//        myprocess_ping[0] = nullptr;
//    }
}

void Widget::ping2_finished_slot(int ret)
{
    Q_UNUSED(ret);
//    if(myprocess_ping[1] != nullptr){
//        delete myprocess_ping[1];
//        myprocess_ping[1] = nullptr;
//    }
}


void Widget::ping3_finished_slot(int ret)
{
    Q_UNUSED(ret);
//    if(myprocess_ping[2] != nullptr){
//        delete myprocess_ping[2];
//        myprocess_ping[2] = nullptr;
//    }
}






//网络测试页 ： ping enp1s0f0
//void Widget::ping_pushButton_function(int ping_num)
//{
//    QString ping_str = "ping 192.168.0.200 -A ";
//    QPushButton* button[3] = {ui->pushButton_2,ui->pushButton_4,ui->pushButton_5};
//    QCheckBox* box[3] = {ui->checkBox_bigpack1,ui->checkBox_bigpack2,ui->checkBox_bigpack3};
//    ping_finished_slot_func_t pfunc[3] = {&Widget::ping1_finished_slot,&Widget::ping2_finished_slot,&Widget::ping3_finished_slot};
//    ping1_info_show_func_t show_func[3] = {&Widget::ping1_info_show,&Widget::ping2_info_show,&Widget::ping3_info_show};
//    QString str[3] = {"ping enp1s0f0","ping enp1s0f1","ping eth2"};

//    if(ping_num < 0 || ping_num > 2)
//        return;


//    if(button[ping_num]->text() == str[ping_num])
//    {
//        if(box[ping_num]->isChecked())
//        {
//            ping_str += " -s 65500 ";
//        }
//        myprocess_ping[ping_num] = new QProcess;
//        myprocess_ping[ping_num]->start(ping_str,QIODevice::ReadOnly);
//        button[ping_num]->setText("结束 ping");
//        box[ping_num]->setEnabled(false);
//        ping_status[ping_num] = true;
//        error_count[ping_num] = 0;
//        connect(this->myprocess_ping[ping_num], SIGNAL(readyReadStandardOutput()),this,SLOT(show_func[ping_num]()));//连接信号
////        connect(this->myprocess_ping1, SIGNAL(readyReadStandardError()),this,SLOT(ping1_errinfo_show()));//连接信号
//        connect(this->myprocess_ping[ping_num], SIGNAL(finished(int)),this,SLOT(pfunc[ping_num](int)));//连接信号
//    }
//    else
//    {
//        terminate_ping1();
//    }
//}



//网络测试页 ： ping enp1s0f0
void Widget::on_pushButton_2_clicked()
{
    mysocket->sendMessage("pushButton_2","1");
#if 0
    QString ping_str = "ping 192.168.0.200 ";
    if(ui->pushButton_2->text() == "ping enp1s0f0")
    {
        if(ui->checkBox_bigpack1->isChecked())
        {
            ping_str += " -s 65500 ";
        }
        if(ui->checkBox_adap1->isChecked())
        {
            ping_str += " -A ";
        }
//        myprocess_ping[0]->start(ping_str,QIODevice::ReadOnly);
        ui->pushButton_2->setText("结束 ping");
        ui->checkBox_bigpack1->setEnabled(false);
        ui->checkBox_adap1->setEnabled(false);
        ping_status[0] = true;
        error_count[0] = 0;
        ui->label_ping_err1->setText("0");

    }
    else
    {
 //       terminate_ping1();
    }
#endif
}


//网络测试页 ： ping enp1s0f1
void Widget::on_pushButton_4_clicked()
{
    mysocket->sendMessage("pushButton_4","1");
#if 0
    QString ping_str = "ping 192.168.1.200 ";
    if(ui->pushButton_4->text() == "ping enp1s0f1")
    {
        if(ui->checkBox_bigpack2->isChecked())
        {
            ping_str += " -s 65500 ";
        }
        if(ui->checkBox_adap2->isChecked())
        {
            ping_str += " -A ";
        }
//        myprocess_ping[1]->start(ping_str,QIODevice::ReadOnly);
        ui->pushButton_4->setText("结束 ping");
        ui->checkBox_bigpack2->setEnabled(false);
        ui->checkBox_adap2->setEnabled(false);
        ping_status[1] = true;
        error_count[1] = 0;
        ui->label_ping_err2->setText("0");
    }
    else
    {
//        terminate_ping2();
    }
#endif
}


//网络测试页 ： ping eth2
void Widget::on_pushButton_5_clicked()
{
    mysocket->sendMessage("pushButton_5","1");
#if 0
    QString ping_str = "ping 192.168.2.200 ";
    if(ui->pushButton_5->text() == "ping eth2" || ui->pushButton_5->text() == "ping eth0")
    {
        if(ui->checkBox_bigpack3->isChecked())
        {
            ping_str += " -s 65500 ";
        }
        if(ui->checkBox_adap3->isChecked())
        {
            ping_str += " -A ";
        }

//        myprocess_ping[2]->start(ping_str,QIODevice::ReadOnly);
        ui->pushButton_5->setText("结束 ping");
        ui->checkBox_bigpack3->setEnabled(false);
        ui->checkBox_adap3->setEnabled(false);
        ping_status[2] = true;
        error_count[2] = 0;
        ui->label_ping_err3->setText("0");
    }
    else
    {
//        terminate_ping3();
    }
#endif
}


//音频测试页： 扬声器音量调整滑动

void Widget::on_horizontalScrollBar_SpeakVol_valueChanged(int value)
{
//    qDebug()<<"扬声器音量 " << value  ;
    value = value;
#ifdef RK_3399_PLATFORM
    drvSetSpeakVolume(value);
#endif
}


//音频测试页： 手柄音量调整滑动
void Widget::on_horizontalScrollBar_HandVol_valueChanged(int value)
{
    value = value;
//    qDebug()<<"手柄音量" << value  ;
#ifdef RK_3399_PLATFORM
    drvSetHandVolume(value);
#endif
}

//音频测试页： 耳机音量调整滑动
void Widget::on_horizontalScrollBar_EarphVol_valueChanged(int value)
{
    value = value;
//    qDebug()<<"耳机音量" << value  ;
#ifdef RK_3399_PLATFORM
    drvSetEarphVolume(value);
#endif
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
#ifdef RK_3399_PLATFORM
    int ip_val;
    int ret;
    if(ui->lineEdit_ip1->text().isEmpty())
        ui->lineEdit_ip1->setText("100");
    if(ui->lineEdit_ip2->text().isEmpty())
        ui->lineEdit_ip2->setText("100");
    if(ui->lineEdit_ip3->text().isEmpty())
        ui->lineEdit_ip3->setText("100");
//    system("nmcli con delete eth2");
//    system("nmcli con delete eth0");
#if 0
    QString cmd;
    QByteArray ba;
    char* ch;
//    nmcli con add type ethernet con-name enp1s0f0 ifname enp1s0f0 ip4 192.168.0.55/24 gw4 192.168.0.1 cloned-mac 02:02:03:04:05:06
//    nmcli con add type ethernet con-name enp1s0f1 ifname enp1s0f1 ip4 168.75.45.30/22 gw4 168.75.44.1 cloned-mac 02:02:03:04:05:07
//    nmcli con add type ethernet con-name eth2 ifname eth2 ip4 192.168.2.100/24 gw4 192.168.2.1 cloned-mac 1a:0f:e8:2f:e7:5f

    if(enp1_dev)
    {
        cmd = "nmcli con add type ethernet con-name enp1s0f0 ifname enp1s0f0 ip4 192.168.0."+ ui->lineEdit_ip1->text() + "/24 gw4 192.168.0.1 cloned-mac 02:02:03:04:05:06";
        qDebug() << "cmd0 = " << cmd;
        ba = cmd.toLatin1();
        ret = system(ba.data());
    }

    if(enp2_dev)
    {
        cmd = "nmcli con add type ethernet con-name enp1s0f1 ifname enp1s0f1 ip4 192.168.1."+ui->lineEdit_ip1->text() +"/24 gw4 192.168.1.1 cloned-mac 02:02:03:04:05:06";
        qDebug() << "cmd1 = " << cmd;
        ba = cmd.toLatin1();
        ret = system(ba.data());
    }

    if(eth2_dev)
    {
        cmd = "nmcli con add type ethernet con-name  eth2  ifname eth2 ip4 192.168.2."+ui->lineEdit_ip1->text() +"/24 gw4 192.168.2.1 cloned-mac 1a:0f:e8:2f:e7:5f";
        qDebug() << "cmd2 = " << cmd;
        ba = cmd.toLatin1();
        ret = system(ba.data());
    }
    else if(eth0_dev)
    {
        cmd = "nmcli con add type ethernet con-name eth0  ifname eth0 ip4 192.168.2."+ui->lineEdit_ip1->text( )+"/24 gw4 192.168.2.1 cloned-mac 1a:0f:e8:2f:e7:5f";
        qDebug() << "cmd3 = " << cmd;
        ba = cmd.toLatin1();
        ret = system(ba.data());
    }

#else

    char const *  str_name[] = {"enp1s0f0","enp1s0f1","eth2","eth0"};
    char num[] = {'0','1','2'};
    char const * str1_ipset = "nmcli connection modify --temporary '%s' connection.autoconnect yes ipv4.method manual ipv4.address 192.168.%c.%s/24 ipv4.gateway 192.168.%c.1  ipv4.dns 114.114.114.114";
    char cmd[256] = {0};


    if(enp1_dev)
    {
        sprintf(cmd,str1_ipset,str_name[0],num[0],ui->lineEdit_ip1->text().toLatin1().data(),num[0]);
    //    qDebug() << "cmd0 = " << cmd;
        ret = system(cmd);
        //ret = system("nmcli connection modify --temporary 'enp1s0f0' connection.autoconnect yes ipv4.method manual ipv4.address 192.168.0.100/24  ipv4.gateway 192.168.0.1  ipv4.dns 114.114.114.114");
    //    qDebug() <<"system 1 :" << ret;
        ret = system("nmcli connection up enp1s0f0");
    //    qDebug() <<"system 2 :" << ret;
    }

    if(enp2_dev)
    {
        sprintf(cmd,str1_ipset,str_name[1],num[1],ui->lineEdit_ip2->text().toLatin1().data(),num[1]);
    //    qDebug() << "cmd1 = " << cmd;
        ret = system(cmd);
        //ret = system("nmcli connection modify --temporary 'enp1s0f1' connection.autoconnect yes ipv4.method manual ipv4.address 192.168.1.100/24  ipv4.gateway 192.168.1.1  ipv4.dns 114.114.114.114");
    //    qDebug() <<"system 3 :" << ret;
        ret = system("nmcli connection up enp1s0f1");
    //    qDebug() <<"system 4 :" << ret;
    }

    if(eth2_dev)
        sprintf(cmd,str1_ipset,str_name[2],num[2],ui->lineEdit_ip3->text().toLatin1().data(),num[2]);
    else if(eth0_dev)
        sprintf(cmd,str1_ipset,str_name[3],num[2],ui->lineEdit_ip3->text().toLatin1().data(),num[2]);
    qDebug() << "cmd2 = " << cmd;
    ret = system(cmd);
    //ret = system("nmcli connection modify --temporary 'eth2' connection.autoconnect yes ipv4.method manual ipv4.address 192.168.2.100/24  ipv4.gateway 192.168.2.1  ipv4.dns 114.114.114.114");
    qDebug() <<"system 5 :" << ret;
    if(ret)
    {
        if(eth2_dev)
            ret = system("nmcli con add type ethernet con-name eth2 ifname eth2 ip4 192.168.2.100/24 gw4 192.168.2.1");
        else if(eth0_dev)
            ret = system("nmcli con add type ethernet con-name eth0 ifname eth0 ip4 192.168.2.100/24 gw4 192.168.2.1");
    }
    else
    {
        if(eth2_dev)
            ret = system("nmcli connection up eth2");
        else if(eth0_dev)
        {
            ret = system("nmcli connection up eth0");
            qDebug() <<"system nmcli connection up eth0 :" << ret;
        }
    }
//    qDebug() <<"system 6 :" << ret;
#endif
#endif

    //getNetDeviceStats();


    g_sys_conf.ip1 = ui->lineEdit_ip1->text().toInt();
    g_sys_conf.ip2 = ui->lineEdit_ip2->text().toInt();
    g_sys_conf.ip3 = ui->lineEdit_ip3->text().toInt();


}

#if 0
void Widget::ifconfig_info_show(int ret)
{
    if(ret == 0)
    {
        ui->textBrowser_ifconfig->setText(myprocess_ifconfig->readAllStandardOutput());
        ui->textBrowser_ifconfig->setVisible(true);
        ui->textBrowser_ifconfig->setFocus();
        ui->textBrowser_ifconfig->raise();

    }
}
#endif


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
}

void Widget::on_radioButton_playmusic_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(false);
        ui->radioButton_micpanel->setEnabled(false);
    }
}

void Widget::on_radioButton_playrec_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(false);
        ui->radioButton_micpanel->setEnabled(false);
    }
}

void Widget::on_radioButton_rec_toggled(bool checked)
{
    if(checked)
    {
        ui->radioButton_michand->setEnabled(true);
        ui->radioButton_micpanel->setEnabled(true);
    }
}





void Widget::on_horizontalScrollBar_light_valueChanged(int value)
{
    ui->label_light_val->setText(QString::number(value));
#ifdef RK_3399_PLATFORM
    drvSetLcdBrt(value*2.55);   //0-255
#endif
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
//    if(myprocess_uart->state() == QProcess::Running)
//        myprocess_uart->kill();
////    char cmd[128] = "i2cset -f -y 4 0x10 0xa 0";
//    myprocess_iicspi->start("i2cset -f -y 4 0x10 0xa 0");
//    myprocess_iicspi->waitForFinished();
}

void Widget::on_radioButton_michand_clicked()
{
//    if(myprocess_uart->state() == QProcess::Running)
//        myprocess_uart->kill();
////    char cmd[128] = "i2cset -f -y 4 0x10 0xa 0x50";
//    myprocess_iicspi->start("i2cset -f -y 4 0x10 0xa 0x50");
//    myprocess_iicspi->waitForFinished();
}



void Widget::on_radioButton_SpeakVol_toggled(bool checked)
{
    if(checked)
    {
#ifdef RK_3399_PLATFORM
        drvDisableSpeaker();
#endif
    }
    else
    {
#ifdef RK_3399_PLATFORM
        drvEnableSpeaker();
#endif
    }
}

void Widget::on_radioButton_HandVol_toggled(bool checked)
{
    if(checked)
    {
#ifdef RK_3399_PLATFORM
        drvDisableHandout();
#endif
    }
    else
    {
#ifdef RK_3399_PLATFORM
        drvEnableHandout();
#endif
    }
}



void Widget::on_pushButton_start_cpustress_clicked()
{
    mysocket->sendMessage("pushButton_start_cpustress","1");   //把这个值发送过去


#if 0
    QString cmd;
    int cpu_n,mem_n;

    cpu_n = ui->comboBox_cpu->currentText().toInt();
    mem_n = ui->comboBox_memory->currentText().toInt();

    if(ui->checkBox_cpu_n->isChecked() && ui->checkBox_mem_n->isChecked())
    {
        if(cpu_n > 1)
            cmd = "stress-ng -c " + QString::number(cpu_n-1) + " --vm 1  --vm-bytes "+ QString::number(mem_n) + "% --vm-method all --verify -t 100d";
        else
            cmd = "stress-ng --vm 1  --vm-bytes "+ QString::number(mem_n) + "% --vm-method all --verify -t 100d";
    }
    else if(ui->checkBox_cpu_n->isChecked())
    {
        cmd =  "stress-ng -c " + QString::number(cpu_n) +" -t 100d";
    }
    else if(ui->checkBox_mem_n->isChecked())
    {
        cmd = "stress-ng --vm 1 --vm-bytes "+ QString::number(mem_n) +"% --vm-method all --verify -t 100d";
    }
    else
        return;
    //qDebug() << "cmd" << cmd;

    if(ui->pushButton_start_cpustress->text() == "开始压力测试")
    {
//        myprocess_cpu_stress->start(cmd);
        ui->pushButton_start_cpustress->setText("结束压力测试");
        ui->pushButton_start_cpustress->setStyleSheet("QPushButton{background-color:#ff0000;font: 20pt \"Ubuntu\";}");
    }
    else
    {
//        myprocess_cpu_stress->kill();
//        myprocess_cpu_stress->waitForFinished();
        ui->pushButton_start_cpustress->setText("开始压力测试");
        ui->pushButton_start_cpustress->setStyleSheet("QPushButton{background-color:#00ff00;font: 20pt \"Ubuntu\";}");
    }
#endif
}

//struct system_config
//{
//    int is_cpu_stress_start;   //启动开始cpu压力测试？0表示不开启，1开启测试
//    int is_gpio_flow_start;   //启动开启gpio流水灯吗？
//    int is_key_lights_start;  //启动开启键灯吗？
//    int default_show_page;    //启动默认显示页面,默认是第一页
//    int cpu_test_core_num;    //cpu的测试核心数（第8位表示是否勾上）
//    int mem_test_usage;       //内存测试的百分比（第8位表示是否勾上）
//}g_sys_conf;


//开机自动启动配置，记录到文件中
void Widget::on_checkBox_cpu_stress_toggled(bool checked)
{
//    g_sys_conf.is_cpu_stress_start = checked;
    mysocket->sendMessage("checkBox_cpu_stress",QString::number(checked));   //把这个值发送过去
//    SetConfigFile();
}


//开机自动启动配置，记录到文件中
void Widget::on_checkBox_gpio_flow_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_gpio_flow",QString::number(checked));   //把这个值发送过去
    //g_sys_conf.is_gpio_flow_start = checked;
//    SetConfigFile();
}


//开机自动启动配置，记录到文件中
void Widget::on_checkBox_keyLights_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_keyLights",QString::number(checked));   //把这个值发送过去
    //g_sys_conf.is_key_lights_start = checked;
//    SetConfigFile();
}



void Widget::on_comboBox_memory_currentIndexChanged(int index)
{
    mysocket->sendMessage("comboBox_memory",QString::number(index));   //把这个值发送过去
    //g_sys_conf.mem_test_usage = index;
//    SetConfigFile();
}

void Widget::on_comboBox_cpu_currentIndexChanged(int index)
{
    mysocket->sendMessage("comboBox_cpu",QString::number(index));   //把这个值发送过去
    //g_sys_conf.cpu_test_core_num = index;
//    SetConfigFile();
}






void Widget::on_checkBox_cpu_n_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_cpu_n",QString::number(checked));   //把这个值发送过去
    //g_sys_conf.is_cpu_test_checked = checked;
//    SetConfigFile();
}

void Widget::on_checkBox_mem_n_toggled(bool checked)
{
    mysocket->sendMessage("checkBox_mem_n",QString::number(checked));   //把这个值发送过去
    //g_sys_conf.is_mem_test_checked = checked;
//    SetConfigFile();
}

void Widget::on_comboBox_currentIndexChanged(int index)
{
    mysocket->sendMessage("comboBox",QString::number(index));   //把这个值发送过去
    //g_sys_conf.default_show_page = index;
//    SetConfigFile();
}



void Widget::show_boardtype_info(int keyboard_type)
{
//    int ret = -1;
    QStringList boardtype_list;
    boardtype_list << "嵌1" << "嵌2" << "嵌3" << "壁挂2" << "防风雨" << "多功能";
    ui->label_keyboard_type->setText("未知");
    if(had_keyboard)
    {
#ifdef RK_3399_PLATFORM
//        ret =  getKeyboardType_gztest();
 #endif
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



void Widget::page9_info_show(void)
{
#ifdef RK_3399_PLATFORM
    QFile file("/etc/os-version");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString line;
        QTextStream in(&file);  //用文件构造流

        line = in.readAll();

        ui->textBrowser_system_info->setText(line);

        file.close();
    }
    file.setFileName("/sys/mysysinfo/mysysinfo");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString line;
        QTextStream in(&file);  //用文件构造流

        line = in.readAll();

        ui->textBrowser_system_info->append(line);

        file.close();
    }

    on_pushButton_disk_info_clicked();
#endif
}




void Widget::on_pushButton_disk_info_clicked()
{

    mysocket->sendMessage("pushButton_disk_info","1");   //把这个值发送过去
//    if(myprocess_play1[0]->state()==QProcess::Running)
//    {
//        myprocess_play1[0]->kill();
//        myprocess_play1[0]->waitForFinished();
//    }
//    if(myprocess_play1[1]->state()==QProcess::Running)
//    {
//        myprocess_play1[1]->kill();
//        myprocess_play1[1]->waitForFinished();
//    }


//    myprocess_play1[0]->start("fdisk -l");
//    myprocess_play1[1]->start("grep GiB");

//    myprocess_play1[1]->waitForFinished();

//    ui->textBrowser_disk_info->setText(myprocess_play1[1]->readAllStandardOutput());

}



//void Widget::timer_uart_send_Function()
//{
//    QString buf = myprocess_iicspi->readAllStandardOutput();
//    qDebug() << "timer_uart_send_Function buf = " << buf;
//    ui->textBrowser_IICSPI->append(buf);
////        ui->textBrowser_IICSPI->setVisible(true);

//}



//void Widget::sereial_info_show()
//{
//    QString buf = serial->readAll();
//    if (!buf.isEmpty())
//    {
//        ui->textBrowser_IICSPI->append(buf);
//    }
//}


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


#if 0
//查询lcd屏幕单片机版本
void Widget::on_pushButton_lcd_mcu_info_clicked()
{
    QString read_data;
    QString content;
    int num;
    ui->pushButton_lcd_mcu_info->setStyleSheet("QPushButton{background-color:#ff0000;font: 20pt \"Ubuntu\";}");
//    if(myprocess_version->state() == QProcess::Running)
//    {
//        myprocess_version->kill();
//        myprocess_version->waitForFinished();
//    }

//    myprocess_version->start("/home/deepin/mcu_update/read_mcu_version_dg_lcd");
//    myprocess_version->waitForFinished();
//    read_data = myprocess_version->readAllStandardOutput();

    QStringList tempStringList =  read_data.split("\n");

    num = tempStringList.count();
    //qDebug() << "num = " << tempStringList.count();

//    for(i=0;i<num;i++)
//        qDebug() << tempStringList.at(i);
    if(num >= 4)
    {
        QStringList templist;
        content = tempStringList.at(1);
        templist = content.split(":");
        if(templist.count()>= 2)
            ui->label_lcd_mcu_md5->setText(templist.at(1));
        content = tempStringList.at(2);
        num = content.indexOf(":");
        ui->label_lcd_mcu_time->setText(content.mid(num+1));

        content = tempStringList.at(3);
        templist = content.split(":");
        if(templist.count()>= 2)
            ui->label_lcd_mcu_version->setText(templist.at(1));
    }
    ui->pushButton_lcd_mcu_info->setStyleSheet("QPushButton{background-color:#00ff00;font: 20pt \"Ubuntu\";}");
}

//查询键盘单片机版本
void Widget::on_pushButton_key_mcu_info_clicked()
{
    QString read_data;
    QString content;
    int num;
    ui->pushButton_key_mcu_info->setStyleSheet("QPushButton{background-color:#ff0000;font: 20pt \"Ubuntu\";}");
//    if(myprocess_version->state() == QProcess::Running)
//    {
//        myprocess_version->kill();
//        myprocess_version->waitForFinished();
//    }


//    myprocess_version->start("/home/deepin/mcu_update/read_mcu_version_dg_keyboard");
//    myprocess_version->waitForFinished();
//    read_data = myprocess_version->readAllStandardOutput();

   // qDebug() << read_data;

    QStringList tempStringList =  read_data.split("\n");

    num = tempStringList.count();
    //qDebug() << "num = " << tempStringList.count();

//    for(i=0;i<num;i++)
//        qDebug() << tempStringList.at(i);
    if(num >= 4)
    {
        QStringList templist;
        content = tempStringList.at(1);
        templist = content.split(":");
        if(templist.count()>= 2)
            ui->label_keyb_mcu_md5->setText(templist.at(1));
        content = tempStringList.at(2);
        num = content.indexOf(":");
        ui->label_keyb_mcu_time->setText(content.mid(num+1));

        content = tempStringList.at(3);
        templist = content.split(":");
        if(templist.count()>= 2)
            ui->label_keyb_mcu_version->setText(templist.at(1));
    }
    ui->pushButton_key_mcu_info->setStyleSheet("QPushButton{background-color:#00ff00;font: 20pt \"Ubuntu\";}");
}


//查询libdrv722.so版本
void Widget::on_pushButton_drv_so_info_clicked()
{
    QString read_data;
    char buildtime[32] = {0};
    int version = 0;
    ui->pushButton_drv_so_info->setStyleSheet("QPushButton{background-color:#ff0000;font: 20pt \"Ubuntu\";}");
//    if(myprocess_version->state() == QProcess::Running)
//    {
//        myprocess_version->kill();
//        myprocess_version->waitForFinished();
//    }

//    myprocess_version->start("md5sum /usr/lib/libdrv722.so");
//    myprocess_version->waitForFinished();
//    read_data = myprocess_version->readAllStandardOutput();
    ui->label_drv_so_md5->setText(read_data.left(32));

#ifdef RK_3399_PLATFORM
    drvGetBuildtimeVersion(buildtime,&version);
#endif
    ui->label_drv_so_time->setText(buildtime);
    ui->label_drv_so_version->setText(QString::number(version));
    ui->pushButton_drv_so_info->setStyleSheet("QPushButton{background-color:#00ff00;font: 20pt \"Ubuntu\";}");
}


//查询jc_keyboard.ko版本
void Widget::on_pushButton_jc_ko_info_clicked()
{
    QString read_data;
    QString content;
    int i,num,n;

    ui->pushButton_jc_ko_info->setStyleSheet("QPushButton{background-color:#ff0000;font: 20pt \"Ubuntu\";}");
//    if(myprocess_version->state() == QProcess::Running)
//    {
//        myprocess_version->kill();
//        myprocess_version->waitForFinished();
//    }

//    myprocess_version->start("md5sum /root/jc_keyboard.ko");
//    myprocess_version->waitForFinished();
//    read_data = myprocess_version->readAllStandardOutput();
    ui->label_jc_ko_md5->setText(read_data.left(32));

//    myprocess_version->start("modinfo /root/jc_keyboard.ko");
//    myprocess_version->waitForFinished();
//    read_data = myprocess_version->readAllStandardOutput();
    QStringList tempStringList =  read_data.split("\n");

    num = tempStringList.count();
    for(i=0;i<num;i++)
    {
        content = tempStringList.at(i);
        if(content.startsWith("version"))
        {
            n = content.indexOf(":");
            if(n>0)
                ui->label_jc_ko_version->setText(content.mid(n+5));
        }
        else if(content.startsWith("description"))
        {
            n = content.indexOf(":");
            n = content.indexOf(":",n+1);
            if(n>0)
                ui->label_jc_ko_time->setText(content.mid(n+1));
         }
    }
    ui->pushButton_jc_ko_info->setStyleSheet("QPushButton{background-color:#00ff00;font: 20pt \"Ubuntu\";}");
}
#endif


void Widget::on_pushButton_Last_page_clicked()
{
//    last_func_page_show();
    mysocket->sendMessage("pushButton_Last_page","1");
}

void Widget::on_pushButton_Next_page_clicked()
{
//    next_func_page_show();
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




//void Widget::on_pushButton_12_clicked()
//{
//    if(ui->pushButton_12->isEnabled())
//    {
////        myftp->get("/123.txt", "./123.txt");

////        qDebug() <
///
///
/// < "myftp->get(123.txt, ./123.txt)";
////        if(!myftp->login("ftp_hnhtjc","123456"))
////            qDebug() << "login success";

////        myftp->list("./");
////        myftp->get("123.txt","123.txt");
//    }
//    else
//    {

//        ui->pushButton_12->setEnabled(true);
//    }
//}


//闪烁间隔时间调整
void Widget::on_lineEdit_interval_editingFinished()
{
    mysocket->sendMessage("lineEdit_interval",ui->lineEdit_interval->text());
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


