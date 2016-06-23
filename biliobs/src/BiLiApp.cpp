#include "BiLiApp.h"
#include "ShadowDlg.h"
#include "BiLiOBSMainWid.h"
#include <QFile>
#include <QTranslator>
#include "../common/bili-stdmutex.h"

#include "BiliGlobalDatas.hpp"
#include "BiliOBSUtility.hpp"
#include "HotkeyManager.h"

#include "BiliApiManager.h"
#include "../biliapi/IBiliApi.h"

#define DEFAULT_LANG "en-US"
#define MAX_REPEATED_LINES 30
#define MAX_CHAR_VARIATION (255 * 3)

#ifdef __APPLE__
#define INPUT_AUDIO_SOURCE  "coreaudio_input_capture"
#define OUTPUT_AUDIO_SOURCE "coreaudio_output_capture"
#elif _WIN32
#define INPUT_AUDIO_SOURCE  "wasapi_input_capture"
#define OUTPUT_AUDIO_SOURCE "wasapi_output_capture"
#else
#define INPUT_AUDIO_SOURCE  "pulse_input_capture"
#define OUTPUT_AUDIO_SOURCE "pulse_output_capture"
#endif

//////////////////////////// OBSBasic Static Method ////////////////////////////
static string currentLogFile;
static string lastLogFile;
static const char *run_bili_obs_program_init = "run_bili_obs_program_init";


static void move_basic_to_profiles(void) {

	char path[512];
	char new_path[512];
	os_glob_t *glob;

	QString name = QString("%1\\%2").arg(QString::fromStdString(gBili_mid));
	/* if not first time use */
	if (GetUserDataPath(path, 512, name.arg("basic").toUtf8().data()) <= 0)
		return;
	if (!os_file_exists(path))
		return;

	/* if the profiles directory doesn't already exist */
	if (GetUserDataPath(new_path, 512, name.arg("profiles").toUtf8().data()) <= 0)
		return;
	if (os_file_exists(new_path))
		return;

	if (os_mkdir(new_path) == MKDIR_ERROR)
		return;

	strcat(new_path, "/");
	strcat(new_path, Str("Untitled"));
	if (os_mkdir(new_path) == MKDIR_ERROR)
		return;

	strcat(path, "/*.*");
	if (os_glob(path, 0, &glob) != 0)
		return;

	strcpy(path, new_path);

	for (size_t i = 0; i < glob->gl_pathc; i++) {
		struct os_globent ent = glob->gl_pathv[i];
		char *file;

		if (ent.directory)
			continue;

		file = strrchr(ent.path, '/');
		if (!file++)
			continue;

		if (astrcmpi(file, "scenes.json") == 0)
			continue;

		strcpy(new_path, path);
		strcat(new_path, "/");
		strcat(new_path, file);
		os_rename(ent.path, new_path);
	}

	os_globfree(glob);
}

static void move_basic_to_scene_collections(void) {

	char path[512];
	char new_path[512];

	QString name = QString("%1\\%2").arg(QString::fromStdString(gBili_mid));
	if (GetUserDataPath(path, 512, name.arg("basic").toUtf8().data() ) <= 0)
		return;
	if (!os_file_exists(path))
		return;

	if (GetUserDataPath(new_path, 512, name.arg("basic\\scenes").toUtf8().data()) <= 0)
		return;
	if (os_file_exists(new_path))
		return;

	if (os_mkdir(new_path) == MKDIR_ERROR)
		return;

	strcat(path, "/scenes.json");
	strcat(new_path, "/");
	strcat(new_path, Str("Untitled"));
	strcat(new_path, ".json");

	os_rename(path, new_path);
}

