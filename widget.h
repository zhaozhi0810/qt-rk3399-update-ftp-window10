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

    void Palette_button(int ison,int val);

private slots:
    void on_pushButton_start_color_test_clicked();
    void on_pushButton_clicked();

    void on_pushButton_Play_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();


    void on_checkBox_toggled(bool checked);

    void on_pushButton_3_clicked();

    void on_pushButton_ifconfig_clicked();

    void on_radioButton_loop_toggled(bool checked);

    void on_radioButton_playmusic_toggled(bool checked);

    void on_radioButton_playrec_toggled(bool checked);

    void on_radioButton_rec_toggled(bool checked);

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

    void displayMessage(QByteArray str);

    void conneted_to_server(void);    //连接上服务器之后需要一些初始化

    void on_verticalScrollBar_lightpwm2_sliderMoved(int position);

    void on_horizontalScrollBar_light_sliderMoved(int position);

    void on_pushButton_lcd_last_color_clicked();

    void on_pushButton_lcd_next_color_clicked();

    void on_pushButton_version_compare_clicked();

    void on_pushButton_update_clicked();

    void on_radioButton_Spitest_clicked(bool checked);

    void on_radioButton_IICtest_clicked(bool checked);

    void on_lineEdit_ip1_textEdited(const QString &arg1);

    void on_lineEdit_ip2_textEdited(const QString &arg1);

    void on_lineEdit_ip3_textEdited(const QString &arg1);

    void on_checkBox_bigpack1_clicked(bool checked);

    void on_checkBox_adap1_clicked(bool checked);

    void on_checkBox_bigpack2_clicked(bool checked);

    void on_checkBox_adap2_clicked(bool checked);

    void on_checkBox_bigpack3_clicked(bool checked);

    void on_checkBox_adap3_clicked(bool checked);

    void on_radioButton_micpanel_clicked(bool checked);

    void on_radioButton_michand_clicked(bool checked);

    void on_horizontalScrollBar_SpeakVol_sliderMoved(int position);

    void on_radioButton_SpeakVol_clicked(bool checked);

    void on_lineEdit_interval_textEdited(const QString &arg1);

private:
    Ui::Widget *ui;
    MytcpSocketClient *mysocket;
//    int check_version_wait;
    int update_command_wait;
//    QString version_store_string;    //用于保存版本信息的数据
    FtpServer *myftp_server;

    int page2_show_color;
    void next_color_page_show();
    void last_color_page_show();
    void next_func_page_show();
    void last_func_page_show();
    void stackedWidget_page_show(int index);

    void ping_info_show(QString &strMsg,int ping_num);

    void get_net_device_through_dir(void);

    void get_softwareversion_info();
    void softwareversion_version_compare(QString message);
    void software_packet();


    bool ping_status[3];   //1：开始ping，0表示没有开始
    unsigned int error_count[3];  //ping出现错误的计数值
    unsigned int icmp_saved[3];    //通过icmp的值判断是否ping异常
    unsigned int icmp_cur[3];   //通过icmp的值判断是否ping异常
    int is_test_press;    //组合键？
//    int key_light_connect;   //键灯控制
//    int lightpwm;    //键灯亮度
//    int lcdPwm;
//    int iicspi_connect;
//    int had_keyboard;    //组合键？
    int eth0_dev;
    int eth2_dev;

    QFrame* buttonFrame;

    QIntValidator* intValidator;
    QRegExpValidator *pReg;

    void show_boardtype_info(int);


};

#endif // WIDGET_H
