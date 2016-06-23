#include <QGuiApplication>
#include <QMouseEvent>
#include <QMenu>

#include <algorithm>
#include <cmath>
#include <graphics/vec4.h>
#include <graphics/matrix4.h>
#include "window-basic-preview.hpp"
#include "../Src/BiLiOBSMainWid.h"
#include "../Src/BiLiApp.h"

#define HANDLE_RADIUS     4.0f
#define HANDLE_SEL_RADIUS (HANDLE_RADIUS * 2.5f)
#define CLAMP_DISTANCE    10.0f

/* TODO: make C++ math classes and clean up code here later */

OBSBasicPreview::OBSBasicPreview(QWidget *parent, Qt::WindowFlags flags)
	: OBSQTDisplay(parent, flags), isIgnoreClickOnce(false)
{
	setMouseTracking(true);
	installEventFilter(this);
}

vec2 OBSBasicPreview::GetMouseEventPos(QMouseEvent *event)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();
	float pixelRatio = main->devicePixelRatio();
	float scale = pixelRatio / main->mPreviewScale;
	vec2 pos;
	vec2_set(&pos,
		(float(event->x()) - main->mPreviewX / pixelRatio) * scale,
		(float(event->y()) - main->mPreviewY / pixelRatio) * scale);

	return pos;
}

struct SceneFindData {
	const vec2   &pos;
	OBSSceneItem item;
	bool         selectBelow;

	SceneFindData(const SceneFindData &) = delete;
	SceneFindData(SceneFindData &&) = delete;
	SceneFindData& operator=(const SceneFindData &) = delete;
	SceneFindData& operator=(SceneFindData &&) = delete;

	inline SceneFindData(const vec2 &pos_, bool selectBelow_)
		: pos         (pos_),
		  selectBelow (selectBelow_)
	{}
};

static bool SceneItemHasVideo(obs_sceneitem_t *item)
{
	obs_source_t *source = obs_sceneitem_get_source(item);
	uint32_t flags = obs_source_get_output_flags(source);
	return (flags & OBS_SOURCE_VIDEO) != 0;
}

static bool CloseFloat(float a, float b, float epsilon=0.01)
{
	using std::abs;
	return abs(a-b) <= epsilon;
}

static bool FindItemAtPos(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param)
{
	SceneFindData *data = reinterpret_cast<SceneFindData*>(param);
	matrix4       transform;
	matrix4       invTransform;
	vec3          transformedPos;
	vec3          pos3;
	vec3          pos3_;

	if (!SceneItemHasVideo(item))
		return true;

	vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

	obs_sceneitem_get_box_transform(item, &transform);

	matrix4_inv(&invTransform, &transform);
	vec3_transform(&transformedPos, &pos3, &invTransform);
	vec3_transform(&pos3_, &transformedPos, &transform);

	if (CloseFloat(pos3.x, pos3_.x) && CloseFloat(pos3.y, pos3_.y) &&
	    transformedPos.x >= 0.0f && transformedPos.x <= 1.0f &&
	    transformedPos.y >= 0.0f && transformedPos.y <= 1.0f) {
		if (data->selectBelow && obs_sceneitem_selected(item)) {
			if (data->item)
				return false;
			else
				data->selectBelow = false;
		}

		data->item = item;
	}

	UNUSED_PARAMETER(scene);
	return true;
}

static vec3 GetTransformedPos(float x, float y, const matrix4 &mat)
{
	vec3 result;
	vec3_set(&result, x, y, 0.0f);
	vec3_transform(&result, &result, &mat);
	return result;
}

static vec3 GetTransformedPosScaled(float x, float y, const matrix4 &mat,
		float scale)
{
	vec3 result;
	vec3_set(&result, x, y, 0.0f);
	vec3_transform(&result, &result, &mat);
	vec3_mulf(&result, &result, scale);
	return result;
}

static inline vec2 GetOBSScreenSize()
{
	obs_video_info ovi;
	vec2 size;
	vec2_zero(&size);

	if (obs_get_video_info(&ovi)) {
		size.x = float(ovi.base_width);
		size.y = float(ovi.base_height);
	}

	return size;
}

