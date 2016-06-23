#include "BiLiOBSMainWid.h"
#include "../common/BiliJsonHelper.hpp"
#include "../biliapi/IBiliApi.h"
#include "BiliOBSUtility.hpp"
#include "BiliGlobalDatas.hpp"
#include "BiLiMsgDlg.h"

#include <QDesktopServices>
#include <QUrl>
#include <QMenu>
#include <QSignalMapper>

#include "system_ret_info_dlg.h"
#include "system_inquiry_dlg.h"

extern bool bIsShouldRestart;

void BiLiOBSMainWid::mCreateRightMenu() {

	mMoreMenu = new QMenu(this);
	if (mMoreMenu->objectName().isEmpty())
		mMoreMenu->setObjectName(tr("MoreMenu"));

	QVector<char *> actTxtList;
	//<< QT_TR_NOOP("Live Commenting Set") 
	//<< QT_TR_NOOP("Using Boot") 
	actTxtList << QT_TR_NOOP("GotoMyRoom") << QT_TR_NOOP("Help") << QT_TR_NOOP("Update Check") << QT_TR_NOOP("Logout") << QT_TR_NOOP("Quit");

	QSignalMapper *sigMapper = new QSignalMapper(this);
	for each (char *actTxt in actTxtList) {
		QAction *act = new QAction(tr(actTxt), this);
		mMoreMenu->addAction(act);
		mMoreMenuActV.push_back(act);

		connect(act, SIGNAL(triggered()), sigMapper, SLOT(map()));
		sigMapper->setMapping(act, actTxt);
	}
	connect(sigMapper, SIGNAL(mapped(const QString&)), this, SLOT(mSltMoreMenuActClicked(const QString&)));
}

void BiLiOBSMainWid::mSltGotoMyRoomAct()
{
	OnOpenRoomClicked();
}

void BiLiOBSMainWid::mShowRightMenu(QPoint pos){

	for each (QAction *act in mMoreMenuActV)
		act->setText(QApplication::translate("BiLiOBSMainWid", act->text().toUtf8().data(), 0));

	QPoint pos2 = ui.MoreBtn->geometry().bottomLeft();
    QPoint pos_rgiht = ui.MoreBtn->geometry().bottomRight();

    pos2 = (pos2 + pos_rgiht) / 2;
	pos2 = mapToGlobal(pos2);

    pos2.setX(pos2.x() - mMoreMenu->width() / 2);

	pos2.setY(pos2.y() + 5); //为了能和userinfo widget对齐
	mMoreMenu->exec(pos2);
	//mMoreMenu->exec(pos);
}

void BiLiOBSMainWid::mSltMoreBtnClicked() {

	//	mSltChangeLang();
	QPoint pos = static_cast<QWidget *>(sender())->mapToGlobal(QPoint(-5, 20));
	mShowRightMenu(pos);
}

void BiLiOBSMainWid::mSltMoreMenuActClicked(const QString &actTxt){

	QStringList actTxtL = actTxt.trimmed().split(" ");
	QString actMethod = QString("mSlt%1%2").arg(actTxtL.join("")).arg("Act");
	QMetaObject::invokeMethod(this, actMethod.toLocal8Bit().data());
}

void BiLiOBSMainWid::mSltLiveCommentingSetAct() {
}

void BiLiOBSMainWid::mSltUsingBootAct() {
}

void BiLiOBSMainWid::mSltHelpAct() {
	//注意：登录窗口的最下面也有一个帮助按钮！
	//修改这里的时候请考虑那边要不要一起改
	//BiLiMsgDlg msgDlg;
	//msgDlg.mSetTitle(tr("Information"));
	//msgDlg.mSetMsgTxtAndBtn(tr("Please content maomao in QQ."), false);
	//msgDlg.exec();

    SystemRetInfoDlg dlg;
    dlg.setTitle("");
    dlg.setSubTitle(tr("Information"));
    dlg.setDetailInfo(tr("Please content maomao in QQ."));
    dlg.resize(dlg.sizeHint());
    move_widget_to_center(&dlg, this);
    dlg.exec();


	/*
	const QUrl url("http://www.bilibili.com/html/help.html#l");
	QDesktopServices::openUrl(url);
	*/
}

void BiLiOBSMainWid::mSltQuitAct() {
	close();
}

void BiLiOBSMainWid::mSltUpdateCheckAct() {
    SystemRetInfoDlg *msgDlg = new SystemRetInfoDlg();
    msgDlg->setSubTitle(tr("Information"));
    msgDlg->setDetailInfo(tr("Checking newest version from server..."));

    msgDlg->show();

	worker.AddTask(std::bind(&BiLiOBSMainWid::mCheckNewVersion, this, msgDlg));
}

void BiLiOBSMainWid::mSltLogoutAct() {
	if (close())
		bIsShouldRestart = true;
}

void* BiLiOBSMainWid::mCheckNewVersion(SystemRetInfoDlg* msgDlg)
{
	try
	{
		//retrive current version
		int v1, v2, v3, v4;
		if (swscanf(gBili_fileVersion.c_str(), L"%d.%d.%d.%d", &v1, &v2, &v3, &v4) == 4)
		{
			if (v4 == 0) //没有build号的，不检查更新
			{
				if (msgDlg != nullptr)
					QMetaObject::invokeMethod(msgDlg, "close");
				return 0;
			}

			int64_t currentTime = _time64(0);

			if (msgDlg == 0) //对于自动检查更新，一天最多一次
			{
				int64_t lastCheckedTime = config_get_int(mBasicConfig, "LastOperations", "CheckUpdateTime");

				if (currentTime - lastCheckedTime < 24 * 60 * 60)
				{
					return 0;
				}
			}

			BiliJsonPtr result = biliApi->GetNewestVersion();

			if (result->GetVal<JSON_INTEGER>({ "ver" }) > v4)
			{
				std::string updateUrl = result->GetVal<JSON_STRING>({ "updateUrl" });

				mInvokeProcdure(BiliThreadWorker::TaskT(
					std::bind(&BiLiOBSMainWid::mAskForOpenNewVersionPage, this, msgDlg == nullptr, QString(updateUrl.c_str()))
					));
			}
			else
			{
				if (msgDlg)
					QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("No new version found.")));
			}

			config_set_int(mBasicConfig, "LastOperations", "CheckUpdateTime", currentTime);
		}
		else //未能获取到当前文件的版本
		{
			if (msgDlg)
				QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Fail to get installed version. Please check online.")));
		}
	}
	catch (CUrlNetworkException&)
	{
		//网络错误
		if (msgDlg)
			QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Network error.")));
	}
	catch (JsonDataError&)
	{
		//服务器内部错误
		if (msgDlg)
			QMetaObject::invokeMethod(this, "OnErrorMessage", Q_ARG(QString, tr("Server error.")));
	}

    if (msgDlg != nullptr) {
        QMetaObject::invokeMethod(msgDlg, "close");
        msgDlg->deleteLater();
    }

	return 0;
}

void* BiLiOBSMainWid::mAskForOpenNewVersionPage(bool shouldAsk, QString Url)
{
	bool shouldOpenWebPage = true;

	if (shouldAsk) {

        SystemRetInfoDlg ifOpenDlg;
        ifOpenDlg.setTitle("");
        ifOpenDlg.setSubTitle(tr("Information"));
        ifOpenDlg.setDetailInfo(tr("Found new version. Update now?"));


		if (ifOpenDlg.exec() == QDialog::Rejected)
			shouldOpenWebPage = false;
	}

	if (shouldOpenWebPage)
		QDesktopServices::openUrl(QUrl(Url));

	return 0;
}
