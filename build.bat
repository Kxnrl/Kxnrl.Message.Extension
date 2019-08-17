
:: Edited From https://github.com/dordnung/System2/blob/master/build.bat

echo "Set environment"
set EXT_DIR=%cd%
set BUILD_DIR=%EXT_DIR%\build-windows
set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "%BUILD_DIR%" (
	mkdir "%BUILD_DIR%"
)

cd %BUILD_DIR%

if "%VSCMD_VER%"=="" (
	set MAKE=
	set CC=
	set CXX=
	call %VCVARSALL% x86 8.1
)

:: Getting sourcemod

echo "Download sourcemod"
if not exist "sourcemod-%BRANCH%" (
	git clone https://github.com/alliedmodders/sourcemod --recursive --branch %BRANCH% --single-branch sourcemod-%BRANCH%
)

cd sourcemod-%BRANCH%
set SOURCEMOD=%cd%

cd %BUILD_DIR%

:: Start build

echo "Build"
cd %EXT_DIR%
msbuild msvc15/Kxnrl.Message.sln /p:Platform="win32"