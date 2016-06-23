#include "bili-sceneitem-widget-item.hpp"
#include <QHBoxLayout>

const int label_context_length_restriction = 13;

BiliSceneWidgetItem::BiliSceneWidgetItem(const char* name, obs_scene_t* pScene, obs_sceneitem_t* pSceneItem)
	: scene(pScene), sceneItem(pSceneItem), itemLabelFullText(name)
{
	setStyleSheet(
		"QWidget { background-color: transparent; } \n"
		"QLabel { background-color: transparent; } \n"
		"QCheckBox { background-color: transparent; } \n"
		);

	setMaximumHeight(28);
	setMinimumHeight(28);

	QHBoxLayout *itemLayout = new QHBoxLayout();
	itemLayout->setContentsMargins(10, 2, 3, 2);

	itemCheckBox = new QCheckBox();
	itemCheckBox->setObjectName("sceneItemWidgetCheckBox");
	itemLabel = new QLabel();

	//itemMoveUpBtn->setStyleSheet(
	//	"QPushButton { background-position: center; background-repeat: no-repeat; background-image: url(:/FucBtn/MoveUp); } \n"
	//	"QPushButton:hover { background-image: url(:/FucBtn/MoveUpHS); } \n"
	//	);
	//itemMoveDownBtn->setStyleSheet(
	//	"QPushButton { background-position: center; background-repeat: no-repeat; background-image: url(:/FucBtn/MoveDown); } \n"
	//	"QPushButton:hover { background-image: url(:/FucBtn/MoveDownHS); } \n"
	//	);

	itemCheckBox->setMaximumSize(24, 24);
	itemCheckBox->setMinimumSize(24, 24);

	itemLayout->addWidget(itemCheckBox);
	itemLayout->addWidget(itemLabel);

	//itemLabel->setText(name);
	limitLabelContext(itemLabel, itemLabelFullText, label_context_length_restriction, QStringLiteral("бн"));

	setLayout(itemLayout);

	sourceVisibleSignal.Connect(obs_source_get_signal_handler(obs_scene_get_source(scene)), "item_visible", &BiliSceneWidgetItem::OnItemVisibleChanged, this);
	QObject::connect(itemCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckStateChanged(int)));
}

BiliSceneWidgetItem::~BiliSceneWidgetItem()
{
}

QString BiliSceneWidgetItem::getName()
{
	return itemLabelFullText;
}

void BiliSceneWidgetItem::setName(const char* newName)
{
	//itemLabel->setText(newName);
	itemLabelFullText = QString(newName);
	limitLabelContext(itemLabel, itemLabelFullText, label_context_length_restriction, QStringLiteral("бн"));
}

bool BiliSceneWidgetItem::getChecked()
{
	return itemCheckBox->isChecked();
}

void BiliSceneWidgetItem::setChecked(bool newState)
{
	itemCheckBox->setChecked(newState);
}

void BiliSceneWidgetItem::OnItemVisibleChanged(void *param, calldata_t *data)
{
	obs_scene_t* pScene = (obs_scene_t*)calldata_ptr(data, "scene");
	obs_sceneitem_t* pSceneItem = (obs_sceneitem_t*)calldata_ptr(data, "item");
	bool visible = calldata_bool(data, "visible");

	auto This = (BiliSceneWidgetItem*)param;
	
	This->OnItemVisibleChangedImpl(pScene, pSceneItem, visible);
}

void BiliSceneWidgetItem::OnItemVisibleChangedImpl(obs_scene_t* pScene, obs_sceneitem_t* pSceneItem, bool newValue)
{
	if (sceneItem == pSceneItem)
		setChecked(newValue);
}

void BiliSceneWidgetItem::onCheckStateChanged(int state)
{
	if (state == Qt::CheckState::Checked)
	{
		obs_sceneitem_set_visible(sceneItem, true);
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		obs_sceneitem_set_visible(sceneItem, false);
	}
	else
		assert(0);
}

void BiliSceneWidgetItem::onRemoveButtonClicked()
{
	obs_sceneitem_remove(sceneItem);
}

void BiliSceneWidgetItem::onMoveUpButtonClicked()
{
	obs_sceneitem_set_order(sceneItem, OBS_ORDER_MOVE_UP);
}

void BiliSceneWidgetItem::onMoveDownButtonClicked()
{
	obs_sceneitem_set_order(sceneItem, OBS_ORDER_MOVE_DOWN);
}

void BiliSceneWidgetItem::limitLabelContext(QLabel* label, 
	                                        const QString &ori_context, 
											int context_len, 
											const QString &spacer)
{
	QString context = ori_context;

	if (ori_context.size() <= context_len + 1)
		label->setText(context);
	else {
		context.resize(context_len);
		label->setText(context + spacer);
	}
	
}