vec3 OBSBasicPreview::GetScreenSnapOffset(const vec3 &tl, const vec3 &br)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();
	vec2 screenSize = GetOBSScreenSize();
	vec3 clampOffset;

	vec3_zero(&clampOffset);

	const float clampDist = CLAMP_DISTANCE / main->mPreviewScale;

	if (fabsf(tl.x) < clampDist)
		clampOffset.x = -tl.x;
	if (fabsf(clampOffset.x) < EPSILON &&
	    fabsf(screenSize.x - br.x) < clampDist)
		clampOffset.x = screenSize.x - br.x;

	if (fabsf(tl.y) < clampDist)
		clampOffset.y = -tl.y;
	if (fabsf(clampOffset.y) < EPSILON &&
	    fabsf(screenSize.y - br.y) < clampDist)
		clampOffset.y = screenSize.y - br.y;

	return clampOffset;
}

OBSSceneItem OBSBasicPreview::GetItemAtPos(const vec2 &pos, bool selectBelow)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();

	OBSScene scene = main->GetCurrentScene();
	if (!scene)
		return OBSSceneItem();

	SceneFindData data(pos, selectBelow);
	obs_scene_enum_items(scene, FindItemAtPos, &data);
	return data.item;
}

static bool CheckItemSelected(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param)
{
	SceneFindData *data = reinterpret_cast<SceneFindData*>(param);
	matrix4       transform;
	vec3          transformedPos;
	vec3          pos3;

	if (!SceneItemHasVideo(item))
		return true;

	vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

	obs_sceneitem_get_box_transform(item, &transform);

	matrix4_inv(&transform, &transform);
	vec3_transform(&transformedPos, &pos3, &transform);

	if (transformedPos.x >= 0.0f && transformedPos.x <= 1.0f &&
	    transformedPos.y >= 0.0f && transformedPos.y <= 1.0f) {
		if (obs_sceneitem_selected(item)) {
			data->item = item;
			return false;
		}
	}

	UNUSED_PARAMETER(scene);
	return true;
}

bool OBSBasicPreview::SelectedAtPos(const vec2 &pos)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();

	OBSScene scene = main->GetCurrentScene();
	if (!scene)
		return false;

	SceneFindData data(pos, false);
	obs_scene_enum_items(scene, CheckItemSelected, &data);
	return !!data.item;
}

struct HandleFindData {
	const vec2   &pos;
	const float  scale;

	OBSSceneItem item;
	ItemHandle   handle = ItemHandle::None;

	HandleFindData(const HandleFindData &) = delete;
	HandleFindData(HandleFindData &&) = delete;
	HandleFindData& operator=(const HandleFindData &) = delete;
	HandleFindData& operator=(HandleFindData &&) = delete;

	inline HandleFindData(const vec2 &pos_, float scale_)
		: pos   (pos_),
		  scale (scale_)
	{}
};

static bool FindHandleAtPos(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param)
{
	if (!obs_sceneitem_selected(item))
		return true;

	HandleFindData *data = reinterpret_cast<HandleFindData*>(param);
	matrix4        transform;
	vec3           pos3;
	float          closestHandle = HANDLE_SEL_RADIUS;

	vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

	obs_sceneitem_get_box_transform(item, &transform);

	auto TestHandle = [&] (float x, float y, ItemHandle handle)
	{
		vec3 handlePos = GetTransformedPosScaled(x, y, transform,
				data->scale);

		float dist = vec3_dist(&handlePos, &pos3);
		if (dist < HANDLE_SEL_RADIUS) {
			if (dist < closestHandle) {
				closestHandle = dist;
				data->handle  = handle;
				data->item    = item;
			}
		}
	};

	TestHandle(0.0f, 0.0f, ItemHandle::TopLeft);
	TestHandle(0.5f, 0.0f, ItemHandle::TopCenter);
	TestHandle(1.0f, 0.0f, ItemHandle::TopRight);
	TestHandle(0.0f, 0.5f, ItemHandle::CenterLeft);
	TestHandle(1.0f, 0.5f, ItemHandle::CenterRight);
	TestHandle(0.0f, 1.0f, ItemHandle::BottomLeft);
	TestHandle(0.5f, 1.0f, ItemHandle::BottomCenter);
	TestHandle(1.0f, 1.0f, ItemHandle::BottomRight);

	UNUSED_PARAMETER(scene);
	return true;
}

static vec2 GetItemSize(obs_sceneitem_t *item)
{
	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(item);
	vec2 size;

	if (boundsType != OBS_BOUNDS_NONE) {
		obs_sceneitem_get_bounds(item, &size);
	} else {
		obs_source_t *source = obs_sceneitem_get_source(item);
		vec2 scale;

		obs_sceneitem_get_scale(item, &scale);
		size.x = float(obs_source_get_width(source))  * scale.x;
		size.y = float(obs_source_get_height(source)) * scale.y;
	}

	return size;
}

