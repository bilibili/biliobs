#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QString>
#include <memory>

#include "obs.hpp"

class QLabel;

class BiliSceneWidgetItem : public QWidget {
	Q_OBJECT

private:
	QCheckBox* itemCheckBox;

	/**
	 * 限制label的内容长度， spacer为长度超过限制时的占位符 
	 */
	void limitLabelContext(QLabel* label, const QString &ori_context, int context_len, const QString &spacer);
	QLabel* itemLabel;
	QString itemLabelFullText;

	//QPushButton* itemMoveUpBtn;
	//QPushButton* itemMoveDownBtn;

	OBSScene scene;
	OBSSceneItem sceneItem;
	OBSSignal sourceVisibleSignal;

	static void OnItemVisibleChanged(void *param, calldata_t *data);
	void OnItemVisibleChangedImpl(obs_scene_t* scene, obs_sceneitem_t* sceneItem, bool newValue);

private slots:
	void onCheckStateChanged(int state);
	void onRemoveButtonClicked();
	void onMoveUpButtonClicked();
	void onMoveDownButtonClicked();

public:
	BiliSceneWidgetItem(const char* name, obs_scene_t* pScene, obs_sceneitem_t* pSceneItem);
	~BiliSceneWidgetItem();

	QString getName();
	void setName(const char* newName);

	bool getChecked();
	void setChecked(bool newState);
};
