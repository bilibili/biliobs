#include "BiliApiManager.h"
#include "BiliGlobalDatas.hpp"
#include "biliapi/IBiliApi.h"
#include "BiliOBSUtility.hpp"

#include <windows.h>
#include <string>
#include <algorithm>
#include <iterator>
#include <memory>
#include <functional>
#include <QVariant>


IBiliAPI* BiliAPIMan::instance_;
int BiliAPIMan::errorCode_ = CREATE_API_SUCCESS;
typedef std::shared_ptr<IBiliLogin> IBiliLoginPtr;

IBiliAPI* BiliAPIMan::GetInstance() {

    if (instance_)
        return instance_;
    
    HMODULE hModule = LoadLibraryW(L"bililogin.DLL");
   // HMODULE hModule = LoadLibraryW(L"bililive_secret.dll");
  
    if (hModule) {

        int (__stdcall *CreateBiliApiEngine)(const char* fileVersion, IBiliAPI** pApi);
        *(LPVOID*)&CreateBiliApiEngine = GetProcAddress(hModule, "_CreateBiliApiEngine@8");
        if (CreateBiliApiEngine) {

			std::string tmp;
			std::copy(gBili_fileVersion.begin(), gBili_fileVersion.end(), std::back_inserter(tmp));
            CreateBiliApiEngine(tmp.c_str(), &instance_);
            
			if (!instance_)
				exit(0);

			IBiliLogin* (__stdcall *getBiliLogin)(void *api);
	        *(LPVOID*)&getBiliLogin = GetProcAddress(hModule, "getBiliLogin");
			if (getBiliLogin){
				IBiliLoginPtr pBiliLogin(getBiliLogin((void *)instance_), std::mem_fn(&IBiliLogin::release_));

				QObject *obj = (QObject *)(pBiliLogin.get()->getOut_());
				gBili_mid = (obj->property("mid")).value<QString>().toStdString();
				gBili_expires  = *(uint64_t *)(obj->property("expires")).value<void *>();
				gBili_userLoginName = (obj->property("usrLoginName")).value<QString>().toStdString();
			}
        }
        else { 
            FreeLibrary(hModule);
        }
    }
    
    return instance_;
}

int BiliAPIMan::GetErrorCode()
{
    return errorCode_;
}