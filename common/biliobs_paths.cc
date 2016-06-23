#include "biliobs_paths.h"
#include <windows.h>
#include <Shlobj.h>

namespace biliobs {

#define kAPPDATA_FLODER_NAME             L"bililive"
#define kAPPDATA_USERDATA_FLODER_NAME    L"bililive\\userdata"
#define DMP_FLODER_NAME                  L"dmp"

class PathCache{
public:
    static PathCache& Instance(){
        static PathCache a;
        return a;
    }

    const std::wstring& GetUserDataPath(){
        return _userdata_path;
    }
    const std::wstring& GetRootPath(){
        return _root_path;
    }
	const std::wstring& GetDmpPath(){
		return _dmp_path;
	}
private:
    PathCache(){
        init();
    }
    void init(){
        wchar_t swzPath[MAX_PATH] = { 0 };
        SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, swzPath);

		std::wstring appdata_root = swzPath;
		appdata_root.append(L"\\" kAPPDATA_FLODER_NAME L"\\");
		CreateDirectoryW(appdata_root.c_str(), NULL);

        _userdata_path = swzPath;
		_userdata_path.append(L"\\" kAPPDATA_USERDATA_FLODER_NAME L"\\");
		CreateDirectoryW(_userdata_path.c_str(), NULL);

		_dmp_path = _userdata_path;
		_dmp_path.append(DMP_FLODER_NAME L"\\");
		CreateDirectoryW(_dmp_path.c_str(), NULL);

        memset(swzPath, 0, sizeof(swzPath));
        GetModuleFileNameW(NULL, swzPath, 260 - 1);
        size_t off = _root_path.rfind('\\');
        if( std::wstring::npos != off ){
            _root_path.resize(off);
        }
        _userdata_path.append(L"\\");
		_dmp_path.append(L"\\");
    }

private:
    std::wstring _userdata_path;
    std::wstring _root_path;
	std::wstring _dmp_path;
};


const std::wstring& GetRootPath(){
    return PathCache::Instance().GetRootPath();
}


const std::wstring& GetUserDataPath(){
    return PathCache::Instance().GetUserDataPath();
}

const std::wstring& GetDmpPath()
{
	AssureUserDataDirectoryExists(DMP_FLODER_NAME);
	return PathCache::Instance().GetDmpPath();
}

void AssureUserDataDirectoryExists(const std::wstring dir_name){
    std::wstring full_name = GetUserDataPath() + dir_name + L"\\";
    CreateDirectoryW(full_name.c_str(), NULL);
}


}
