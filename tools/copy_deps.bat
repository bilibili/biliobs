

set PROJECT_ROOT=%~dp0\..\
set PROJECT_PLUGINS_ROOT=%PROJECT_ROOT%\plugins
set PROJECT_DEBUG_PATH=%PROJECT_ROOT%build\Debug
set PROJECT_RELEASE_PATH=%PROJECT_ROOT%build\Debug
set QTDIR="C:\Qt\Qt5.14.1\5.14.1\msvc2017"

mkdir %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\curl\dlls\*.dll %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\directx_setup\*.dll %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\directx_setup\.exe %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\ffmpeg\*.dll %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\openssl\*.dll %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\x264\*.dll %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\third_party_lib\zlib\*.dll %PROJECT_DEBUG_PATH%
copy %QTDIR%\bin\Qt5Cored.dll  %PROJECT_DEBUG_PATH%
copy %QTDIR%\bin\Qt5Guid.dll  %PROJECT_DEBUG_PATH%
copy %QTDIR%\bin\Qt5Widgetsd.dll  %PROJECT_DEBUG_PATH%
copy %QTDIR%\bin\Qt5Sqld.dll  %PROJECT_DEBUG_PATH%
copy %QTDIR%\bin\qt.conf  %PROJECT_DEBUG_PATH%
mkdir %PROJECT_DEBUG_PATH%\imageformats
xcopy /S /E %QTDIR%\plugins\imageformats\*d.dll  %PROJECT_DEBUG_PATH%\imageformats
mkdir %PROJECT_DEBUG_PATH%\platforms
xcopy /S /E %QTDIR%\plugins\platforms\*d.dll  %PROJECT_DEBUG_PATH%\platforms

mkdir %PROJECT_DEBUG_PATH%\data
mkdir %PROJECT_DEBUG_PATH%\data\libobs
copy %PROJECT_ROOT%\libobs\data\*.*   %PROJECT_DEBUG_PATH%\data\libobs\*.*

copy %PROJECT_ROOT%\biliapi\bililogin.dll  %PROJECT_DEBUG_PATH%
copy %PROJECT_ROOT%\build\libcurl.dll  %PROJECT_DEBUG_PATH%

call :copy_data coreaudio-encoder %PROJECT_DEBUG_PATH%
call :copy_data image-source %PROJECT_DEBUG_PATH%
call :copy_data obs-ffmpeg %PROJECT_DEBUG_PATH%
call :copy_data obs-filters %PROJECT_DEBUG_PATH%
call :copy_data obs-outputs %PROJECT_DEBUG_PATH%
call :copy_data obs-x264 %PROJECT_DEBUG_PATH%
call :copy_data rtmp-services %PROJECT_DEBUG_PATH%
call :copy_data text-freetype2 %PROJECT_DEBUG_PATH%
call :copy_data win-capture %PROJECT_DEBUG_PATH%
call :copy_data win-dshow %PROJECT_DEBUG_PATH%
call :copy_data win-mf %PROJECT_DEBUG_PATH%
call :copy_data win-wasapi %PROJECT_DEBUG_PATH%

exit /b

:copy_data 
set PLUGIN_NAME=%1
set OUTPUT_BASE_DIR=%2
set OUTPUT_DIR=%OUTPUT_BASE_DIR%\data\%PLUGIN_NAME%
mkdir %OUTPUT_DIR%
pushd %OUTPUT_DIR%
xcopy /S /E /Y %PROJECT_PLUGINS_ROOT%\%PLUGIN_NAME%\data\*.* .
popd
