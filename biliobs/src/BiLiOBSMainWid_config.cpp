#include "BiLiOBSMainWid.h"
#include "BiLiApp.h"
#include "display-helpers.hpp"
#include "BiliUIConfigSync.hpp"
#include "BiliGlobalDatas.hpp"

#include <QDesktopWidget>

#define DEFAULT_OUTPUT_CX 1280
#define DEFAULT_OUTPUT_CY 720


static const double scaled_vals[] = {
	1.0,
	1.25,
	(1.0/0.75),
	1.5,
	(1.0/0.6),
	1.75,
	2.0,
	2.25,
	2.5,
	2.75,
	3.0,
	0.0
};


int BiLiOBSMainWid::mGetProfilePath(char *path, size_t size, const char *file) const {

	char profiles_path[512];
	const char *profile = config_get_string(App()->mGetGlobalConfig(),
			"Basic", "ProfileDir");
	int ret;

	if (!profile)
		return -1;
	if (!path)
		return -1;
	if (!file)
		file = "";

	ret = GetUserDataPath(profiles_path, 512, QString("%1\\%2").arg(QString::fromStdString(gBili_mid)).arg("basic/profiles/").toUtf8().data() );
	if (ret <= 0)
		return ret;

	if (!*file)
		return snprintf(path, size, "%s/%s", profiles_path, profile);

	return snprintf(path, size, "%s/%s/%s", profiles_path, profile, file);
}


