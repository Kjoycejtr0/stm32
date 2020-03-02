#include    "mainwindow.h"
#include    "ui_mainwindow.h"
#include    "uart_config.h"
#include    <QtSerialPort/QtSerialPort>
#include    <QtSerialPort/QSerialPortInfo>
#include    "QMessageBox"


/************************************************************************
AT+WS DATA CRC16 0x0D 0x0A //Start Write Flash
AT+WE DATA CRC16 0x0D 0x0A //End Write Flash
 ***********************************************************************/

#define End     0x45
#define Start   0x53

void MainWindow::UART_Init()
{
    Refresh_COM_Port();
}

//刷新串口号
void MainWindow::Refresh_COM_Port()
{
    ui->COM->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        serial->setPort(info);
        if(serial->open(QIODevice::ReadWrite))
        {
            ui->COM->addItem(serial->portName());
            serial->close();
        }
    }
}

//配置串口和打开串口
void MainWindow::Config_UART_Open()
{
    //设置串口名
    serial->setPortName(ui->COM->currentText());
    //打开串口
    if(serial->open(QIODevice::ReadWrite))
    {
        //设置波特率
        serial->setBaudRate(115200);
        //设置数据位数
        serial->setDataBits(QSerialPort::Data8);
        //设置奇偶校验
        serial->setParity(QSerialPort::NoParity);
        //设置停止位
        serial->setStopBits(QSerialPort::OneStop);
        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->close();
        serial->open(QIODevice::ReadWrite);
        QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_UART);
        UART_Open_OK=1;
    }
    else
    {
        QMessageBox ::information(NULL, NULL, "打开串口失败","知道哒^^");
        serial->clear();
        serial->close();
        UART_Open_OK=0;
    }
}

//读取UART数据
void MainWindow::Read_UART()
{
    QByteArray  UART_Buff;

    UART_Buff   =   serial->readAll();
    Data_Processing(UART_Buff);
}

//处理接收到的数据

void MainWindow::Data_Processing(QByteArray Data)
{
    uint16_t cnt;
    uint8_t Buff[7]={0};
    char UART_Transmit[256];
    static QFile Serial_Buff(Program_File_Name);

    if(Open_Tx_Flag==1)
    {
        Serial_Buff.open(QIODevice::ReadOnly);
        Open_Tx_Flag=0;
    }
    static uchar *Serial_Buff_map=Serial_Buff.map(0,Serial_Buff.size());
    static uint32_t Serial_Buff_Length=Serial_Buff.size();
    static uint32_t Transmit_cnt;

    if(Length_Flag==1)
    {
        if((Serial_Buff_Length%256)==0) Transmit_cnt=Serial_Buff_Length/256;
        else Transmit_cnt=(Serial_Buff_Length/256)+1;

        ui->progressBar->setMaximum(Transmit_cnt);
        Length_Flag=0;
    }

    for(cnt=0;cnt<5;cnt++)
    {
        Buff[cnt]=Data[cnt];
    }
    if( (UART_Write_cnt<Transmit_cnt)    &&  (memcmp(Buff,WS,5)==0))
    {
        memset(UART_Transmit,0xFF,256);
        memcpy(UART_Transmit,Serial_Buff_map+UART_Write_cnt*256,256);
        Send_Data(UART_Transmit,Start);
        UART_Write_cnt+=1;
    }
    else if(UART_Write_cnt==Transmit_cnt)
    {
        memset(UART_Transmit,0xFF,256);
        memcpy(UART_Transmit,Serial_Buff_map+UART_Write_cnt*256,256);
        Send_Data(UART_Transmit,End);
        UART_Write_cnt=0;
    }
    else if(memcmp(Buff,RS,5)==0)
    {
        qDebug()<<"CMD RS";
        Send_Data(UART_Transmit,Start);
    }
    else if(memcmp(Buff,WE,5)==0)
    {
        Serial_Buff.close();
        Open_Tx_Flag=1;
        Length_Flag=1;
        UART_Write_cnt=0;
    }
    else if(memcmp(Buff,EE,5)==0)
    {
        Serial_Buff.close();
        Open_Tx_Flag=1;
        Length_Flag=1;
        UART_Write_cnt=0;
        ui->progressBar->setValue(0);
        QMessageBox::warning(NULL, "警告", "传输错误", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }

    qDebug()<<UART_Write_cnt;
    ui->progressBar->setValue(UART_Write_cnt);
}


void MainWindow::Send_Data(char *Data,uint8_t Status)
{
    uint16_t cnt;
    char TX_Buffer[265];
    uint16_t CRC16_t;

    memset(TX_Buffer,0x00,265);
    memcpy(TX_Buffer,WS,5);
    TX_Buffer[4]=Status;
    TX_Buffer[263]=0x0D;
    TX_Buffer[264]=0x0A;

    for(cnt=0;cnt<256;cnt++)
    {
        TX_Buffer[cnt+5]=*(Data+cnt);
    }

    CRC16_t=CRC16((uint8_t *)TX_Buffer,261);
    TX_Buffer[261]=(CRC16_t>>8)   &   0xFF;
    TX_Buffer[262]=CRC16_t  &   0xFF;

    serial->write(TX_Buffer,265);
}

/*********************握手*********************/
void MainWindow::Connect_Target(void)
{
    char CC[265];

    UART_Write_cnt=0;
    memset(CC,0x00,265);
    memcpy(CC,WS,5);
    CC[3]=0x43;
    CC[4]=0x43;


    CC[263]=0x0D;
    CC[264]=0x0A;


    serial->write(CC,265);
}
