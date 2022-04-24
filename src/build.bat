@echo off

IF NOT EXIST ..\build\ mkdir ..\build
pushd ..\build\
cl -nologo -O2 -W3 -FC -Z7 -EHsc -D_CRT_SECURE_NO_WARNINGS /Qvec-report:2 ..\src\main.cpp 
popd

pushd ..\test
..\build\main.exe %1
start test.bmp
popd
