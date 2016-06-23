#if !defined(BILIGLOBALDATAS_H)
#define BILIGLOBALDATAS_H

#include <string>
#include <stdint.h>

__declspec(selectany) bool gBili_isDisableLogin;

//file information
__declspec(selectany) std::wstring gBili_fileVersion;

//login data
__declspec(selectany) std::string gBili_mid;
__declspec(selectany) uint64_t gBili_expires;

__declspec(selectany) uint64_t gBili_roomId; //此数据只有在OnRetryButtonClicked执行完成之后才可以使用
__declspec(selectany) std::string gBili_userLoginName; //登录框里面输入的
__declspec(selectany) std::string gBili_userName; //此数据只有在主窗口构造里面OnUserInfoGot第一次调用及之后才能使用
__declspec(selectany) std::string gBili_userFace; //此数据只有在主窗口构造里面OnUserInfoGot第一次调用及之后才能使用
__declspec(selectany) std::string gBili_pushServer; //此数据暂时没在用
__declspec(selectany) std::string gBili_pushPath; //此数据暂时没在用
__declspec(selectany) std::string gBili_danmakuServer; //弹幕服务器地址

__declspec(selectany) std::string gBili_faceUrl;

__declspec(selectany) uint64_t gBili_rcost;
__declspec(selectany) uint64_t gBili_audienceCount;

#endif
