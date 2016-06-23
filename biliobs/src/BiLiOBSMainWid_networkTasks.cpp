#include "BiLiOBSMainWid.h"

#include "BiliGlobalDatas.hpp"
#include "../biliapi/IBiliApi.h"
#include "BiliOBSUtility.hpp"
#include "BiliUIConfigSync.hpp"

void* BiLiOBSMainWid::mGetUserFacePixmapTask()
{
	try
	{
		BiliJsonPtr result = biliApi->MyInfo();
		gBili_userName = result->GetVal<JSON_STRING>({ "uname" });
		gBili_userFace = result->GetVal<JSON_STRING>({ "face" });

		std::vector<char> faceData = biliApi->GetFace(gBili_userFace);
		QPixmap userFace;
		if (!faceData.empty())
			userFace.loadFromData((uchar*)&faceData[0], faceData.size());

		QMetaObject::invokeMethod(this, "OnUserInfoGot", Q_ARG(QString, gBili_userName.c_str()), Q_ARG(QPixmap, userFace));

		//存最后登录信息
		ConfigFile cf;
		if (cf.Open(BiliConfigFile::GetLoginConfigPath().c_str(), CONFIG_OPEN_ALWAYS) == CONFIG_SUCCESS)
		{
			std::string userFaceData = BiliBinToStr(faceData);

			config_set_encryptedstdstring(cf, "LastLoginInfo", "UserName", gBili_userName);
			config_set_encryptedstdstring(cf, "LastLoginInfo", "UserId", gBili_userLoginName);
			config_set_encryptedstdstring(cf, "LastLoginInfo", "UserFace", userFaceData);

			cf.SaveSafe("tmp");
			cf.Close();
		}
	}
	catch (CUrlNetworkException&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), "Network error.");
		//网络错误
	}
	catch (JsonDataError&)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), biliApi->GetLastContent());

		//服务器内部错误
	}

	return 0;
}

void* BiLiOBSMainWid::mGetUserFacePixmapTaskWrapper(wcs::WeakRef<BiLiOBSMainWid> own, void*(BiLiOBSMainWid::*task)())
{
    
    return own.doOwnTask(task);


}

void* BiLiOBSMainWid::mUpdateRoomPresentCountTask()
{
	try
	{
		BiliJsonPtr roomInfoResult = biliApi->GetRoomInfo(lexical_cast<int>(gBili_mid));
		gBili_rcost = roomInfoResult->GetVal<JSON_INTEGER>({ "data", "rcost" }) / 100;
	}
	catch (...)
	{
		SaveHttpLogToFile(biliApi->GetLastUrl(), biliApi->GetLastContent());
	}

	return 0;
}

void* BiLiOBSMainWid::mUpdateRoomPresentCountTaskWrapper(wcs::WeakRef<BiLiOBSMainWid> own, void*(BiLiOBSMainWid::*task)())
{
    return own.doOwnTask(task);

    return 0;
}
