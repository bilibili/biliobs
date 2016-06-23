call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"
cd /d "%~dp0"
csc /out:fix_biliobs_zh_ts.exe fix_biliobs_zh_ts.cs
pause
