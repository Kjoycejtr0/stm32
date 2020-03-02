#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QTextStream"
#include "QInputDialog"
#include "QProcess"
#include <QCryptographicHash>

void MainWindow::Output_Config_File(QFile   &Output_File)
{
    if(Output_File.open(QIODevice::WriteOnly))
    {
        QFile Copy_Out(Program_File_Name);
        Copy_Out.open(QIODevice::ReadOnly);
        uchar *Copy_map=Copy_Out.map(0,Copy_Out.size());
        Wirte_Len=Copy_Out.size();

        QDataStream Output(&Output_File);
        Output.writeRawData((char *)Copy_map,Wirte_Len);

        Copy_Out.unmap(Copy_map);
        Copy_Out.close();
        Output_File.close();
        Flash_Message.information(NULL,"数据导出","导出成功~~！","好哒");
    }
    else Flash_Message.warning(NULL,"数据导出","导出失败 T_T","知道啦~~");
}


void MainWindow::Input_Config_File(QFile    &Input_Config)
{

    if(Input_Config.open(QIODevice::ReadOnly))
    {
        /*********************建立读取文件内存映射************************/
        uchar *Input_map=Input_Config.map(0,Input_Config.size());
        uint32_t Input_length=Input_Config.size();

        QFile Pro_Input(Program_Tmp_name);
        QFile Main_Input(Flash_Tmp_name);

        /********************读取FLM字段到临时文件********************/
        Pro_Input.open(QIODevice::ReadWrite);
        QDataStream Input_FLM(&Pro_Input);
        Input_FLM.writeRawData((char *)Input_map,1024);
        Pro_Input.close();
        FLM_Read_Flag=1;
        ui->FLM_TEXT->setText(tr("已读取FLM数据"));

        /********************读取Flash字段到临时文件*******************/
        Main_Input.open(QIODevice::ReadWrite);
        QDataStream Input_Flash(&Main_Input);
        Input_Flash.writeRawData((char *)Input_map+1024,Input_length-1024);
        Main_Input.close();
        ui->Flash_Name->setText(tr("已读取加密Flash数据"));

        /*******************读取镜像注释**************************/
        char Title[16];

        Main_Flash_Length=Input_length-1024;
        qDebug()<<Main_Flash_Length;
        memcpy(Title,Input_map,16);
        ui->File_Name->setText(QString(Title));
        /********************关闭读取文件*************************/
        Input_Config.unmap(Input_map);
        Input_Config.close();
    }
}


void MainWindow::Open_Flash_File(QFile  &Open_Flash)
{
    uint32_t TEA_cnt=0;
    char TEA_Buff[1024];
    uint32_t Write_cnt=0;
    if(Open_Flash.open(QIODevice::ReadOnly))
    {
        uchar *Flash_map=Open_Flash.map(0,Open_Flash.size());
        Flash_length=Open_Flash.size();
        qDebug()<<Flash_length;

        if(Flash_length%1024==0) Write_cnt=Flash_length/1024;
        else Write_cnt=(Flash_length/1024)+1;
        qDebug()<<Write_cnt;

        /***********读数据到临时文件*************/
        QFile Flash_Tmp_Config(Flash_Tmp_name);
        Flash_Tmp_Config.open(QIODevice::ReadWrite);
        QDataStream Flash_Tmp(&Flash_Tmp_Config);

        /**********加密FLASH文件************/
        for(TEA_cnt=0;TEA_cnt<Write_cnt;TEA_cnt++)
        {
            Flash_Tmp_Config.seek(1024*TEA_cnt);
            memcpy(TEA_Buff,Flash_map+TEA_cnt*1024,1024);
            EncryptBuffer(TEA_Buff,1024,key);
            Flash_Tmp.writeRawData(TEA_Buff,1024);
        }

        Main_Flash_Length=Flash_length;
        FLM_Read_Flag=0;

        Open_Flash.unmap(Flash_map);

        Flash_Tmp_Config.close();

        QByteArray Hash = QCryptographicHash::hash(Open_Flash.readAll(), QCryptographicHash::Md5);

        Open_Flash.close();


        ui->Hash->setText(tr("Flash文件MD5: ")+Hash.toHex().constData());
    }

    Input_Flag=0;
    Flash_Message.close();
    Flash_Message.information(NULL,"加载Flash数据","载入成功成功~~！","好哒");
}

void MainWindow::Open_FLM_File(QFile  &Open_FLM)
{
    if(Open_FLM.open(QIODevice::ReadOnly))
    {
        uchar *FLM_map=Open_FLM.map(0,Open_FLM.size());
        FLM_length=Open_FLM.size();
        qDebug()<<FLM_length;

        /***********读数据到临时文件*************/
        QFile FLM_Tmp_Config(FLM_Tmp_name);
        FLM_Tmp_Config.open(QIODevice::ReadWrite);
        QDataStream FLM_Tmp(&FLM_Tmp_Config);
        FLM_Tmp.writeRawData((char *)FLM_map,FLM_length);
        FLM_Tmp_Config.close();

        Open_FLM.unmap(FLM_map);
        Open_FLM.close();

        FLM_Decode();
    }


    Input_Flag=0;
    Flash_Message.close();
    Flash_Message.information(NULL,"加载FLM数据","载入成功成功~~！","好哒");
}