void OBSBasicPreview::GetStretchHandleData(const vec2 &pos)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();

	OBSScene scene = main->GetCurrentScene();
	if (!scene)
		return;

	HandleFindData data(pos, main->mPreviewScale / main->devicePixelRatio());
	obs_scene_enum_items(scene, FindHandleAtPos, &data);

	stretchItem     = std::move(data.item);
	stretchHandle   = data.handle;

	if (stretchHandle != ItemHandle::None) {
		matrix4 boxTransform;
		vec3    itemUL;
		float   itemRot;

		stretchItemSize = GetItemSize(stretchItem);

		obs_sceneitem_get_box_transform(stretchItem, &boxTransform);
		itemRot = obs_sceneitem_get_rot(stretchItem);
		vec3_from_vec4(&itemUL, &boxTransform.t);

		/* build the item space conversion matrices */
		matrix4_identity(&itemToScreen);
		matrix4_rotate_aa4f(&itemToScreen, &itemToScreen,
				0.0f, 0.0f, 1.0f, RAD(itemRot));
		matrix4_translate3f(&itemToScreen, &itemToScreen,
				itemUL.x, itemUL.y, 0.0f);

		matrix4_identity(&screenToItem);
		matrix4_translate3f(&screenToItem, &screenToItem,
				-itemUL.x, -itemUL.y, 0.0f);
		matrix4_rotate_aa4f(&screenToItem, &screenToItem,
				0.0f, 0.0f, 1.0f, RAD(-itemRot));
	}
}

void OBSBasicPreview::mousePressEvent(QMouseEvent *event)
{
	//disable right button
	if (event->button() == Qt::RightButton)
		return;

	if (isIgnoreClickOnce)
	{
		isIgnoreClickOnce = false;
		return;
	}

	BiLiOBSMainWid* main = App()->mGetMainWindow();
	float pixelRatio = main->devicePixelRatio();
	float x = float(event->x()) - main->mPreviewX / pixelRatio;
	float y = float(event->y()) - main->mPreviewY / pixelRatio;

	OBSQTDisplay::mousePressEvent(event);

	if (event->button() != Qt::LeftButton &&
	    event->button() != Qt::RightButton)
		return;

	if (event->button() == Qt::LeftButton)
		mouseDown = true;

	vec2_set(&startPos, x, y);
	GetStretchHandleData(startPos);

	vec2_divf(&startPos, &startPos, main->mPreviewScale / pixelRatio);
	startPos.x = std::round(startPos.x);
	startPos.y = std::round(startPos.y);

	mouseOverItems = SelectedAtPos(startPos);
	vec2_zero(&lastMoveOffset);

	obs_source_t* source = obs_sceneitem_get_source(stretchItem); //不需要释放
	if (source)
	{
		cropFilter = obs_source_get_filter_by_name(source, "cropFilter");
		if (static_cast<obs_source_t*>(cropFilter) != 0)
			obs_source_release(cropFilter); //cropfilter是智能指针，这里要释放抵消get的返回

		//备份开始操作前的数据
		obs_data_t* settings = obs_source_get_settings(cropFilter);
		cropSettingsOnStart = obs_data_create();
		obs_data_release(cropSettingsOnStart); //上面那个是智能指针，这里要释放抵消data_create的返回
		obs_data_apply(cropSettingsOnStart, settings);
		obs_data_release(settings);

		obs_sceneitem_get_bounds(stretchItem, &itemBoundOnStart);
		obs_sceneitem_get_pos(stretchItem, &itemPosOnStart);
	}

	if (event->button() == Qt::RightButton)
		ProcessClick(startPos);
}

static bool select_one(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	obs_sceneitem_t *selectedItem =
		reinterpret_cast<obs_sceneitem_t*>(param);
	obs_sceneitem_select(item, (selectedItem == item));

	UNUSED_PARAMETER(scene);
	return true;
}

void OBSBasicPreview::DoSelect(const vec2 &pos)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();

	OBSScene     scene = main->GetCurrentScene();
	OBSSceneItem item  = GetItemAtPos(pos, true);

	obs_scene_enum_items(scene, select_one, (obs_sceneitem_t*)item);
}

void OBSBasicPreview::DoCtrlSelect(const vec2 &pos)
{
	OBSSceneItem item = GetItemAtPos(pos, false);
	if (!item)
		return;

	bool selected = obs_sceneitem_selected(item);
	obs_sceneitem_select(item, !selected);
}

