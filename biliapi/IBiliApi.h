#ifndef IBILIAPI_H
#define IBILIAPI_H

#include <vector>
#include <string>
#include <QString>

#include "../../../common/BiliJsonHelper.hpp"

//=======================================================
//               HTTP访问异常
//=======================================================
class CUrlNetworkException
{
	int errorCode;
public:
	CUrlNetworkException(int err) : errorCode(err) {}
	int GetErrorCode() const { return errorCode; }
};

//=======================================================
//                自定义异常
//=======================================================
class BiliCustomException : public std::exception
{
	std::string message_;
	int code_;
public:
	BiliCustomException(const char* msg) : message_(msg), code_(0) {}
	BiliCustomException(int code, const char* msg) : message_(msg), code_(code) {}
	BiliCustomException(const std::string msg) : message_(msg), code_(0) {}
	BiliCustomException(int code, const std::string msg) : message_(msg), code_(code) {}
#ifdef QT_VERSION
	BiliCustomException(const QString& msg) : message_(msg.toUtf8().data()), code_(0) {}
	BiliCustomException(int code, const QString& msg) : message_(msg.toUtf8().data()), code_(code) {}
#endif
	const char* what() const override { return message_.c_str(); }
	int code() const { return code_; }
};

//=======================================================
//                编解码异常
//=======================================================
class WrongBase64StringException {};
class WrongEncodedDataException {};


class IBiliAPI
{
	IBiliAPI(const IBiliAPI&);
	IBiliAPI& operator= (const IBiliAPI&);

protected:
	IBiliAPI() {}

public:
	virtual ~IBiliAPI() = 0 {}

	virtual bool IsLoggedIn() = 0;
	virtual bool ClearLoginInfo() = 0;

	virtual const std::vector<char>& GetLastContent() = 0;
	virtual const std::string GetLastUrl() = 0;

	virtual BiliJsonPtr GetNewestVersion() = 0;

	virtual BiliJsonPtr GetRoomInfo(int uid) = 0;
	virtual BiliJsonPtr LiveStatusMng(int roomId, bool status) = 0;
	virtual BiliJsonPtr MyInfo() = 0;

	virtual std::string GetRoomAdminUrl(const std::string& tag = std::string()) = 0;
	virtual BiliJsonPtr IsAnchor() = 0;

	virtual BiliJsonPtr GetCutoffReason(int rid) = 0;
	virtual BiliJsonPtr GetAPIRoomInfo(int roomid) = 0;

	virtual std::vector<char> GetFace(const std::string& faceUrl) = 0;
};

struct IBiliLogin{
	virtual void release_() = 0;
	virtual void *getOut_() = 0;
};

enum
{
	CREATE_API_SUCCESS = 0,
	CREATE_API_ERROR_SIGNATURE_FAILED,
	CREATE_API_ERROR_OTHER_REASON
};

extern "C"
#ifdef BUILDING_DLL
__declspec(dllexport)
#endif
int __stdcall CreateBiliApiEngine(const char* fileVersion, IBiliAPI** pApi);


extern "C"
#ifdef BUILDING_DLL
__declspec(dllexport)
#endif
IBiliLogin* _stdcall getBiliLogin(void *api);



#endif
