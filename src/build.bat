@echo off
IF NOT EXIST ..\build\ mkdir ..\build
pushd ..\build\
cl -nologo -O2 -W3 -FC -Z7 -EHsc -D_CRT_SECURE_NO_WARNINGS ..\src\main.cpp 
popd

pushd ..\data
..\build\main.exe
start test.bmp
popd
