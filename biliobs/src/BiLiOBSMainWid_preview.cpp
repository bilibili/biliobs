#include "BiLiOBSMainWid.h"
#include "BiLiApp.h"
#include "display-helpers.hpp"
#include "bili_obs_source_helper.h"
#include <QMenu>

#ifdef _WIN32
#define IS_WIN32 1
#else
#define IS_WIN32 0
#endif

#define PREVIEW_EDGE_SIZE 0
#define BORDERSIZE	1

void BiLiOBSMainWid::mInitPrimitives() {

	ProfileScope("BiLiOBSMainWid::mInitPrimitives");

	obs_enter_graphics();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(0.0f, 0.0f);
	mBox = gs_render_save();

	gs_render_start(true);
	for (int i = 0; i <= 360; i += (360/20)) {
		float pos = RAD(float(i));
		gs_vertex2f(cosf(pos), sinf(pos));
	}
	mCircle = gs_render_save();

	obs_leave_graphics();
}

static bool enumSourceCallbackResetSource(void* param, obs_source_t* source) {

	obs_scene_t* scene = obs_scene_from_source(source);
	if (scene){
		for (OBSSceneItem& item : OBSEnumSceneItems(scene)) {
			if (obs_source_get_type(obs_sceneitem_get_source(item)) == OBS_SOURCE_TYPE_INPUT) {

				obs_source_t* selectedSource = obs_sceneitem_get_source(item);
				int sourceWidth = obs_source_get_base_width(selectedSource);
				int sourceHeight = obs_source_get_base_height(selectedSource);
				int outputWidth = obs_source_get_base_width(source);
				int outputHeight = obs_source_get_base_height(source);

				vec2 *vs = static_cast<vec2 *>(param);
				vec2 viewSize = *vs;
				vec2 prevViewSize = *(vs + 1);

				float scale = (float)outputWidth/viewSize.x;
				vec2 prevPos;
				obs_sceneitem_get_pos(item, &prevPos);
				vec2 newPos;
				vec2_set(&newPos, ((prevPos.x*viewSize.x)/prevViewSize.x)*scale, ((prevPos.y*viewSize.y)/prevViewSize.y)*scale);

				float newW = sourceWidth,  newH = sourceHeight;
				if (sourceWidth > viewSize.x) {
					newW = viewSize.x;
					newH = newW / ((float)sourceWidth/sourceHeight);
				}
				else if (sourceHeight > viewSize.y){
					newH = viewSize.y;
					newW = newH * ((float)sourceWidth / sourceHeight);
				}
				else{
					vec2_set(&newPos, (newPos.x*viewSize.x)/prevViewSize.x, (newPos.y*viewSize.y)/prevViewSize.y);
				}

				obs_transform_info itemInfo;
				obs_sceneitem_get_info(item, &itemInfo);
				vec2_set(&itemInfo.pos, newPos.x, newPos.y);
				vec2_set(&itemInfo.scale, scale, scale);
				itemInfo.rot = 0.0f;

				vec2_set(&itemInfo.bounds, newW, newH);

				obs_sceneitem_set_info(item, &itemInfo);

				vec2 boundSize;
				boundSize.x = newW;
				boundSize.y = newH;
				obs_sceneitem_set_bounds(item, &boundSize);
			}
		}
	}

	return true;
}

static void getPreviewSize(struct vec2 *dst, float previewX, float previewY, float outputX, float outputY){

	float previewRatio = (float)previewX / previewY;
	float outputRatio = (float)outputX / outputY;
	int newH, newW;
	if (previewRatio > outputRatio){
		newH = previewY;
		newW = newH * outputRatio;
	}
	else{
		newW = previewX;
		newH = previewX / previewRatio;
	}
	vec2_set(dst, newW, newH);
}

