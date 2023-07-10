#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMainWindow>

#include<QIntValidator>
#include <QtNetwork/QNetworkInterface>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFrame>
#include "mytcpsocketclient.h"
#include "ftpServer/ftpserver.h"
#include "debuglogdialog/debuglogdialog.h"


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void Palette_button(int ison,int val);

private slots:
    void on_pushButton_start_color_test_clicked();
#ifdef RK_3399_PLATFORM
    void timer_key_leds_slot_Function();
#endif

    void ping1_info_show();
//    void topcmd_info_show();
    void ping2_info_show();
    void ping3_info_show();
    void ping1_finished_slot(int ret);
    void ping2_finished_slot(int ret);
    void ping3_finished_slot(int ret);
    void play_finished_slot(int ret);
//    void top_cmd_finished_slot(int ret);
    void on_pushButton_clicked();

    void on_pushButton_start_lcd_touch_clicked();

    void on_pushButton_Play_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

//    void on_verticalSlider_lightpwm2_valueChanged(int value);

    void on_checkBox_toggled(bool checked);

    void on_pushButton_3_clicked();

    void on_pushButton_ifconfig_clicked();

    void on_radioButton_loop_toggled(bool checked);

    void on_radioButton_playmusic_toggled(bool checked);

    void on_radioButton_playrec_toggled(bool checked);

    void on_radioButton_rec_toggled(bool checked);

    void on_horizontalScrollBar_SpeakVol_valueChanged(int value);

    void on_horizontalScrollBar_HandVol_valueChanged(int value);

    void on_horizontalScrollBar_EarphVol_valueChanged(int value);

    void on_verticalScrollBar_lightpwm2_valueChanged(int value);

    void on_pushButton_FlowLEDS_clicked();

    void on_horizontalScrollBar_light_valueChanged(int value);

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_radioButton_micpanel_clicked();

    void on_radioButton_michand_clicked();


    void on_radioButton_SpeakVol_toggled(bool checked);

    void on_radioButton_HandVol_toggled(bool checked);

    void on_pushButton_start_cpustress_clicked();

    void on_checkBox_cpu_stress_toggled(bool checked);

    void on_checkBox_gpio_flow_toggled(bool checked);

    void on_checkBox_keyLights_toggled(bool checked);

    void on_comboBox_memory_currentIndexChanged(int index);

    void on_comboBox_cpu_currentIndexChanged(int index);

    void on_checkBox_cpu_n_toggled(bool checked);

    void on_checkBox_mem_n_toggled(bool checked);

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_disk_info_clicked();

    void on_radioButton_Uarttest_toggled(bool checked);

    void on_pushButton_clear_display_clicked();


    void on_pushButton_Last_page_clicked();
    void on_pushButton_Next_page_clicked();

    void on_pushButton_Help_clicked();

//    void on_pushButton_12_clicked();

    void displayMessage(QByteArray str);

    void conneted_to_server(void);    //连接上服务器之后需要一些初始化


    void on_lineEdit_interval_editingFinished();


    void on_verticalScrollBar_lightpwm2_sliderMoved(int position);

    void on_horizontalScrollBar_light_sliderMoved(int position);

//    void on_pushButton_13_clicked();

    void on_pushButton_lcd_last_color_clicked();

    void on_pushButton_lcd_next_color_clicked();



    void on_pushButton_version_compare_clicked();


//    void on_pushButton_software_packet_clicked();
    void on_pushButton_update_clicked();

    void on_radioButton_Spitest_clicked(bool checked);

    void on_radioButton_IICtest_clicked(bool checked);

private:
    Ui::Widget *ui;
    MytcpSocketClient *mysocket;
    int check_version_wait;
    int update_command_wait;
    QString version_store_string;    //用于保存版本信息的数据
    FtpServer *myftp_server;

#ifdef RK_3399_PLATFORM
    QTimer * timer_key_leds;
    int is_light_all_leds;   //点亮所有灯-->1？还是循环点亮-->2,0表示不需要控制
    void light_off_all_leds(void);
    void circle_light_off_leds(void);

#endif
//    typedef void (Widget::*ping_finished_slot_func_t)(int);
//    typedef void (Widget::*ping1_info_show_func_t)();


    QTimer * timer_net_stat;
    QTimer * timer_cpu_mem_info;
//    QTimer * timer_uart_send;
//    QSerialPort *serial;
    int page2_show_color;
    void next_color_page_show();
    void last_color_page_show();
    void next_func_page_show();
    void last_func_page_show();
    void stackedWidget_page_show(int index);
//    void terminate_ping1();
//    void terminate_ping2();
//    void terminate_ping3();
    void ping_info_show(QString &strMsg,int ping_num);
    void page9_info_show(void);
    void get_net_device_through_dir(void);

    void get_softwareversion_info();
    void softwareversion_version_compare(QString message);
    void software_packet();

//    void ping_pushButton_function(int ping_num);
    bool ping_status[3];   //1：开始ping，0表示没有开始
    unsigned int error_count[3];  //ping出现错误的计数值
    unsigned int icmp_saved[3];    //通过icmp的值判断是否ping异常
    unsigned int icmp_cur[3];   //通过icmp的值判断是否ping异常
    int is_test_press;    //组合键？
    int key_light_connect;   //键灯控制
    int lightpwm;    //键灯亮度
    int lcdPwm;
    int iicspi_connect;
    int had_keyboard;    //组合键？
    int eth0_dev;
    int eth2_dev;
    int enp1_dev;
    int enp2_dev;

//    int keyboardtype;

    QMainWindow* lcd_touch_ui;
//    QProcess *myprocess_ifconfig;
//    QProcess *myprocess_iicspi;
//    QProcess *myprocess_uart;
//    QProcess *myprocess_FlowLEds;
//    QProcess *myprocess_ping[3];
//    QProcess *myprocess_play;   //播放进程
//    QProcess *myprocess_version;   //
    QFrame* buttonFrame;
//    QProcess *myprocess_top_cmd;   //播放进程
//    QProcess *myprocess_cpu_stress;   //播放进程
#if 1
//    QProcess *myprocess_play1[2];   //播放进程
#endif
    QIntValidator* intValidator;
    QRegExpValidator *pReg;
    void getNetDeviceStats();
    void show_boardtype_info(int);

//    FtpManager* myftp;

};

#endif // WIDGET_H