void MainWindow::FLM_Decode(void)
{
    QProcess        Algo(nullptr);//CMD
    QString         Scripts="/scripts/generate_blobs.py";
    QString         Scripts_Path;
    QString         Gen_Algo;
    QByteArray      Char_buf;
    char*           CMD_Path;
    QString         Flash_Algo_Buff;
    QStringList     Algo_List;
    QStringList     Code_List;
    uint32_t        Algo_Buff_Length;
    uint32_t        Code_Buff_Length;
    uint32_t        cnt=0;
    QString         App_Path=QDir::currentPath();
    QString         Flash_Code;
    QString         Flash_Algo;

    /******************CMD Path***************/
    Scripts_Path=App_Path+Scripts;
    Gen_Algo=tr("python ")+Scripts_Path+tr(" ")+FLM_Tmp_name;
    Char_buf=Gen_Algo.toLatin1();
    CMD_Path=Char_buf.data();

    /*************CMD**************/
    Algo.start("cmd");
    Algo.waitForStarted();
    Algo.write(CMD_Path);
    Algo.write("\r\n");
    Algo.closeWriteChannel();
    Algo.waitForFinished();

    QFile Blob(Blob_File_Name);
    if(Blob.open(QIODevice::ReadOnly))
    {
        QTextStream Blob_Message(&Blob);
        Flash_Algo_Buff=Blob_Message.readAll();
        Blob.close();
    }

    Flash_Code=Flash_Algo_Buff.mid(Flash_Algo_Buff.indexOf(code_start)+11,Flash_Algo_Buff.indexOf(code_end)-12);
    Flash_Algo=Flash_Algo_Buff.mid(Flash_Algo_Buff.indexOf(algo_start)+11,131);

    Code_List=Flash_Code.split(",",QString::SkipEmptyParts);
    Algo_List=Flash_Algo.split(",",QString::SkipEmptyParts);

    Code_Buff_Length=Code_List.length();
    Algo_Buff_Length=Algo_List.length();

    memset(FLM_Buff,0x00,1024);
     /************************************Flash code长度******************************************/
    for(cnt=0;cnt<4;cnt++)
    {
        FLM_Buff[cnt+16]=static_cast<char>((Code_Buff_Length>>(3-cnt)*8)&0xFF);
    }
    /*************************************Flash algo 长度*****************************************/
    for(cnt=0;cnt<4;cnt++)
    {
        FLM_Buff[20+cnt]=static_cast<char>((Algo_Buff_Length>>(3-cnt)*8)&0xFF);
    }

    /*************************************Flash code*****************************************/
    for(cnt=0;cnt<Code_Buff_Length;cnt++)
    {
        QString NumBuff=Code_List[cnt];
        QStringList Num_Buff_Hex=NumBuff.split("\n ",QString::SkipEmptyParts);
        QString Hex_Buff=Num_Buff_Hex[0];
        Flash_Algo_Hex[cnt]=Hex_Buff.toULong(NULL,16);
    }
    for(cnt=0;cnt<Code_Buff_Length;cnt++)
    {
        FLM_Buff[64+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt]>>24)&0xFF);
        FLM_Buff[65+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt]>>16)&0xFF);
        FLM_Buff[66+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt]>>8)&0xFF);
        FLM_Buff[67+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt])&0xFF);
    }

    /*************************************Flash algo*****************************************/
    memset(Flash_Algo_Hex,0x00,0x200);
    for(cnt=0;cnt<Algo_Buff_Length;cnt++)
    {
        QString NumBuff=Algo_List[cnt];
        QStringList Num_Buff_Hex=NumBuff.split("\n ",QString::SkipEmptyParts);
        QString Hex_Buff=Num_Buff_Hex[0];
        Flash_Algo_Hex[cnt]=Hex_Buff.toULong(NULL,16);
    }

    for(cnt=0;cnt<Algo_Buff_Length;cnt++)
    {
        FLM_Buff[768+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt]>>24)&0xFF);
        FLM_Buff[769+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt]>>16)&0xFF);
        FLM_Buff[770+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt]>>8)&0xFF);
        FLM_Buff[771+cnt*4]=static_cast<uint8_t>((Flash_Algo_Hex[cnt])&0xFF);
    }
}

void MainWindow::Gen_Program_File(void)
{
    FLM_Buff[1020]=Main_Flash_Length>>24     &   0xFF;
    FLM_Buff[1021]=Main_Flash_Length>>16     &   0xFF;
    FLM_Buff[1022]=Main_Flash_Length>>8      &   0xFF;
    FLM_Buff[1023]=Main_Flash_Length         &   0xFF;

    if(FLM_Read_Flag!=1)
    {
        QFile Program(Program_Tmp_name);
        Program.open(QIODevice::WriteOnly);
        QDataStream Program_Buff(&Program);
        Program_Buff.writeRawData((char *)FLM_Buff,1024);
        Program.close();
    }

    QProcess        Program_CMD(nullptr);//CMD
    QByteArray      Char_buf;
    char*           CMD_Path;

    /*****************************CMD Path****************************/
    QString Offine_Head="qt_temp.";
    QString Work_Path=FLM_Tmp_name.mid(0,FLM_Tmp_name.indexOf(Offine_Head));
    QString PTN=Program_Tmp_name.mid(Program_Tmp_name.indexOf(Offine_Head),-1);
    QString FTN=Flash_Tmp_name.mid(Flash_Tmp_name.indexOf(Offine_Head),-1);

    Program_File_Name=FLM_Tmp_name.mid(0,FLM_Tmp_name.indexOf(Offine_Head))+Program_Name;

    QString CMD_Path_Name=tr("copy /Y ")+tr("/b ")+PTN+tr("+")+FTN+tr(" ")+Program_Name;

    Char_buf=CMD_Path_Name.toLatin1();
    CMD_Path=Char_buf.data();
    qDebug()<<CMD_Path;

    /****************************CMD**********************************/
    Program_CMD.setWorkingDirectory(Work_Path);
    Program_CMD.start("cmd");
    Program_CMD.waitForStarted();
    Program_CMD.write(CMD_Path);
    Program_CMD.write("\n\r");
    Program_CMD.closeWriteChannel();
    Program_CMD.waitForFinished();
}
