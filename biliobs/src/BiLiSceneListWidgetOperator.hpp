#pragma once

#include "obs.h"
#include "obs.hpp"
#include <deque>
#include <memory>
#include <sstream>
#include <functional>
#include <QObject>
#include <QPoint>
#include <unordered_map>

class QTabWidget;
class QListWidgetItem;
class QListWidget;
class QMenu;
class QEvent;
class QPushButton;
class RightListToolbar;
class BiLiPropertyDlg;
class BiliSceneWidgetItem;
class BarrageHistory;

class BiliSceneListWidgetOperator : public QObject
{
	Q_OBJECT

private:
	BiliSceneListWidgetOperator(const BiliSceneListWidgetOperator&) = delete;
	BiliSceneListWidgetOperator& operator = (const BiliSceneListWidgetOperator&) = delete;

public:
	BiliSceneListWidgetOperator(QTabWidget* tabWidget, QListWidget*listWidget, RightListToolbar* toolbar, BarrageHistory *history);
	//~BiliSceneListWidgetOperator();

	QMenu* createSceneEditMenu();
	obs_sceneitem_t* GetSelectedItem();

	void loadImgOnNoItems(uint reso_w, uint reso_h);
	obs_hotkey_id RegisterSwitchSceneHotkey(obs_source_t* scene);

private slots:
	void OnSceneTabCurrentChanged(int index);
	void OnSceneTabBarClicked(int index);

	void OnSceneListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void OnSceneListItemClicked(QListWidgetItem *item);
	void OnSceneListItemDoubleClicked(QListWidgetItem*);
	void addDefaultItem(uint reso_w, uint reso_h);

	void OnSceneListButtonClicked();
	void OnSceneListMenuItemClicked();

public slots:
	void NotifyCurrentSceneChanged();

protected:
	void OnSceneListMenuAboutToHide(QMenu* popupMenu);

private:
	bool eventFilter(QObject* sender, QEvent* event) override;

private:
	BiliSceneWidgetItem* createCustomWidgetItem(obs_scene_t* scene, obs_sceneitem_t* item);

	//static void on_scene_change_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);
	void on_scene_change_hotkey_impl(obs_source_t* sceneSource, bool pressed);

	static void on_signal_source_add(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_source_add_impl(obs_source_t* source);
	void on_signal_scene_add_impl(obs_scene_t* scene);

	static void on_signal_scene_remove(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_scene_remove_impl(obs_scene_t* scene);

	static void on_signal_sceneitem_add(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_sceneitem_add_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem);

	static void on_signal_sceneitem_remove(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_sceneitem_remove_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem);

	static void on_signal_sceneitem_select(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_sceneitem_select_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem);

	static void on_signal_sceneitem_deselect(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_sceneitem_deselect_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem);

	static void on_signal_sceneitem_reorder(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_sceneitem_reorder_impl(obs_scene_t* scene);

	static void on_signal_source_rename(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_source_rename_impl(obs_source_t* source);

	static void on_signal_sceneitem_transform(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_sceneitem_transform_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem);

	static void on_signal_ft2_source_height_changed(BiliSceneListWidgetOperator* This, calldata_t* params);
	void on_signal_ft2_source_height_changed_impl(obs_source_t* source, int before_height, int after_height);

public slots:
	void sceneEditMenuRemove();

private slots:
	void sceneEditMenuMoveUp();
	void sceneEditMenuMoveDown();
	void sceneEditMenuMoveTop();
	void sceneEditMenuMoveBottom();
	void sceneEditMenuResetSize();
	void sceneEditMenuFullScreenSize();
	void sceneEditMenuProperty();
	void sceneEditMenuRename();

public:
	void sceneEditMenuFullScreenSizeImpl(obs_sceneitem_t* sceneItem);
	void mPopPropDlg(BiLiPropertyDlg *pDlg, obs_sceneitem_t *item);
	void mOnPropDlgFinished(obs_sceneitem_t* sceneItem, int ret);

private:
	QTabWidget* mTabWidget;
	QListWidget* mSceneItemList;
	QPushButton* mSceneListButton;
	BarrageHistory *const history_wgt_;

	obs_scene_t* currentScene;
	obs_source_t* currentSceneSource;

	QMenu* mSceneListMenu;

	bool disableSceneMenuOnce;

	typedef std::unique_ptr<OBSSignal> OBSSignalPtr;

	struct SceneSignal
	{
		obs_scene_t* scene;
		OBSSignal signal;
	};
	typedef std::unique_ptr<SceneSignal> SceneSignalPtr;

	//全局信号、场景和场景元素的信号对象
	std::vector<OBSSignalPtr> globalSignals;
	std::vector<SceneSignalPtr> sceneSignals;

	//list item和自定义item的对应关系
	std::unordered_map<QListWidgetItem*, BiliSceneWidgetItem*> customItemsMap;
	std::unordered_map<obs_sceneitem_t*, QListWidgetItem*> obsSceneItemsMap;

	volatile bool isInSelectedUpdating;
};

