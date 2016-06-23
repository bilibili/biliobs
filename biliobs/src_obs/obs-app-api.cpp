#include "obs-app-api.hpp"
#include <chrono>
#include "common/biliobs_paths.h"
#ifdef __APPLE__
#define BASE_PATH ".."
#else
#define BASE_PATH "../.."
#endif

#define CONFIG_PATH BASE_PATH "/config"

#ifndef OBS_UNIX_STRUCTURE
#define OBS_UNIX_STRUCTURE 0
#endif

static bool portable_mode = false;

int GetUserDataPath(char *path, size_t size, const char *name)
{
    std::wstring wsPath = biliobs::GetUserDataPath();
    os_wcs_to_utf8(wsPath.c_str(), wsPath.length(), path, size);
    errno_t err;
    err = strcat_s(path, size, name);
    if (err != 0)
        return -1;

	return strlen(path);
}

bool do_mkdir(const char *path)
{
	if (os_mkdirs(path) == MKDIR_ERROR) {
		OBSErrorBox(NULL, "Failed to create directory %s", path);
		return false;
	}

	return true;
}

char *GetConfigPathPtr(const char *name)
{
	if (!OBS_UNIX_STRUCTURE && portable_mode) {
		char path[512];

		if (snprintf(path, sizeof(path), CONFIG_PATH "/%s", name) > 0) {
			return bstrdup(path);
		} else {
			return NULL;
		}
	} else {
		char path[512];
		if (GetUserDataPath(path, sizeof(path), name) <= 0)
			return NULL;
		return bstrdup(path);
		//return os_get_config_path_ptr(name);
	}
}
string GenerateTimeDateFilename(const char *extension)
{
	time_t    now = time(0);
	char      file[256] = {};
	struct tm *cur_time;

	cur_time = localtime(&now);
	snprintf(file, sizeof(file), "%d-%02d-%02d %02d-%02d-%02d.%s",
			cur_time->tm_year+1900,
			cur_time->tm_mon+1,
			cur_time->tm_mday,
			cur_time->tm_hour,
			cur_time->tm_min,
			cur_time->tm_sec,
			extension);

	return string(file);
}

string CurrentTimeString()
{
	using namespace std::chrono;

	struct tm  tstruct;
	char       buf[80];

	auto tp = system_clock::now();
	auto now = system_clock::to_time_t(tp);
	tstruct = *localtime(&now);

	size_t written = strftime(buf, sizeof(buf), "%X", &tstruct);
	if (ratio_less<system_clock::period, seconds::period>::value &&
			written && (sizeof(buf) - written) > 5) {
		auto tp_secs =
			time_point_cast<seconds>(tp);
		auto millis  =
			duration_cast<milliseconds>(tp - tp_secs).count();

		snprintf(buf + written, sizeof(buf) - written, ".%03u",
				static_cast<unsigned>(millis));
	}

	return buf;
}

vector<pair<string, string>> GetLocaleNames() {

	string path;
	if (!GetDataFilePath("locale.ini", path))
		throw "Could not find locale.ini path";

	ConfigFile ini;
	if (ini.Open(path.c_str(), CONFIG_OPEN_EXISTING) != 0)
		throw "Could not open locale.ini";

	size_t sections = config_num_sections(ini);

	vector<pair<string, string>> names;
	names.reserve(sections);
	for (size_t i = 0; i < sections; i++) {
		const char *tag = config_get_section(ini, i);
		const char *name = config_get_string(ini, tag, "Name");
		names.emplace_back(tag, name);
	}

	return names;
}