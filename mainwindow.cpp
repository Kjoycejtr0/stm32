#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QPushButton"
#include "QFileDialog"
#include "QTextStream"
#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "QInputDialog"
#include <QCloseEvent>

QTemporaryFile Flash_Temp_file;
QTemporaryFile FLM_Temp_file;
QTemporaryFile Program_Temp_file;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    setWindowTitle("脱机烧录器");
    setFixedSize(580,420);

    ui->Designer->setOpenExternalLinks(true);
    ui->Designer->setText("<a href=https://me.csdn.net/u012918754>Designer : airtech");

    ui->FLM_TEXT->setReadOnly(true);
    ui->Flash_Name->setReadOnly(true);

    /************Flash 临时文件***************/
//    QTemporaryFile Flash_Temp_file;
    Flash_Temp_file.setAutoRemove(true);
    Flash_Temp_file.open();
    Flash_Tmp_name=Flash_Temp_file.fileName();
//    qDebug()<<tr("Flash : ")+Flash_Tmp_name;

    /************FLM 临时文件**************/
//    QTemporaryFile FLM_Temp_file;
    FLM_Temp_file.setAutoRemove(true);
    FLM_Temp_file.open();
    FLM_Tmp_name=FLM_Temp_file.fileName();
//    qDebug()<<tr("FLM : ")+FLM_Tmp_name;

    /************Program 临时文件**************/
//    QTemporaryFile Program_Temp_file;
    Program_Temp_file.setAutoRemove(true);
    Program_Temp_file.open();
    Program_Tmp_name=Program_Temp_file.fileName();
//    qDebug()<<tr("Program : ")+Program_Tmp_name;

    /**********Flash Blob 文件目录***********/
    QString Offine_Head="Offline.";
    Blob_File_Name=FLM_Tmp_name.mid(0,FLM_Tmp_name.indexOf(Offine_Head))+Blob_Name;
//    qDebug()<<Blob_File_Name;

    UART_Init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

static uint16_t const wCRC16Table[256] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};


uint16_t MainWindow::CRC16(const uint8_t* pDataIn, uint16_t	iLenIn)
{
    uint16_t wResult = 0;
    uint16_t wTableNo = 0;
    int i = 0;
    for( i = 0; i < iLenIn; i++)
    {
     wTableNo = ((wResult & 0xff) ^ (pDataIn[i] & 0xff));
        wResult = ((wResult >> 8) & 0xff) ^ wCRC16Table[wTableNo];
    }
    return wResult;
}


void MainWindow::on_Open_COM_clicked()
{
    if(UART_Open_OK==0) Config_UART_Open();
    if((ui->Open_COM->text() ==  "打开串口") &&  UART_Open_OK==1)
    {
        ui->Open_COM->setText("关闭串口");
    }
    else if(ui->Open_COM->text() ==  "关闭串口")
    {
        serial->clear();
        serial->close();
        ui->Open_COM->setText("打开串口");
        UART_Open_OK=0;
    }
}

void MainWindow::on_Open_FLM_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
    tr("打开FLM文件"),
    "",
    tr("FLM文件 (*.flm)"),
    0);
    if (!fileName.isNull())
    {
        ui->FLM_TEXT->setText(fileName);
        Open_FLM_File_OK=1;
        QFile   FLM(fileName);
        Open_FLM_File(FLM);
    }
    else
    {
        Open_FLM_File_OK=0;
    }
}

void MainWindow::on_Open_Flash_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
    tr("打开Flash文件"),
    "",
    tr("Flash文件 (*.bin)"),
    0);
    if (!fileName.isNull())
    {
        ui->Flash_Name->setText(fileName);
        Open_Flash_File_OK=1;
        QFile   Flash(fileName);
        Open_Flash_File(Flash);
    }
    else
    {
        Open_Flash_File_OK=0;
    }
}

void MainWindow::on_Write_Offine_clicked()
{
    char* buff;
    QByteArray note=ui->File_Name->text().toLatin1();
    buff=note.data();

    memcpy(FLM_Buff,buff,ui->File_Name->text().length());
    Gen_Program_File();

    Connect_Target();
}

void MainWindow::on_Output_Conf_clicked()
{
    char* buff;
    QByteArray note=ui->File_Name->text().toLatin1();
    buff=note.data();
    memcpy(FLM_Buff,buff,ui->File_Name->text().length());

    QString fileName = QFileDialog::getSaveFileName(this,
    "导出配置文件",
    ui->File_Name->text(),
    tr("Config Files (*.Config)"));

    if (!fileName.isNull())
    {
        Gen_Program_File();
        QFile   Out_Config(fileName);
        Output_Config_File(Out_Config);
    }

}

void MainWindow::on_Input_Conf_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开配置文件"),
                                                    "",
                                                    tr("Config Files (*.Config)"),
                                                    0);
    if (!fileName.isNull())
    {
        QFile   Input_Config(fileName);
        Input_Config_File(Input_Config);
        Input_Flag=1;
    }
    else
    {
        Input_Flag=0;
    }
}

void MainWindow::on_Refresh_COM_clicked()
{
    Refresh_COM_Port();
}


void MainWindow::closeEvent(QCloseEvent *event) //系统自带退出确定程序
{
    int choose;
    choose= QMessageBox::question(this, tr("退出程序"),
                                   QString(tr("确认退出程序?")),
                                   QMessageBox::Yes | QMessageBox::No);

    if (choose== QMessageBox::No)
     {
          event->ignore();  //忽略//程序继续运行
    }
    else if (choose== QMessageBox::Yes)
    {
        Flash_Temp_file.remove();
        FLM_Temp_file.remove();
        Program_Temp_file.remove();
        event->accept();  //介绍//程序退出
    }
}


void MainWindow::on_About_triggered()
{
    QMessageBox About;
    About.information(this,"关于程序","脱机烧录器 Release 1.0\r\nBuild 1100\r\n"
                                  "Powered by DAPLink\r\n"
                      "https://github.com/ARMmbed/DAPLink\r\n"
                      "\r\n"
                      "Designer : airtech\r\n"
                      "https://me.csdn.net/u012918754\r\n","好哒");
    About.show();
}

void MainWindow::on_Help_triggered()
{
    QMessageBox help;

    help.information(this,"帮助","1. 将设备通过USB接入计算机\r\n"
                                   "2. 打开本软件，并点击 “刷新串口” 按钮刷新COM口号\r\n"
                                   "3. 选择相应的COM口，并点击 “打开串口”\r\n"
                                   "4. 点击“选择芯片FLM文件” 选择相应单片机的FLM文件\r\n"
                                   "5. 点击 “载入Flash文件” 导入Flash（bin）文件并加密\r\n"
                                   "6. 使用 “导出配置文件” 选项可以将配置好的数据保存到计算机\r\n"
                                   "7. 使用 “导入配置文件” 选项可以读取和使用已保存的数据\r\n"
                                   "8. 使用 “写入烧录器” 将数据传输到脱机烧录器\r\n"
                                   "9. “镜像注释”写入烧录器的镜像名称最大16字符","好哒");
    help.show();
}