bool BiLiOBSMainWid::mInitBasicConfigDefaults() {

	vector<MonitorInfo> monitors;
	GetMonitors(monitors);

	if (!monitors.size()) {
		OBSErrorBox(NULL, "There appears to be no monitors.  Er, this "
		                  "technically shouldn't be possible.");
		return false;
	}

	uint32_t cx = monitors[0].cx;
	uint32_t cy = monitors[0].cy;

	/* ----------------------------------------------------- */
	/* move over mixer values in advanced if older config */
	if (config_has_user_value(mBasicConfig, "AdvOut", "RecTrackIndex") &&
	    !config_has_user_value(mBasicConfig, "AdvOut", "RecTracks")) {

		uint64_t track = config_get_uint(mBasicConfig, "AdvOut",
				"RecTrackIndex");
		track = 1ULL << (track - 1);
		config_set_uint(mBasicConfig, "AdvOut", "RecTracks", track);
		config_remove_value(mBasicConfig, "AdvOut", "RecTrackIndex");
		config_save_safe(mBasicConfig, "tmp", nullptr);
	}

	/* ----------------------------------------------------- */

	config_set_default_string(mBasicConfig, "Output", "Mode", "Simple");

	config_set_default_string(mBasicConfig, "SimpleOutput", "FilePath",
			GetDefaultVideoSavePath().c_str());
	config_set_default_string(mBasicConfig, "SimpleOutput", "RecFormat",
			"flv");
	config_set_default_uint  (mBasicConfig, "SimpleOutput", "VBitrate",
			1200);
	config_set_default_uint  (mBasicConfig, "SimpleOutput", "ABitrate", 80);
	config_set_default_bool  (mBasicConfig, "SimpleOutput", "UseAdvanced",
			true);
	config_set_default_string(mBasicConfig, "SimpleOutput", "Preset",
			"veryfast");
	config_set_default_string(mBasicConfig, "SimpleOutput", "RecQuality",
			"Stream");
	config_set_default_string(mBasicConfig, "SimpleOutput", "RecEncoder",
			SIMPLE_ENCODER_X264);

	config_set_default_bool  (mBasicConfig, "AdvOut", "ApplyServiceSettings",
			true);
	config_set_default_bool  (mBasicConfig, "AdvOut", "UseRescale", false);
	config_set_default_uint  (mBasicConfig, "AdvOut", "TrackIndex", 1);
	config_set_default_string(mBasicConfig, "AdvOut", "Encoder", "obs_x264");

	config_set_default_string(mBasicConfig, "AdvOut", "x264Settings", "qpmin=25");

	config_set_default_string(mBasicConfig, "AdvOut", "RecType", "Standard");

	config_set_default_string(mBasicConfig, "AdvOut", "RecFilePath",
			GetDefaultVideoSavePath().c_str());
	config_set_default_string(mBasicConfig, "AdvOut", "RecFormat", "flv");
	config_set_default_bool  (mBasicConfig, "AdvOut", "RecUseRescale",
			false);
	config_set_default_uint  (mBasicConfig, "AdvOut", "RecTracks", (1<<0));
	config_set_default_string(mBasicConfig, "AdvOut", "RecEncoder",
			"none");

	config_set_default_bool  (mBasicConfig, "AdvOut", "FFOutputToFile",
			true);
	config_set_default_string(mBasicConfig, "AdvOut", "FFFilePath",
			GetDefaultVideoSavePath().c_str());
	config_set_default_string(mBasicConfig, "AdvOut", "FFExtension", "mp4");
	config_set_default_uint  (mBasicConfig, "AdvOut", "FFVBitrate", 2500);
	config_set_default_bool  (mBasicConfig, "AdvOut", "FFUseRescale",
			false);
	config_set_default_uint  (mBasicConfig, "AdvOut", "FFABitrate", 160);
	config_set_default_uint  (mBasicConfig, "AdvOut", "FFAudioTrack", 1);

	config_set_default_uint  (mBasicConfig, "AdvOut", "Track1Bitrate", 160);
	config_set_default_uint  (mBasicConfig, "AdvOut", "Track2Bitrate", 160);
	config_set_default_uint  (mBasicConfig, "AdvOut", "Track3Bitrate", 160);
	config_set_default_uint  (mBasicConfig, "AdvOut", "Track4Bitrate", 160);

	config_set_default_string(mBasicConfig, "AdvOut", "PushStreamServer", "");
	config_set_default_string(mBasicConfig, "AdvOut", "PushStreamPath", "");

	config_set_default_string(mBasicConfig, "AdvOut", "PushServerType", "pushToBiliRadio");


	config_set_default_bool  (mBasicConfig, "Output", "DelayEnable", false);
	config_set_default_uint  (mBasicConfig, "Output", "DelaySec", 20);
	config_set_default_bool  (mBasicConfig, "Output", "DelayPreserve", true);

	config_set_default_bool  (mBasicConfig, "Output", "Reconnect", true);
	config_set_default_uint  (mBasicConfig, "Output", "RetryDelay", 5);
	config_set_default_uint  (mBasicConfig, "Output", "MaxRetries", 10000);

	int i = 0;
	uint32_t scale_cx = cx;
	uint32_t scale_cy = cy;

#if 0
	/* use a default scaled resolution that has a pixel count no higher
	 * than 1280x720 */
	while (((scale_cx * scale_cy) > (1280 * 720)) && scaled_vals[i] > 0.0) {
		double scale = scaled_vals[i++];
		scale_cx = uint32_t(double(cx) / scale);
		scale_cy = uint32_t(double(cy) / scale);
	}
#endif
	//config_set_default_uint  (mBasicConfig, "Video", "BaseCX",   cx);
	//config_set_default_uint  (mBasicConfig, "Video", "BaseCY",   cy);
	config_set_default_uint(mBasicConfig, "Video", "BaseCX", DEFAULT_OUTPUT_CX);
	config_set_default_uint(mBasicConfig, "Video", "BaseCY", DEFAULT_OUTPUT_CY);

	config_set_default_uint(mBasicConfig, "Video", "OutputCX", DEFAULT_OUTPUT_CX);
	config_set_default_uint(mBasicConfig, "Video", "OutputCY", DEFAULT_OUTPUT_CY);
	config_set_default_uint(mBasicConfig, "Video", "PrevOutputCX", DEFAULT_OUTPUT_CX);
	config_set_default_uint(mBasicConfig, "Video", "PrevOutputCY", DEFAULT_OUTPUT_CY);

	QDesktopWidget *dk = QApplication::desktop();
	int availableH = dk->availableGeometry(dk->primaryScreen()).height();
	int outH = DEFAULT_OUTPUT_CY;
	int outW = DEFAULT_OUTPUT_CX;
	if ((availableH - DEFAULT_OUTPUT_CY) < 50) {
		outW = 712;
		outH = 400;
	}

	config_set_default_uint(mBasicConfig, "Video", "ViewX", outW);
	config_set_default_uint(mBasicConfig, "Video", "ViewY", outH);
	config_set_default_uint(mBasicConfig, "Video", "PrevViewX", outW);
	config_set_default_uint(mBasicConfig, "Video", "PrevViewY", outH);

	config_set_default_uint  (mBasicConfig, "Video", "FPSType", 0);
	config_set_default_string(mBasicConfig, "Video", "FPSCommon", "30");
	config_set_default_uint  (mBasicConfig, "Video", "FPSInt", 30);
	config_set_default_uint  (mBasicConfig, "Video", "FPSNum", 30);
	config_set_default_uint  (mBasicConfig, "Video", "FPSDen", 1);
	config_set_default_string(mBasicConfig, "Video", "ScaleType", "bicubic");
	config_set_default_string(mBasicConfig, "Video", "ColorFormat", "NV12");
	config_set_default_string(mBasicConfig, "Video", "ColorSpace", "601");
	config_set_default_string(mBasicConfig, "Video", "ColorRange",
			"Partial");

	config_set_default_uint  (mBasicConfig, "Audio", "SampleRate", 44100);
	config_set_default_string(mBasicConfig, "Audio", "ChannelSetup",
			"Stereo");
	config_set_default_uint  (mBasicConfig, "Audio", "BufferingTime", 1000);

    //danmaku history
    config_set_default_int(mBasicConfig, "DanmakuHistory", "DanmakuRefreshMode", 0);
    config_set_default_int(mBasicConfig, "DanmakuHistory", "DanmakuRefreshInterval", 10);

	return true;
}