void OBSBasicPreview::ProcessClick(const vec2 &pos)
{
	Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();

	if (modifiers & Qt::ControlModifier)
		DoCtrlSelect(pos);
	else
		DoSelect(pos);
}

void OBSBasicPreview::mouseReleaseEvent(QMouseEvent *event)
{
	if (mouseDown) {
		vec2 pos = GetMouseEventPos(event);

		if (!mouseMoved)
			ProcessClick(pos);

		stretchItem = nullptr;
		mouseDown   = false;
		mouseMoved  = false;
	}
}

struct SelectedItemBounds {
	bool first = true;
	vec3 tl, br;
};

static bool AddItemBounds(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param)
{
	SelectedItemBounds *data = reinterpret_cast<SelectedItemBounds*>(param);

	if (!obs_sceneitem_selected(item))
		return true;

	matrix4 boxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);

	vec3 t[4] = {
		GetTransformedPos(0.0f, 0.0f, boxTransform),
		GetTransformedPos(1.0f, 0.0f, boxTransform),
		GetTransformedPos(0.0f, 1.0f, boxTransform),
		GetTransformedPos(1.0f, 1.0f, boxTransform)
	};

	for (const vec3 &v : t) {
		if (data->first) {
			vec3_copy(&data->tl, &v);
			vec3_copy(&data->br, &v);
			data->first = false;
		} else {
			vec3_min(&data->tl, &data->tl, &v);
			vec3_max(&data->br, &data->br, &v);
		}
	}

	UNUSED_PARAMETER(scene);
	return true;
}

void OBSBasicPreview::SnapItemMovement(vec2 &offset)
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();
	OBSScene scene = main->GetCurrentScene();

	SelectedItemBounds data;
	obs_scene_enum_items(scene, AddItemBounds, &data);

	data.tl.x += offset.x;
	data.tl.y += offset.y;
	data.br.x += offset.x;
	data.br.y += offset.y;

	vec3 snapOffset = GetScreenSnapOffset(data.tl, data.br);
	offset.x += snapOffset.x;
	offset.y += snapOffset.y;
}

static bool move_items(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	vec2 *offset = reinterpret_cast<vec2*>(param);

	if (obs_sceneitem_selected(item)) {
		vec2 pos;
		obs_sceneitem_get_pos(item, &pos);
		vec2_add(&pos, &pos, offset);
		obs_sceneitem_set_pos(item, &pos);
	}

	UNUSED_PARAMETER(scene);
	return true;
}

void OBSBasicPreview::moveItemByKeyboard_(vec2 moveOffset){

	OBSScene scene = App()->mGetMainWindow()->GetCurrentScene();
	obs_scene_enum_items(scene, move_items, &moveOffset);
}

void OBSBasicPreview::MoveItems(const vec2 &pos)
{
	Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
	BiLiOBSMainWid* main = App()->mGetMainWindow();
	OBSScene scene = main->GetCurrentScene();

	vec2 offset, moveOffset;
	vec2_sub(&offset, &pos, &startPos);
	vec2_sub(&moveOffset, &offset, &lastMoveOffset);

	if (!(modifiers & Qt::ControlModifier))
		SnapItemMovement(moveOffset);

	vec2_add(&lastMoveOffset, &lastMoveOffset, &moveOffset);

	obs_scene_enum_items(scene, move_items, &moveOffset);
}

vec3 OBSBasicPreview::CalculateStretchPos(const vec3 &tl, const vec3 &br)
{
	uint32_t alignment = obs_sceneitem_get_alignment(stretchItem);
	vec3 pos;

	vec3_zero(&pos);

	if (alignment & OBS_ALIGN_LEFT)
		pos.x = tl.x;
	else if (alignment & OBS_ALIGN_RIGHT)
		pos.x = br.x;
	else
		pos.x = (br.x - tl.x) * 0.5f + tl.x;

	if (alignment & OBS_ALIGN_TOP)
		pos.y = tl.y;
	else if (alignment & OBS_ALIGN_BOTTOM)
		pos.y = br.y;
	else
		pos.y = (br.y - tl.y) * 0.5f + tl.y;

	return pos;
}

