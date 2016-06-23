#if !defined(__COMMON_BILIOBS_PATH_H__)
#define __COMMON_BILIOBS_PATH_H__
#include <windows.h>
#if defined(__cplusplus)
extern "C" IMAGE_DOS_HEADER __ImageBase;
#else
extern IMAGE_DOS_HEADER __ImageBase;
#endif
static inline void biliobs_get_root_path(struct dstr* dir_path){
    wchar_t tmp_path[260] = { 0 };
    wchar_t* p = NULL;
    GetModuleFileNameW((HMODULE)&__ImageBase, tmp_path, 260 - 1);
    p = wcsrchr(tmp_path, L'\\');
    if( NULL != p )
        *p = 0;
    dstr_ensure_capacity(dir_path, 260 + 1);
    dir_path->len = os_wcs_to_utf8(tmp_path, wcslen(tmp_path), dir_path->array, dir_path->capacity);
    dstr_cat_ch(dir_path, '\\');
}

static inline bool biliobs_check_root_path(const char *data, const char *path, struct dstr *output)
{
	struct dstr dir_path;
	dstr_init(&dir_path);
	biliobs_get_root_path(&dir_path);
	dstr_copy_dstr(output, &dir_path);
	dstr_cat(output, path);
	dstr_cat(output, data);
	return os_file_exists(output->array);
}

#if defined(__cplusplus)
static inline bool biliobs_check_root_path(const char* data, const char *path, std::string &output)
{
    struct dstr output_path;
    dstr_init(&output_path);
    if(!biliobs_check_root_path(data, path, &output_path))
        return false;

	ostringstream str;
	str << output_path.array << data;
	output = str.str();
	
	return os_file_exists(output.c_str());
}
#endif



#endif