bool BiLiOBSMainWid::mInitBasicConfig() {

	ProfileScope("BiLiOBSMainWid::mInitBasicConfig");

	char configPath[512];

	int ret = mGetProfilePath(configPath, sizeof(configPath), "");
	if (ret <= 0) {
		OBSErrorBox(nullptr, "Failed to get profile path");
		return false;
	}

	if (os_mkdir(configPath) == MKDIR_ERROR) {
		OBSErrorBox(nullptr, "Failed to create profile path");
		return false;
	}

	ret = mGetProfilePath(configPath, sizeof(configPath), "basic.ini");
	if (ret <= 0) {
		OBSErrorBox(nullptr, "Failed to get base.ini path");
		return false;
	}

	int code = mBasicConfig.Open(configPath, CONFIG_OPEN_ALWAYS);
	if (code != CONFIG_SUCCESS) {
		OBSErrorBox(NULL, "Failed to open basic.ini: %d", code);
		return false;
	}
	dmOpt_->loadSettings_(qVPtr<config_t>::toVariant(mBasicConfig));

	if (config_get_string(mBasicConfig, "General", "Name") == nullptr) {
		const char *curName = config_get_string(App()->mGetGlobalConfig(),
				"Basic", "Profile");

		config_set_string(mBasicConfig, "General", "Name", curName);
		mBasicConfig.SaveSafe("tmp");
	}

	return mInitBasicConfigDefaults();
}


void BiLiOBSMainWid::mGetFPSInteger(uint32_t &num, uint32_t &den) const {

	num = (uint32_t)config_get_uint(mBasicConfig, "Video", "FPSInt");
	den = 1;
}

void BiLiOBSMainWid::mGetFPSFraction(uint32_t &num, uint32_t &den) const {

	num = (uint32_t)config_get_uint(mBasicConfig, "Video", "FPSNum");
	den = (uint32_t)config_get_uint(mBasicConfig, "Video", "FPSDen");
}

void BiLiOBSMainWid::mGetFPSNanoseconds(uint32_t &num, uint32_t &den) const {

	num = 1000000000;
	den = (uint32_t)config_get_uint(mBasicConfig, "Video", "FPSNS");
}

void BiLiOBSMainWid::mGetFPSCommon(uint32_t &num, uint32_t &den) const {

	const char *val = config_get_string(mBasicConfig, "Video", "FPSCommon");

	if (strcmp(val, "10") == 0) {
		num = 10;
		den = 1;
	} else if (strcmp(val, "20") == 0) {
		num = 20;
		den = 1;
	} else if (strcmp(val, "25") == 0) {
		num = 25;
		den = 1;
	} else if (strcmp(val, "29.97") == 0) {
		num = 30000;
		den = 1001;
	} else if (strcmp(val, "48") == 0) {
		num = 48;
		den = 1;
	} else if (strcmp(val, "59.94") == 0) {
		num = 60000;
		den = 1001;
	} else if (strcmp(val, "60") == 0) {
		num = 60;
		den = 1;
	} else {
		num = 30;
		den = 1;
	}
}

void BiLiOBSMainWid::mGetConfigFPS(uint32_t &num, uint32_t &den) const {

    den = 1;
    
    if (config_has_user_value(mBasicConfig, "Video", "FPSInt")) {
        num = config_get_uint(mBasicConfig, "Video", "FPSInt");

    } else {
        num = config_get_default_uint(mBasicConfig, "Video", "FPSInt");
    }

    return;
}
