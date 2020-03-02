#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "QInputDialog"
#include "QFileDialog"
#include "QMessageBox"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void sha256(const unsigned char *data, size_t len, unsigned char *out);
    void EncryptTEA(unsigned int *firstChunk, unsigned int *secondChunk, unsigned int* key);
    void DecryptTEA(unsigned int *firstChunk, unsigned int *secondChunk, unsigned int* key);
    void EncryptBuffer(char* buffer, int size, unsigned int* key);
    void DecryptBuffer(char* buffer, int size, unsigned int* key);
    void UART_Init();
    void Refresh_COM_Port();
    void Config_UART_Open();
    void Read_UART();
    void Data_Processing(QByteArray Data);
    uint16_t CRC16(const uint8_t* pDataIn, uint16_t	iLenIn);
    void Send_Data(char *Data,uint8_t Status);
    void Open_Flash_File(QFile  &Open_Flash);
    void Open_FLM_File(QFile  &Open_FLM);
    void FLM_Decode(void);
    void Gen_Program_File(void);
    void Output_Config_File(QFile   &Output_File);
    void Input_Config_File(QFile    &Input_Config);
    void Connect_Target(void);

private slots:

    void on_Open_COM_clicked();

    void on_Open_FLM_clicked();

    void on_Open_Flash_clicked();

    void on_Write_Offine_clicked();

    void on_Output_Conf_clicked();

    void on_Input_Conf_clicked();

    void on_Refresh_COM_clicked();

    void on_About_triggered();

    void on_Help_triggered();

private:
    Ui::MainWindow *ui;

    const uint8_t WE[5]={0x41, 0x54, 0x2B, 0x57, 0x45 };
    const uint8_t WS[5]={0x41, 0x54, 0x2B, 0x57, 0x53 };
    const uint8_t RS[5]={0x41, 0x54, 0x2B, 0x52, 0x53 };
    const uint8_t EE[5]={0x41, 0x54, 0x2B, 0x45, 0x45 };
    unsigned int *key = (unsigned int *)"This_is_Key_1057";
    QSerialPort *serial =   new QSerialPort;
    QMessageBox Flash_Message;
    bool        UART_Open_OK=0;
    bool        Open_Flash_File_OK=0;
    bool        Open_FLM_File_OK=0;

    uint8_t     FLM_Buff[1024];
    uint32_t    Flash_length;
    uint32_t    FLM_length;

    uint32_t    Flash_Algo_Hex[512];

    uint32_t    Wirte_Len=0;
    bool        Input_Flag=0;
    uint32_t    Send_Len=0;

    QString     Flash_Tmp_name;
    QString     FLM_Tmp_name;
    QString     Program_Tmp_name;

    QString     code_start="code_start:";
    QString     code_end="code_end:";

    QString     algo_start="algo_start:";
    QString     algo_end="algo_end:";

    QString     Blob_File_Name;
    QString     Blob_Name="c_blob.c";

    QString     Program_File_Name;
    QString     Program_Name="AT.bin";

    bool        Open_Tx_Flag=1;
    bool        Length_Flag=1;

    bool        FLM_Read_Flag=0;

    uint32_t    Main_Flash_Length=0;

    uint32_t    UART_Write_cnt=0;

    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
