echo "Build type %BUILD%"
@echo on
goto %BUILD%
echo "Error: unknown build type %BUILD%"
goto eof

:MSVCDYNAMIC
call "ci\openssl.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cd win32
perl Configure --config=release --with-sdk --with-ipv6 --with-winextdll --linktype=dynamic --with-ssl --with-sslincdir=C:\OpenSSL-Win64\include --with-ssllibdir=C:\OpenSSL-Win64\lib\vc
nmake
goto eof

:MSVCSTATIC
call "ci\openssl.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cd win32
perl Configure --config=release --with-sdk --with-ipv6 --with-winextdll --linktype=static --with-ssl --with-sslincdir=C:\OpenSSL-Win64\include --with-ssllibdir=C:\OpenSSL-Win64\lib\vc

nmake
goto eof

:MinGW
C:\msys64\usr\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:Cygwin
c:\cygwin\bin\bash --login -c 'set -x; cd "${APPVEYOR_BUILD_FOLDER}"; ci/build.sh'
goto eof

:eof