void OBSBasicPreview::ClampAspect(vec3 &tl, vec3 &br, vec2 &size,
		const vec2 &baseSize)
{
	float    baseAspect   = baseSize.x / baseSize.y;
	float    aspect       = size.x / size.y;
	uint32_t stretchFlags = (uint32_t)stretchHandle;

	if (stretchHandle == ItemHandle::TopLeft    ||
	    stretchHandle == ItemHandle::TopRight   ||
	    stretchHandle == ItemHandle::BottomLeft ||
	    stretchHandle == ItemHandle::BottomRight) {
		if (aspect < baseAspect)
			size.x = size.y * baseAspect;
		else
			size.y = size.x / baseAspect;

	} else if (stretchHandle == ItemHandle::TopCenter ||
	           stretchHandle == ItemHandle::BottomCenter) {
		size.x = size.y * baseAspect;

	} else if (stretchHandle == ItemHandle::CenterLeft ||
	           stretchHandle == ItemHandle::CenterRight) {
		size.y = size.x / baseAspect;
	}

	size.x = std::round(size.x);
	size.y = std::round(size.y);

	if (stretchFlags & ITEM_LEFT)
		tl.x = br.x - size.x;
	else if (stretchFlags & ITEM_RIGHT)
		br.x = tl.x + size.x;

	if (stretchFlags & ITEM_TOP)
		tl.y = br.y - size.y;
	else if (stretchFlags & ITEM_BOTTOM)
		br.y = tl.y + size.y;
}

void OBSBasicPreview::SnapStretchingToScreen(vec3 &tl, vec3 &br)
{
	uint32_t stretchFlags = (uint32_t)stretchHandle;
	vec3     newTL        = GetTransformedPos(tl.x, tl.y, itemToScreen);
	vec3     newTR        = GetTransformedPos(br.x, tl.y, itemToScreen);
	vec3     newBL        = GetTransformedPos(tl.x, br.y, itemToScreen);
	vec3     newBR        = GetTransformedPos(br.x, br.y, itemToScreen);
	vec3     boundingTL;
	vec3     boundingBR;

	vec3_copy(&boundingTL, &newTL);
	vec3_min(&boundingTL, &boundingTL, &newTR);
	vec3_min(&boundingTL, &boundingTL, &newBL);
	vec3_min(&boundingTL, &boundingTL, &newBR);

	vec3_copy(&boundingBR, &newTL);
	vec3_max(&boundingBR, &boundingBR, &newTR);
	vec3_max(&boundingBR, &boundingBR, &newBL);
	vec3_max(&boundingBR, &boundingBR, &newBR);

	vec3 offset = GetScreenSnapOffset(boundingTL, boundingBR);
	vec3_add(&offset, &offset, &newTL);
	vec3_transform(&offset, &offset, &screenToItem);
	vec3_sub(&offset, &offset, &tl);

	if (stretchFlags & ITEM_LEFT)
		tl.x += offset.x;
	else if (stretchFlags & ITEM_RIGHT)
		br.x += offset.x;

	if (stretchFlags & ITEM_TOP)
		tl.y += offset.y;
	else if (stretchFlags & ITEM_BOTTOM)
		br.y += offset.y;
}

