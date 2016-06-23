
set PROJECT_ROOT=%~dp0\..\
set PROJECT_PLUGINS_ROOT=%PROJECT_ROOT%\plugins
set PROJECT_RELEASE_PATH=%PROJECT_ROOT%build\Release
set QTDIR="C:\Qt\Qt5.5.0\5.5\msvc2013"

mkdir %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\curl\dlls\*.dll %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\directx_setup\*.dll %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\directx_setup\.exe %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\ffmpeg\*.dll %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\openssl\*.dll %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\x264\*.dll %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\third_party_lib\zlib\*.dll %PROJECT_RELEASE_PATH%
copy %QTDIR%\bin\Qt5Core.dll  %PROJECT_RELEASE_PATH%
copy %QTDIR%\bin\Qt5Gui.dll  %PROJECT_RELEASE_PATH%
copy %QTDIR%\bin\Qt5Widgets.dll  %PROJECT_RELEASE_PATH%
copy %QTDIR%\bin\Qt5Sql.dll  %PROJECT_RELEASE_PATH%
copy %QTDIR%\bin\qt.conf  %PROJECT_RELEASE_PATH%
mkdir %PROJECT_RELEASE_PATH%\imageformats
xcopy /S /E %QTDIR%\plugins\imageformats\*.dll  %PROJECT_RELEASE_PATH%\imageformats
mkdir %PROJECT_RELEASE_PATH%\platforms
xcopy /S /E %QTDIR%\plugins\platforms\*.dll  %PROJECT_RELEASE_PATH%\platforms

mkdir %PROJECT_RELEASE_PATH%\data
mkdir %PROJECT_RELEASE_PATH%\data\libobs
copy %PROJECT_ROOT%\libobs\data\*.*   %PROJECT_RELEASE_PATH%\data\libobs\*.*

copy %PROJECT_ROOT%\biliapi\bililogin.dll  %PROJECT_RELEASE_PATH%
copy %PROJECT_ROOT%\build\libcurl.dll  %PROJECT_RELEASE_PATH%

call :copy_data coreaudio-encoder %PROJECT_RELEASE_PATH%
call :copy_data image-source %PROJECT_RELEASE_PATH%
call :copy_data obs-ffmpeg %PROJECT_RELEASE_PATH%
call :copy_data obs-filters %PROJECT_RELEASE_PATH%
call :copy_data obs-outputs %PROJECT_RELEASE_PATH%
call :copy_data obs-x264 %PROJECT_RELEASE_PATH%
call :copy_data rtmp-services %PROJECT_RELEASE_PATH%
call :copy_data text-freetype2 %PROJECT_RELEASE_PATH%
call :copy_data win-capture %PROJECT_RELEASE_PATH%
call :copy_data win-dshow %PROJECT_RELEASE_PATH%
call :copy_data decklink %PROJECT_RELEASE_PATH%
call :copy_data win-mf %PROJECT_RELEASE_PATH%
call :copy_data win-wasapi %PROJECT_RELEASE_PATH%

exit /b

:copy_data 
set PLUGIN_NAME=%1
set OUTPUT_BASE_DIR=%2
set OUTPUT_DIR=%OUTPUT_BASE_DIR%\data\%PLUGIN_NAME%
mkdir %OUTPUT_DIR%
pushd %OUTPUT_DIR%
xcopy /S /E /Y %PROJECT_PLUGINS_ROOT%\%PLUGIN_NAME%\data\*.* .
popd
