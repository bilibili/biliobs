#include <QtWidgets/QApplication>
#include "BiLiApp.h"

#include "BiliGlobalDatas.hpp"

#include <Windows.h>
#include <iostream>
#include <DbgHelp.h>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QProcess>
#include <QObject>

#include "BiLiMsgDlg.h"
#include "BiliOBSUtility.hpp"

#include "base/command_line.h"
#include "third_party_lib/directx_setup/include/dsetup.h"

#include "../common/biliobs_paths.h"
#include "system_ret_info_dlg.h"

#pragma comment(lib, "dbghelp.lib")


#ifdef WIN32
#include <Windows.h>
static const wchar_t* InstanceMutexName = L"E82991C9-BFF7-4F93-B6FE-E05D9EB71ADF";
#endif

bool bIsShouldRestart;

void CreateDumpFile(LPCWSTR lpDumpFilePath, EXCEPTION_POINTERS *pE){

	HANDLE hDumpF = CreateFile(lpDumpFilePath, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pE;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = FALSE;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpF, MiniDumpNormal,
		&dumpInfo, NULL, NULL);

	CloseHandle(hDumpF);
}

LONG AppCrashHandleCallback(EXCEPTION_POINTERS *pE){


	QString path = QString::fromStdWString(biliobs::GetDmpPath());

	wchar_t filePath[MAX_PATH] = { 0 };
	QString fileName = QString("biliobs_%1_%2.dmp").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")).arg( QString::fromStdWString(gBili_fileVersion));
	
	(path + fileName).toWCharArray(filePath);
	CreateDumpFile(filePath, pE);

	wchar_t tip[MAX_PATH] = { 0 };
	//QStringLiteral("请联系我们，并将错误报告文件 '%1' 发送给我们，谢谢!").arg(fileName).toWCharArray(tip);
    QString("\350\257\267\350\201\224\347\263\273\346\210\221\344\273\254,"
            "\345\271\266\345\260\206\351\224\231\350\257\257\346\212\245\345\221\212\346\226\207\344\273\266 "
            "'%1' \345\217\221\351\200\201\347\273\231\346\210\221\344\273\254,\350\260\242\350\260\242!").arg(fileName).toWCharArray(tip);
	//FatalAppExit(-1, tip);

	MessageBox(0, tip, 0, 0);



	/*open explorer*/
	QString pure_path = path;
	pure_path.resize(path.length() - 1);
	pure_path.append(fileName);

	QString arg("/select, ");
	arg.append(pure_path);

	ShellExecuteW(NULL, L"open", L"explorer.exe", arg.toStdWString().c_str(), NULL, SW_SHOWNORMAL);
	/*sleep用于使新启进程能够raise到窗口上层*/
	Sleep(1000);

	return EXCEPTION_EXECUTE_HANDLER;
}

bool DetectDX(){

	DWORD dwVersion = 0;
	DWORD dwRevision = 0;
	if (!DirectXSetupGetVersion(&dwVersion, &dwRevision))
		return false;

	HMODULE dxDll = LoadLibrary(L"D3DCompiler_43.dll");
	if (!dxDll)
		return false;

	return true;
}

static void OnProcessError(QProcess::ProcessError e)
{
	QMessageBox m;
	m.setText(QString(e));
	m.exec();
}

static void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus){
	QString msg = QString("%1, %2").arg(exitCode).arg(exitStatus);
	QMessageBox m;
	m.setText(msg);
	m.exec();
}

bool InstallDX(){

	QString curPath = QDir::currentPath();
	curPath.append("\\DxWebUpdate.exe");
	curPath = QString("\"%1\"").arg(curPath);

	QProcess *process = new QProcess();
	QObject::connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), &OnProcessError);
	QObject::connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), &OnProcessFinished);

	process->execute(curPath);

	return true;

}


int main(int argc, char *argv[]) {
	base::CommandLine::Init(argc, argv);
     base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	 QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#ifdef WIN32
	
	
	HANDLE instanceMutex = CreateMutexW(NULL, TRUE, InstanceMutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(instanceMutex);
		return 0;
	}

		if (cmd_line->HasSwitch("console")) {
			AllocConsole();
			freopen("con", "w", stdout);
		}




#endif

	bIsShouldRestart = false;

	BiLiApp BiLiOBSApp(argc, argv);
	BiLiOBSApp.mLoadTranslator();
	BiLiOBSApp.setWindowIcon(QIcon(":/SysIcon/BlueIcon"));

	gBili_fileVersion = BiliGetFileVertion(QCoreApplication::applicationFilePath().toStdWString());
	//gBili_fileVersion = L"3.13.3.1616";
	QFile styleFile(":/QSS/BiLiStyle");
	if (styleFile.open(QFile::ReadOnly)) {
		qApp->setStyleSheet(styleFile.readAll());
		styleFile.close();
	}else
		return 0;

	if (!DetectDX()){

		//BiLiMsgDlg dlg;
		//dlg.mSetMsgTxtAndBtn(QObject::tr("Can not detect DirectX! Do you want to install?"));
		//dlg.mSetTitle(QObject::tr("Warning"));
		//dlg.exec();

        SystemRetInfoDlg dlg;
        dlg.setDetailInfo(QObject::tr("Can not detect DirectX! Do you want to install?"));
        dlg.setSubTitle(QObject::tr("Warning"));
        dlg.setTitle("");
        dlg.exec();

		if (!dlg.result())
			return 0;
		InstallDX();
	}

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)AppCrashHandleCallback);

	//check disable auto login
  if (cmd_line->HasSwitch("disable-auto-login")){
        gBili_isDisableLogin = true;
    }else{
        gBili_isDisableLogin = false;
    }

	int r;
	if (BiLiOBSApp.mInitApp())
		r = BiLiOBSApp.exec();

	if (instanceMutex)
		CloseHandle(instanceMutex);

	if (bIsShouldRestart){
		RestartWithoutAutoLogin();
		/*sleep用于使新启进程能够raise到窗口上层*/
		Sleep(1000);
	}
	return r;
}
