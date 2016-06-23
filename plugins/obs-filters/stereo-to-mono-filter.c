#include <obs-module.h>
#include <media-io/audio-math.h>
#include <math.h>

#include <intrin.h>

#define do_log(level, format, ...) \
	blog(level, "[forcemono filter: '%s'] " format, \
			obs_source_get_name(gf->context), ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)

enum { MODE_NOTOUCH = 0, MODE_LEFT, MODE_RIGHT, MODE_MIX };

struct forcemono_data {
	obs_source_t *context;

	int mode;
};

static const char *forcemono_name(void *unused)
{
	return obs_module_text("Force mono");
}

static void forcemono_destroy(void *data)
{
	struct forcemono_data *fd = data;
	bfree(fd);
}

static void forcemono_update(void *data, obs_data_t *s)
{
	struct forcemono_data *fd = data;

	if (s)
	{
		const char* mode = obs_data_get_string(s, "mode");
		if (mode)
		{
			if (strcmp(mode, "forcemono_notouch") == 0)
				fd->mode = MODE_NOTOUCH;
			else if (strcmp(mode, "forcemono_leftonly") == 0)
				fd->mode = MODE_LEFT;
			else if (strcmp(mode, "forcemono_rightonly") == 0)
				fd->mode = MODE_RIGHT;
			else if (strcmp(mode, "forcemono_mix") == 0)
				fd->mode = MODE_MIX;
			else
				blog(LOG_WARNING, "[stereo to mono] unsupport mode '%s'", mode);
		}
	}
}

static void *forcemono_create(obs_data_t *settings, obs_source_t *filter)
{
	struct forcemono_data *fd = bzalloc(sizeof(*fd));
	fd->context = filter;
	forcemono_update(fd, settings);
	return fd;
}

inline static mixdown(float* lch, float* rch)
{
	float x;
	if (*lch < 0 && *rch < 0)
		x = *lch + *rch + *lch * *rch;
	else
		x = *lch + *rch - *lch * *rch;
	*lch = x;
	*rch = x;
}

static struct obs_audio_data *forcemono_filter_audio(void *data, struct obs_audio_data *audio)
{
	struct forcemono_data *fd = data;

	float *adata[2] = { (float*)audio->data[0], (float*)audio->data[1] };

	if (adata[0] && adata[1])
	{
		if (fd->mode == MODE_LEFT)
		{
			memcpy(adata[1], adata[0], audio->frames * sizeof(float));
		}
		else if (fd->mode == MODE_RIGHT)
		{
			memcpy(adata[0], adata[1], audio->frames * sizeof(float));
		}
		else if (fd->mode == MODE_MIX)
		{
			for (int i = 0; i < audio->frames; ++i)
				mixdown(&adata[0][i], &adata[1][i]);
		}
	}

	return audio;
}

static void forcemono_defaults(obs_data_t *s)
{
	if (s)
	{
		obs_data_set_string(s, "mode", "forcemono_notouch");
	}
}

static obs_properties_t *forcemono_properties(void *data)
{
	obs_properties_t *ppts = obs_properties_create();
	return ppts;
}

struct obs_source_info forcemono_filter = {
	.id = "forcemono_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = forcemono_name,
	.create = forcemono_create,
	.destroy = forcemono_destroy,
	.update = forcemono_update,
	.filter_audio = forcemono_filter_audio,
	.get_defaults = forcemono_defaults,
	.get_properties = forcemono_properties,
};