void OBSBasicPreview::StretchItem(const vec2 &pos)
{
	obs_source_t *source = obs_sceneitem_get_source(stretchItem);
	Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(stretchItem);
	uint32_t stretchFlags = (uint32_t)stretchHandle;
	bool shiftDown = (modifiers & Qt::ShiftModifier);
	vec3 tl, br, pos3;

	vec3_zero(&tl);
	vec3_set(&br, stretchItemSize.x, stretchItemSize.y, 0.0f);

	vec3_set(&pos3, pos.x, pos.y, 0.0f);
	vec3_transform(&pos3, &pos3, &screenToItem);

	vec2 baseSize;
	vec2_set(&baseSize,
		float(obs_source_get_width(source)),
		float(obs_source_get_height(source)));

	//对于拖动不同的控制点有不同的行为
	//拖动四个角：能够按照比例缩放
	//拖动上下左右的点：能够不按照比例缩放
	switch (stretchFlags)
	{
	//拖动上下左右四个点的情况
	case ITEM_LEFT:
		tl.x = pos3.x;
		break;

	case ITEM_RIGHT:
		br.x = pos3.x;
		break;

	case ITEM_TOP:
		tl.y = pos3.y;
		break;

	case ITEM_BOTTOM:
		br.y = pos3.y;
		break;

	//拖动四个角的点的情况
	default: {
		auto computeNewSize = [](int cx, int cy, int dx, int dy, int* newCX, int* newCY)->void
		{
			double xScale = double(cx + dx) / cx;
			double yScale = double(cy + dy) / cy;
			double avgScale = (xScale + yScale) / 2;
			*newCX = cx * avgScale;
			*newCY = cy * avgScale;
		};

		int newCX, newCY;
		if (stretchFlags == (ITEM_LEFT | ITEM_TOP))
		{
			//往左拉变大，往上拉变大
			computeNewSize(stretchItemSize.x, stretchItemSize.y, tl.x - pos3.x, tl.y - pos3.y, &newCX, &newCY);
			tl.x = br.x - newCX;
			tl.y = br.y - newCY;
		}
		else if (stretchFlags == (ITEM_LEFT | ITEM_BOTTOM))
		{
			//往左拉变大，往下拉变大
			computeNewSize(stretchItemSize.x, stretchItemSize.y, tl.x - pos3.x, pos3.y - br.y, &newCX, &newCY);
			tl.x = br.x - newCX;
			br.y = tl.y + newCY;
		}
		else if (stretchFlags == (ITEM_RIGHT | ITEM_TOP))
		{
			//往右拉变大，往上拉变大
			computeNewSize(stretchItemSize.x, stretchItemSize.y, pos3.x - br.x, tl.y - pos3.y, &newCX, &newCY);
			br.x = tl.x + newCX;
			tl.y = br.y - newCY;
		}
		else if (stretchFlags == (ITEM_RIGHT | ITEM_BOTTOM))
		{
			//往右拉变大，往下拉变大
			computeNewSize(stretchItemSize.x, stretchItemSize.y, pos3.x - br.x, pos3.y - br.y, &newCX, &newCY);
			br.x = tl.x + newCX;
			br.y = tl.y + newCY;
		}
		else
		{
			//？！
			assert(0);
		}
		break;
	} //default: {
	}


	if (!(modifiers & Qt::ControlModifier))
		SnapStretchingToScreen(tl, br);

	vec2 size;
	vec2_set(&size, br.x - tl.x, br.y - tl.y);

	if (boundsType != OBS_BOUNDS_NONE) {
		if (shiftDown)
			ClampAspect(tl, br, size, baseSize);

		if (tl.x > br.x) std::swap(tl.x, br.x);
		if (tl.y > br.y) std::swap(tl.y, br.y);

		vec2_abs(&size, &size);

		obs_sceneitem_set_bounds(stretchItem, &size);
	} else {
		//if (!shiftDown)
		//	ClampAspect(tl, br, size, baseSize);

		vec2_div(&size, &size, &baseSize);
		obs_sceneitem_set_scale(stretchItem, &size);
	}

	pos3 = CalculateStretchPos(tl, br);
	vec3_transform(&pos3, &pos3, &itemToScreen);

	vec2 newPos;
	vec2_set(&newPos, std::round(pos3.x), std::round(pos3.y));
	obs_sceneitem_set_pos(stretchItem, &newPos);
}