static bool MakeBiliCommonDirs(){

	char path[512] = { 0 };

	if (GetUserDataPath(path, sizeof(path), "bilibili") <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

	if (GetUserDataPath(path, sizeof(path), "bilibili/log") <= 0)
		return false;
	if (!do_mkdir(path))
		return false;
	return true;
}

static bool MakeUserDirs() {

	char path[512];

	QString name = QString("%1\\%2").arg(QString::fromStdString(gBili_mid));
	if (GetUserDataPath(path, sizeof(path), name.arg("basic").toUtf8().data()) <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

#ifdef _WIN32
	if (GetUserDataPath(path, sizeof(path), "common/crashes") <= 0)
		return false;
	if (!do_mkdir(path))
		return false;
#endif

	if (GetUserDataPath(path, sizeof(path), "common/logs") <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

	if (GetUserDataPath(path, sizeof(path), "common/profiler_data") <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

	if (GetUserDataPath(path, sizeof(path), "common/plugin_config") <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

	move_basic_to_profiles();
	move_basic_to_scene_collections();

	return true;
}

static bool MakeUserProfileDirs() {

	char path[512];

	QString name = QString("%1\\%2").arg(QString::fromStdString(gBili_mid));
	if (GetUserDataPath(path, sizeof(path), name.arg("basic\\profiles").toUtf8().data()) <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

	if (GetUserDataPath(path, sizeof(path), name.arg("basic\\scenes").toUtf8().data() ) <= 0)
		return false;
	if (!do_mkdir(path))
		return false;

	return true;
}

static bool get_token(lexer *lex, string &str, base_token_type type) {

	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	str.assign(token.text.array, token.text.len);
	return true;
}

static bool expect_token(lexer *lex, const char *str, base_token_type type) {

	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	return strref_cmp(&token.text, str) == 0;
}

static uint64_t convert_log_name(const char *name) {

	BaseLexer  lex;
	string     year, month, day, hour, minute, second;

	lexer_start(lex, name);

	if (!get_token(lex, year,   BASETOKEN_DIGIT)) return 0;
	if (!expect_token(lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(lex, month,  BASETOKEN_DIGIT)) return 0;
	if (!expect_token(lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(lex, day,    BASETOKEN_DIGIT)) return 0;
	if (!get_token(lex, hour,   BASETOKEN_DIGIT)) return 0;
	if (!expect_token(lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(lex, minute, BASETOKEN_DIGIT)) return 0;
	if (!expect_token(lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(lex, second, BASETOKEN_DIGIT)) return 0;

	stringstream timestring;
	timestring << year << month << day << hour << minute << second;
	return std::stoull(timestring.str());
}

static void get_last_log(void) {
	BPtr<char>       logDir(GetConfigPathPtr("common/logs"));
	struct os_dirent *entry;
	os_dir_t         *dir        = os_opendir(logDir);
	uint64_t         highest_ts = 0;

	if (dir) {
		while ((entry = os_readdir(dir)) != NULL) {
			if (entry->directory || *entry->d_name == '.')
				continue;

			uint64_t ts = convert_log_name(entry->d_name);

			if (ts > highest_ts) {
				lastLogFile = entry->d_name;
				highest_ts  = ts;
			}
		}

		os_closedir(dir);
	}
}

static inline int sum_chars(const char *str) {

	int val = 0;
	for (; *str != 0; str++)
		val += *str;

	return val;
}

static inline bool too_many_repeated_entries(fstream &logFile, const char *msg,
		const char *output_str) {
	return true;

	static bili::mutex log_mutex;
	static const char *last_msg_ptr = nullptr;
	static int last_char_sum = 0;
	static char cmp_str[4096];
	static int rep_count = 0;

	int new_sum = sum_chars(output_str);

	bili::lock_guard<bili::mutex> guard(log_mutex);

	if (last_msg_ptr == msg) {
		int diff = std::abs(new_sum - last_char_sum);
		if (diff < MAX_CHAR_VARIATION) {
			return (rep_count++ >= MAX_REPEATED_LINES);
		}
	}

	if (rep_count > MAX_REPEATED_LINES) {
		logFile << CurrentTimeString() <<
			": Last log entry repeated for " <<
			to_string(rep_count - MAX_REPEATED_LINES) <<
			" more lines" << endl;
	}

	last_msg_ptr = msg;
	strcpy(cmp_str, output_str);
	last_char_sum = new_sum;
	rep_count = 0;

	return false;
}

static inline void LogString(fstream &logFile, const char *timeString,
		char *str) {

	//logFile << timeString << str << endl;
}

static inline void LogStringChunk(fstream &logFile, char *str) {

	char *nextLine = str;
	string timeString = CurrentTimeString();
	timeString += ": ";

	while (*nextLine) {
		char *nextLine = strchr(str, '\n');
		if (!nextLine)
			break;

		if (nextLine != str && nextLine[-1] == '\r') {
			nextLine[-1] = 0;
		} else {
			nextLine[0] = 0;
		}

		LogString(logFile, timeString.c_str(), str);
		nextLine++;
		str = nextLine;
	}

	LogString(logFile, timeString.c_str(), str);
}

static void do_log(int log_level, const char *msg, va_list args, void *param) {

	fstream &logFile = *static_cast<fstream*>(param);
	char str[4096];

#ifndef _WIN32
	va_list args2;
	va_copy(args2, args);
#endif

	vsnprintf(str, 4095, msg, args);

#ifdef _WIN32
	//OutputDebugStringA(str);
	//OutputDebugStringA("\n");
	fprintf(stdout, "%s%c", str, '\n');
#else
	def_log_handler(log_level, msg, args2, nullptr);
#endif

	if (too_many_repeated_entries(logFile, msg, str))
		return;

	if (log_level <= LOG_INFO)
		LogStringChunk(logFile, str);

#ifdef _WIN32
	if (log_level <= LOG_ERROR && IsDebuggerPresent())
		__debugbreak();
#endif
}

static void delete_oldest_file(const char *location) {

	BPtr<char>       logDir(GetConfigPathPtr(location));
	string           oldestLog;
	uint64_t         oldest_ts = (uint64_t)-1;
	struct os_dirent *entry;

	unsigned int maxLogs = (unsigned int)config_get_uint(
			App()->mGetGlobalConfig(), "General", "MaxLogs");

	os_dir_t *dir = os_opendir(logDir);
	if (dir) {
		unsigned int count = 0;

		while ((entry = os_readdir(dir)) != NULL) {
			if (entry->directory || *entry->d_name == '.')
				continue;

			uint64_t ts = convert_log_name(entry->d_name);

			if (ts) {
				if (ts < oldest_ts) {
					oldestLog = entry->d_name;
					oldest_ts = ts;
				}

				count++;
			}
		}

		os_closedir(dir);

		if (count > maxLogs) {
			stringstream delPath;

			delPath << logDir << "/" << oldestLog;
			os_unlink(delPath.str().c_str());
		}
	}
}

static void create_log_file(fstream &logFile) {

	stringstream dst;

	get_last_log();

	currentLogFile = GenerateTimeDateFilename("txt");
	dst << "common/logs/" << currentLogFile.c_str();

	BPtr<char> path(GetConfigPathPtr(dst.str().c_str()));
	logFile.open(path, ios_base::in | ios_base::out | ios_base::trunc);

	if (logFile.is_open()) {
		delete_oldest_file("common/logs");
		base_set_log_handler(do_log, &logFile);
	} else {
		blog(LOG_ERROR, "Failed to open log file");
	}
}

static bool StartupOBS(const char *locale, profiler_name_store_t *store) {

	char path[512];

	if (GetUserDataPath(path, sizeof(path), "common/plugin_config") <= 0)
		return false;

	return obs_startup(locale, path, store);
}

static auto ProfilerNameStoreRelease = [](profiler_name_store_t *store) {

	profiler_name_store_free(store);
};

using ProfilerNameStore =
	std::unique_ptr<profiler_name_store_t,
			decltype(ProfilerNameStoreRelease)>;

ProfilerNameStore CreateNameStore() {

	return ProfilerNameStore{profiler_name_store_create(),
					ProfilerNameStoreRelease};
}

static auto SnapshotRelease = [](profiler_snapshot_t *snap) {

	profile_snapshot_free(snap);
};

using ProfilerSnapshot = 
	std::unique_ptr<profiler_snapshot_t, decltype(SnapshotRelease)>;

ProfilerSnapshot GetSnapshot() {

	return ProfilerSnapshot{profile_snapshot_create(), SnapshotRelease};
}

static void SaveProfilerData(const ProfilerSnapshot &snap) {

	if (currentLogFile.empty())
		return;

	auto pos = currentLogFile.rfind('.');
	if (pos == currentLogFile.npos)
		return;

#define LITERAL_SIZE(x) x, (sizeof(x) - 1)
	ostringstream dst;
	dst.write(LITERAL_SIZE("common/profiler_data/"));
	dst.write(currentLogFile.c_str(), pos);
	dst.write(LITERAL_SIZE(".csv.gz"));
#undef LITERAL_SIZE

	BPtr<char> path = GetConfigPathPtr(dst.str().c_str());
	if (!profiler_snapshot_dump_csv_gz(snap.get(), path))
		blog(LOG_WARNING, "Could not save profiler data to '%s'",
				static_cast<const char*>(path));
}

static auto ProfilerFree = [](void *) {

	profiler_stop();

	auto snap = GetSnapshot();

	profiler_print(snap.get());
	profiler_print_time_between_calls(snap.get());

	SaveProfilerData(snap);

	profiler_free();
};

////////////////////////////////////////////////////////////////////


BiLiApp::BiLiApp(int &argc, char **argv)
	: QApplication(argc, argv) {
}

BiLiApp::~BiLiApp() { }

bool BiLiApp::mInitApp() {

	QCoreApplication::addLibraryPath(".");

	try {

		//create dirs before login
		if (!MakeBiliCommonDirs())
			throw "Failed to create common directories";

		IBiliAPI* biliApi = BiliAPIMan::GetInstance();

		HotkeyManager::InitializeInstance();
		EnableMuteHotkeySupport();

		if (!mOBSInitGlobalConfig())
			throw "Failed to initialize global config";
		if (!MakeUserDirs())
			throw "Failed to make user dirs";
		if (!MakeUserProfileDirs())
			throw "Failed to make user profile dirs";

		fstream logFile;
		create_log_file(logFile);
		delete_oldest_file("common/profiler_data");

		if (!mOBSInit())
			return false;
		mBiLiOBSMainWid = new BiLiOBSMainWid(biliApi);	//create main widget and set it's shadow wid
		mBiLiOBSMainWid->setAttribute(Qt::WA_DeleteOnClose);
		mBiLiOBSMainWid->mOBSInit();
		mBiLiOBSMainWid->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	} catch (const char *error) {
		blog(LOG_ERROR, "%s", error);
		OBSErrorBox(nullptr, "%s", error);

		return false;
	}
	mBiLiOBSMainWid->mShow();
	return true;
}

bool BiLiApp::mOBSInit() {

	ProfileScope("BiLiApp::mOBSInit");

	bool licenseAccepted = config_get_bool(mGlobalConfig, "General", "LicenseAccepted");
	if (!licenseAccepted) {
		config_set_bool(mGlobalConfig, "General", "LicenseAccepted", true);
		config_save(mGlobalConfig);
	}

	if (!StartupOBS(mLocale.c_str(), mGetProfilerNameStore()))
		return false;

	return true;
}
///////////////////////////////////////////////////////////////////////////

const char *BiLiApp::mInputAudioSource() const {
	return INPUT_AUDIO_SOURCE;
}

const char *BiLiApp::mOutputAudioSource() const {
	return OUTPUT_AUDIO_SOURCE;
}

const char *BiLiApp::mGetRenderModule() const {

	const char *renderer = config_get_string(mGlobalConfig, "Video", "Renderer");

	return (astrcmpi(renderer, "Direct3D 11") == 0) ?
		DL_D3D11 : DL_OPENGL;
}

bool BiLiApp::mOBSInitGlobalConfig() {

	char path[512];
	int len = GetUserDataPath(path, sizeof(path), "global.ini");
	if (len <= 0) 
		return false;

	int errorcode = mGlobalConfig.Open(path, CONFIG_OPEN_ALWAYS);
	if (errorcode != CONFIG_SUCCESS) {
		OBSErrorBox(NULL, "Failed to open global.ini: %d", errorcode);
		return false;
	}

	return mOBSInitGlobalConfigDefaults();
}

bool BiLiApp::mOBSInitGlobalConfigDefaults() {

	config_set_default_string(mGlobalConfig, "General", "Language", DEFAULT_LANG);
	const char *lang = config_get_string(mGlobalConfig, "General", "Language");
	mLocale = lang;
	config_set_default_uint(mGlobalConfig, "General", "MaxLogs", 10);

#if _WIN32
	config_set_default_string(mGlobalConfig, "Video", "Renderer", "Direct3D 11");
#else
	config_set_default_string(mGlobalConfig, "Video", "Renderer", "OpenGL");
#endif

	config_set_default_bool(mGlobalConfig, "BasicWindow", "PreviewEnabled", true);

	config_set_default_string(mGlobalConfig, "Basic", "Profile", Str("Untitled"));
	config_set_default_string(mGlobalConfig, "Basic", "ProfileDir", Str("Untitled"));
	config_set_default_string(mGlobalConfig, "Basic", "SceneCollection", Str("Untitled"));
	config_set_default_string(mGlobalConfig, "Basic", "SceneCollectionFile", Str("Untitled"));

	return true;
}

BiLiOBSMainWid* BiLiApp::mGetMainWindow() const
{
	return mBiLiOBSMainWid;
}

void BiLiApp::mLoadTranslator()
{
	mTrans.reset(new QTranslator());
	mTrans->load(":/Trans/BiLi_zh");
	qApp->installTranslator(mTrans.get());
}
