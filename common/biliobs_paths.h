#if !defined(__COMMON_BILIOBS_PATHS_H__)
#define __COMMON_BILIOBS_PATHS_H__

#include <string>

namespace biliobs {
    const std::wstring& GetUserDataPath();
    const std::wstring& GetRootPath();

	const std::wstring& GetDmpPath();

    void AssureUserDataDirectoryExists(const std::wstring dir_name);
}

#endif