void OBSBasicPreview::CropItem(const vec2& pos)
{
	if (QGuiApplication::keyboardModifiers() & Qt::AltModifier && stretchHandle != ItemHandle::None)
	{
		obs_source_t* selectedSource = obs_sceneitem_get_source(stretchItem);
		if (selectedSource == 0)
			return;

		if (strcmp(obs_source_get_id(selectedSource), "text_ft2_source") == 0)
			return; //文本不给操作

		BiLiOBSMainWid* main = App()->mGetMainWindow();

		vec2 itemScale;
		obs_sceneitem_get_scale(stretchItem, &itemScale);

		//int xOffset = (pos.x - startPos.x) * itemScale.x;
		//int yOffset = (pos.y - startPos.y) * itemScale.y;

		int sourceWidth = obs_source_get_base_width(selectedSource);
		int sourceHeight = obs_source_get_base_height(selectedSource);

		int xOffset = pos.x - startPos.x;
		int yOffset = pos.y - startPos.y;

		if (xOffset != 0 && yOffset != 0)
		{
			if (static_cast<obs_source_t*>(cropFilter) == 0)
			{
				obs_data_release(cropSettingsOnStart);
				cropSettingsOnStart = obs_data_create();

				cropFilter = obs_source_create(OBS_SOURCE_TYPE_FILTER, "crop_filter", "cropFilter", nullptr, nullptr);
				obs_source_release(cropFilter);

				obs_source_filter_add(selectedSource, cropFilter);
			}
		}

		int startLeft = obs_data_get_int(cropSettingsOnStart, "left");
		int startRight = obs_data_get_int(cropSettingsOnStart, "right");
		int startTop = obs_data_get_int(cropSettingsOnStart, "top");
		int startBottom = obs_data_get_int(cropSettingsOnStart, "bottom");

		vec2 itemNewPos = itemPosOnStart;
		//vec2 itemNewBound = itemBoundOnStart;

		if ((uint32_t)stretchHandle & ITEM_LEFT)
		{
			itemNewPos.x += xOffset;
			//itemNewBound.x -= xOffset;
			startLeft += xOffset / itemScale.x;
		}
		else if ((uint32_t)stretchHandle & ITEM_RIGHT)
		{
			//itemNewBound.x += xOffset;
			startRight -= xOffset / itemScale.x;
		}

		if ((uint32_t)stretchHandle & ITEM_TOP)
		{
			itemNewPos.y += yOffset;
			//itemNewBound.y -= yOffset;
			startTop += yOffset / itemScale.y;
		}
		else if ((uint32_t)stretchHandle & ITEM_BOTTOM)
		{
			//itemNewBound.y += yOffset;
			startBottom -= yOffset / itemScale.y;
		}

		//校正数据
		if (startLeft < 0) startLeft = 0, itemNewPos.x = itemPosOnStart.x;
		if (startTop < 0) startTop = 0, itemNewPos.y = itemPosOnStart.y;
		if (startRight < 0) startRight = 0;
		if (startBottom < 0) startBottom = 0;

		if (startLeft + startRight > sourceWidth - 1)
		{
			//左边往右拖拖过头了
			if ((uint32_t)stretchHandle & ITEM_LEFT)
			{
				startLeft = sourceWidth - startRight - 1;
				itemNewPos.x = itemPosOnStart.x + sourceWidth * itemScale.x;
			}
			//右边往左拖拖过头了
			else if ((uint32_t)stretchHandle & ITEM_RIGHT)
			{
				startRight = sourceWidth - startLeft - 1;
			}
		}
		if (startTop + startBottom > sourceHeight - 1)
		{
			//上面往下拖拖过头了
			if ((uint32_t)stretchHandle & ITEM_TOP)
			{
				startTop = sourceHeight - startBottom - 1;
				itemNewPos.y = itemPosOnStart.y + sourceHeight * itemScale.x;
			}
			//下面往上拖拖过头了
			else if (((uint32_t)stretchHandle & ITEM_BOTTOM))
			{
				startBottom = sourceHeight - startTop - 1;
			}
		}

		obs_sceneitem_set_pos(stretchItem, &itemNewPos);
		//obs_sceneitem_set_bounds(stretchItem, &itemNewBound);

		obs_data_t* currentSettings = obs_source_get_settings(cropFilter);
		obs_data_set_int(currentSettings, "left", startLeft);
		obs_data_set_int(currentSettings, "right", startRight);
		obs_data_set_int(currentSettings, "top", startTop);
		obs_data_set_int(currentSettings, "bottom", startBottom);
		obs_source_update(cropFilter, currentSettings);
		obs_data_release(currentSettings);
	}
}

bool OBSBasicPreview::eventFilter(QObject *obj, QEvent *e) {

	if (e->type() == QEvent::ContextMenu){

		QContextMenuEvent *ctxMenuEvent = static_cast<QContextMenuEvent *>(e);
		ContextMenuSignal(ctxMenuEvent->globalPos());

		return true;
	}
	else
		return QWidget::eventFilter(obj, e);
}

void OBSBasicPreview::mAddCursorShape(QMouseEvent *e){

	BiLiOBSMainWid* main = App()->mGetMainWindow();
	float pixelRatio = main->devicePixelRatio();
	float x = float(e->x()) - main->mPreviewX / pixelRatio;
	float y = float(e->y()) - main->mPreviewY / pixelRatio;

	vec2 pos;
	vec2_set(&pos, x, y);
	vec2_divf(&pos, &pos, main->mPreviewScale / pixelRatio);
	pos.x = std::round(pos.x);
	pos.y = std::round(pos.y);
	if (!SelectedAtPos(pos)){
		setCursor(Qt::ArrowCursor);
		return;
	}

	OBSScene scene = main->GetCurrentScene();
	if (!scene)
		return;
	vec2_set(&pos, x, y);
	HandleFindData data(pos, main->mPreviewScale / main->devicePixelRatio());
	obs_scene_enum_items(scene, FindHandleAtPos, &data);

	Qt::CursorShape cShape = Qt::SizeAllCursor;
	switch (data.handle){
		case ItemHandle::TopCenter:
		case ItemHandle::BottomCenter:
			cShape = Qt::SizeVerCursor;
			break;
		case ItemHandle::CenterLeft:
		case ItemHandle::CenterRight:
			cShape = Qt::SizeHorCursor;
			break;
		case ItemHandle::TopRight:
		case ItemHandle::BottomLeft:
			cShape = Qt::SizeBDiagCursor ;
			break;
		case ItemHandle::TopLeft:
		case ItemHandle::BottomRight:
			cShape = Qt::SizeFDiagCursor;
			break;
	}
	setCursor(cShape);
}

