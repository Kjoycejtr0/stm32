@echo off

set FileName=%1

echo 源文件：%FileName%
echo.
echo 生成Flash算法参数文件为：c_blob.c
echo.
echo Decive Flash参数为：
echo.
python generate_blobs.py %FileName%

pause

