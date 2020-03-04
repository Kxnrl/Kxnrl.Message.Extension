set EXT_DIR=%cd%
set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if "%VSCMD_VER%"=="" (
	set MAKE=
	set CC=
	set CXX=
	call %VCVARSALL% x86 8.1
)

git clone https://github.com/alliedmodders/sourcemod --recursive --branch %BRANCH% --single-branch %EXT_DIR%/sourcemod-%BRANCH%

mkdir build
pushd build
python ../configure.py --enable-optimize --sm-path %EXT_DIR%/sourcemod-%BRANCH% && ambuild
popd