void OBSBasicPreview::mouseMoveEvent(QMouseEvent *event)
{
	if (mouseDown) {
		vec2 pos = GetMouseEventPos(event);

		if (!mouseMoved && !mouseOverItems &&
		    stretchHandle == ItemHandle::None) {
			ProcessClick(startPos);
			mouseOverItems = SelectedAtPos(startPos);
		}

		pos.x = std::round(pos.x);
		pos.y = std::round(pos.y);

		if (stretchHandle != ItemHandle::None)
		{
			if (QGuiApplication::keyboardModifiers() & Qt::AltModifier)
				CropItem(pos);
			else
				StretchItem(pos);
		}
		else if (mouseOverItems){
			MoveItems(pos);
			setCursor(Qt::SizeAllCursor);
		}

		mouseMoved = true;
	}else
		mAddCursorShape(event);
}

static void DrawCircleAtPos(float x, float y, matrix4 &matrix,
		float previewScale)
{
	struct vec3 pos;
	vec3_set(&pos, x, y, 0.0f);
	vec3_transform(&pos, &pos, &matrix);
	vec3_mulf(&pos, &pos, previewScale);

	gs_matrix_push();
	gs_matrix_translate(&pos);
	gs_matrix_scale3f(HANDLE_RADIUS, HANDLE_RADIUS, 1.0f);
	gs_draw(GS_LINESTRIP, 0, 0);
	gs_matrix_pop();
}

bool OBSBasicPreview::DrawSelectedItem(obs_scene_t *scene,
		obs_sceneitem_t *item, void *param)
{
	if (!obs_sceneitem_selected(item))
		return true;

	BiLiOBSMainWid* main = App()->mGetMainWindow();

	matrix4 boxTransform;
	matrix4 invBoxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);
	matrix4_inv(&invBoxTransform, &boxTransform);

	vec3 bounds[] = {
		{{{0.f, 0.f, 0.f}}},
		{{{1.f, 0.f, 0.f}}},
		{{{0.f, 1.f, 0.f}}},
		{{{1.f, 1.f, 0.f}}},
	};

	bool visible = std::all_of(std::begin(bounds), std::end(bounds),
			[&](const vec3 &b)
	{
		vec3 pos;
		vec3_transform(&pos, &b, &boxTransform);
		vec3_transform(&pos, &pos, &invBoxTransform);
		return CloseFloat(pos.x, b.x) && CloseFloat(pos.y, b.y);
	});

	if (!visible)
		return true;

	gs_load_vertexbuffer(main->mCircle);

	DrawCircleAtPos(0.0f, 0.0f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(0.0f, 1.0f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(1.0f, 0.0f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(1.0f, 1.0f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(0.5f, 0.0f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(0.0f, 0.5f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(0.5f, 1.0f, boxTransform, main->mPreviewScale);
	DrawCircleAtPos(1.0f, 0.5f, boxTransform, main->mPreviewScale);

	gs_load_vertexbuffer(main->mBox);

	gs_matrix_push();
	gs_matrix_scale3f(main->mPreviewScale, main->mPreviewScale, 1.0f);
	gs_matrix_mul(&boxTransform);
	gs_draw(GS_LINESTRIP, 0, 0);

	gs_matrix_pop();

	UNUSED_PARAMETER(scene);
	UNUSED_PARAMETER(param);
	return true;
}

void OBSBasicPreview::DrawSceneEditing()
{
	BiLiOBSMainWid* main = App()->mGetMainWindow();

	gs_effect_t    *solid = obs_get_solid_effect();
	gs_technique_t *tech  = gs_effect_get_technique(solid, "Solid");

	vec4 color;
	vec4_set(&color, 1.0f, 0.0f, 0.0f, 1.0f);
	gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), &color);

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	OBSScene scene = main->GetCurrentScene();
	if (scene)
		obs_scene_enum_items(scene, DrawSelectedItem, this);

	gs_load_vertexbuffer(nullptr);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}

void OBSBasicPreview::ignoreClickOnce()
{
	isIgnoreClickOnce = true;
}