void BiLiOBSMainWid::sltResetSource(){

	uint32_t prevOutputCX = config_get_uint(mBasicConfig, "Video", "PrevOutputCX");
	uint32_t prevOutputCY = config_get_uint(mBasicConfig, "Video", "PrevOutputCY");
	uint32_t outputCX = config_get_uint(mBasicConfig, "Video", "OutputCX");
	uint32_t outputCY = config_get_uint(mBasicConfig, "Video", "OutputCY");
	if (prevOutputCX == outputCX)
		return;

	uint32_t prevPreviewX = config_get_uint(mBasicConfig, "Video", "PrevViewX");
	uint32_t prevPreviewY = config_get_uint(mBasicConfig, "Video", "PrevViewY");
	vec2 prevViewSize;
	getPreviewSize(&prevViewSize, prevPreviewX, prevPreviewY, prevOutputCX, prevOutputCY);

	uint32_t previewX = config_get_uint(mBasicConfig, "Video", "ViewX");
	uint32_t previewY = config_get_uint(mBasicConfig, "Video", "ViewY");
	vec2 viewSize;
	getPreviewSize(&viewSize, previewX, previewY, outputCX, outputCY);

#if 1
	vec2 vs[] = {viewSize, prevViewSize};
	obs_enum_sources(enumSourceCallbackResetSource, &vs);
#else
	float wRatio;
	float prevRatio = float(prevOutputCX) / prevOutputCY;
	float ratio = float(outputCX) / outputCY;
	if (prevRatio > ratio){
		wRatio = float(prevOutputCX) / outputCX;
	}
	else{
		int newH = outputCY;
		int newW = newH*prevRatio;
		wRatio = float(prevOutputCX) / newW;
	}
	obs_enum_sources(enumSourceCallbackResetSource, &wRatio);
#endif

}

void BiLiOBSMainWid::sltResetPreviewWid(){

	uint32_t viewX = config_get_uint(mBasicConfig, "Video", "ViewX");
	uint32_t viewY = config_get_uint(mBasicConfig, "Video", "ViewY");

	int height = viewY + ui.LeftTitleWid->height() + ui.LeftBottomWid->height();
	ui.LeftCenterWid->resize(viewX, viewY);

	if (isRightTabShow_)
		resize(viewX + ui.RightSceneTabWid->width() + BORDERSIZE*2, height + BORDERSIZE*2);
	else
		resize(viewX + BORDERSIZE*2, height + BORDERSIZE*2);
	ui.ZoomBtn->raise();
}

void BiLiOBSMainWid::ResetPreview() {

	struct obs_video_info ovi;
	int ret;

	ovi.graphics_module = App()->mGetRenderModule();

	ovi.base_width = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "BaseCX");
	ovi.base_height = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "BaseCY");
	ovi.output_width = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "OutputCX");
	ovi.output_height = (uint32_t)config_get_uint(mBasicConfig,
		"Video", "OutputCY");

	ovi.output_format = VIDEO_FORMAT_NV12;
	ovi.colorspace = VIDEO_CS_709;
	ovi.range = VIDEO_RANGE_FULL;
	ovi.adapter = 0;
	ovi.gpu_conversion = true;
	ovi.scale_type = OBS_SCALE_LANCZOS;

	mGetConfigFPS(ovi.fps_num, ovi.fps_den);

	//begin resize preview window
	QSize previewAreaSize = ui.LeftCenterWid->size();
	int previewAreaWidth = previewAreaSize.width();
	int previewAreaHeight = previewAreaSize.height();
	double previewAreaRatio = double(previewAreaWidth) / previewAreaHeight;

	int videoWidth = ovi.output_width;
	int videoHeight = ovi.output_height;
	double videoRatio = double(videoWidth) / videoHeight;

	int newPreviewWidth, newPreviewHeight;
	int newPreviewInnerWidth, newPreviewInnerHeight;
	if (videoRatio > previewAreaRatio) //preview video's height should be smaller
	{
		newPreviewWidth = previewAreaWidth;
		newPreviewInnerWidth = newPreviewWidth - PREVIEW_EDGE_SIZE * 2;
		newPreviewHeight = newPreviewWidth / videoRatio;
		newPreviewInnerHeight = newPreviewInnerWidth / videoRatio;
	}
	else //preview vidoe's width should be smaller
	{
		newPreviewHeight = previewAreaHeight;
		newPreviewInnerHeight = newPreviewHeight - PREVIEW_EDGE_SIZE * 2;
		newPreviewWidth = newPreviewHeight * videoRatio;
		newPreviewInnerWidth = newPreviewInnerHeight * videoRatio;
	}
	//end resize preview window


	//save preview information to this object
	sourceWidth = ovi.base_width;
	sourceHeight = ovi.base_height;

	mPreviewScale = (newPreviewInnerWidth) / float(sourceWidth);
	mPreviewX = (newPreviewWidth - newPreviewInnerWidth) / 2;
	mPreviewY = (newPreviewHeight - newPreviewInnerHeight) / 2;

	obsPreview->resize(newPreviewWidth, newPreviewHeight);
	obsPreview->move((previewAreaWidth - newPreviewWidth) / 2, (previewAreaHeight - newPreviewHeight) / 2);
	obsPreview->show();


	obs_reset_video(&ovi);

}

