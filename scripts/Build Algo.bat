@echo off

set FileName=%1

echo Դ�ļ���%FileName%
echo.
echo ����Flash�㷨�����ļ�Ϊ��c_blob.c
echo.
echo Decive Flash����Ϊ��
echo.
python generate_blobs.py %FileName%

pause

