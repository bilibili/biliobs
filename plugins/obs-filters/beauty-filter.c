#include <obs-module.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>

struct beauty_filter_data {
	obs_source_t                   *context;

	gs_effect_t                    *effect;
	gs_eparam_t                    *param_beauty_params;
	gs_eparam_t                    *param_step_offset;

	struct vec4                    beauty_params;
	struct vec2                    step_offset;
};

static const char *beauty_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("BeautyFilter");
}

static void *beauty_filter_create(obs_data_t *settings, obs_source_t *context)
{
	struct beauty_filter_data *filter = bzalloc(sizeof(*filter));
	char *effect_path = obs_module_file("beauty_filter.effect");

	filter->context = context;

	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	obs_leave_graphics();

	bfree(effect_path);

	if (!filter->effect) {
		bfree(filter);
		return NULL;
	}

	filter->param_beauty_params = gs_effect_get_param_by_name(filter->effect,
		"beauty_params");
	filter->param_step_offset = gs_effect_get_param_by_name(filter->effect,
		"step_offset");

	obs_source_update(context, settings);
	return filter;
}

static void beauty_filter_destroy(void *data)
{
	struct beauty_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
	obs_leave_graphics();

	bfree(filter);
}

static void beauty_filter_update(void *data, obs_data_t *settings)
{
	struct beauty_filter_data *filter = data;

	float beauty_params[][4] = {
		{ 135, 0.05, 0.02, 0.7 },
		{ 120, 0.08, 0.04, 1.0 },
		{ 85, 0.1, 0.05, 1.4 },
		{ 50, 0.12, 0.06, 1.8 },
		{ 0, 0, 0, 0 }
	};

	int level = obs_data_get_int(settings, "beauty_level") - 1;
	if (level < 0 || level > sizeof(beauty_params) / sizeof(*beauty_params))
		level = 0;

	vec4_set(&filter->beauty_params, beauty_params[level][0],
									 beauty_params[level][1],
									 beauty_params[level][2], 
									 beauty_params[level][3]);
}

static obs_properties_t *beauty_filter_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	return props;
}

static void beauty_filter_defaults(obs_data_t *settings)
{
	obs_data_set_int(settings, "beauty_level", 0);
}

static void beauty_filter_render(void *data, gs_effect_t *effect)
{
	struct beauty_filter_data *filter = data;

	obs_source_t* parent_source = obs_filter_get_parent(filter->context);

	uint32_t base_width = obs_source_get_base_width(parent_source);
	uint32_t base_height = obs_source_get_base_height(parent_source);

	obs_source_process_filter_begin(filter->context, GS_RGBA,
		OBS_ALLOW_DIRECT_RENDERING);

	vec2_set(&filter->step_offset, 1.0 / base_width, 1.0 / base_height);

	gs_effect_set_vec4(filter->param_beauty_params, &filter->beauty_params);
	gs_effect_set_vec2(filter->param_step_offset, &filter->step_offset);

	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

	UNUSED_PARAMETER(effect);
}

struct obs_source_info beauty_filter = {
	.id = "beauty_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = beauty_filter_get_name,
	.create = beauty_filter_create,
	.destroy = beauty_filter_destroy,
	.update = beauty_filter_update,
	.get_properties = beauty_filter_properties,
	.get_defaults = beauty_filter_defaults,
	.video_render = beauty_filter_render,
};