static void OnDisplayCreatedCallback(OBSQTDisplay* preview, void (*callback)(void* data, uint32_t cx, uint32_t cy), void* param)
{
	obs_display_add_draw_callback(preview->GetDisplay(),
		callback, param);
}

void BiLiOBSMainWid::InitPreview()
{
	ui.LeftCenterWid->setAttribute(Qt::WA_TranslucentBackground);
	obsPreview = new OBSBasicPreview(ui.LeftCenterWid);
	obsPreview->hide();
	ui.ZoomBtn->raise();

	QObject::connect(obsPreview, &OBSQTDisplay::DisplayCreated, std::bind(&OnDisplayCreatedCallback, std::placeholders::_1, &BiLiOBSMainWid::RenderPreview, this));

	//预览窗口的信号
	QObject::connect(obsPreview, SIGNAL(ContextMenuSignal(QPoint)), this, SLOT(OnPreviewContextMenu(QPoint)));
}

void BiLiOBSMainWid::RenderPreview(void* data, uint32_t cx, uint32_t cy)
{
	BiLiOBSMainWid *window = static_cast<BiLiOBSMainWid*>(data);
	obs_video_info ovi;

	obs_get_video_info(&ovi);

	//window->previewCX = int(window->previewScale * float(ovi.base_width));
	//window->previewCY = int(window->previewScale * float(ovi.base_height));

	gs_viewport_push();
	gs_projection_push();

	/* --------------------------------------- */

	QSize previewSize = window->obsPreview->size() * window->obsPreview->devicePixelRatio();

	gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
		-100.0f, 100.0f);
	gs_set_viewport(0, 0, previewSize.width(), previewSize.height());

	gs_effect_t    *solid = obs_get_solid_effect();
	gs_eparam_t    *color = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

	//begin draw background
	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);
	gs_matrix_push();

	gs_matrix_identity();
	gs_matrix_scale3f(window->sourceWidth, window->sourceHeight, 1.0f);

	vec4 backgroundColorVal;
	//vec4_set(&backgroundColorVal, 0.1678f, 0.1678f, 0.1678f, 1.0f);
	vec4_set(&backgroundColorVal, 0.0f, 0.0f, 0.0f, 1.0f);
	gs_effect_set_vec4(color, &backgroundColorVal);

	gs_load_vertexbuffer(window->mBox);
	gs_draw(GS_TRISTRIP, 0, 0);
	//end draw background


	//begin draw inner background
	//gs_matrix_identity();
	//gs_matrix_translate3f(window->previewX, window->previewY, 0);
	//
	//vec4 innerBackgroundColorVal;
	//vec4_set(&innerBackgroundColorVal, 0.0, 0.0f, 0.0f, 1.0f);
	//gs_effect_set_vec4(color, &innerBackgroundColorVal);
	//gs_load_vertexbuffer(window->previewBox);
	//gs_draw(GS_TRISTRIP, 0, 0);
	//end draw inner background

	gs_matrix_pop();
	gs_technique_end_pass(tech);
	gs_technique_end(tech);

	//end draw background

	gs_set_viewport(window->mPreviewX, window->mPreviewY, previewSize.width() - window->mPreviewX * 2, previewSize.height() - window->mPreviewY * 2);

	gs_load_vertexbuffer(nullptr);

	obs_render_main_view();
	gs_load_vertexbuffer(nullptr);

	/* --------------------------------------- */
	gs_matrix_push();
	gs_matrix_translate3f(window->mPreviewX, window->mPreviewY, 0);

	gs_ortho(
		0, previewSize.width(),
		0, previewSize.height(),
		-100.0f, 100.0f);
	gs_reset_viewport();

	window->obsPreview->DrawSceneEditing();
	gs_matrix_pop();

	/* --------------------------------------- */

	gs_projection_pop();
}

void BiLiOBSMainWid::OnPreviewContextMenu(QPoint mousePos)
{
	if (!sceneListWidgetOperator->GetSelectedItem())
		return;
	auto menu = sceneListWidgetOperator->createSceneEditMenu();
	auto menuAction = menu->exec(mousePos);

	if (menuAction == 0)
	{
		QPoint curMousePos = QCursor::pos();
		
		QRect previewRect = ui.LeftCenterWid->rect();
		QPoint mouseInPreview = ui.LeftCenterWid->mapFromGlobal(curMousePos);
		if (mouseInPreview.x() > 0 && mouseInPreview.y() > 0)
		{
			if (mouseInPreview.x() < previewRect.width() && mouseInPreview.y() < previewRect.height())
			{
				obsPreview->ignoreClickOnce();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////