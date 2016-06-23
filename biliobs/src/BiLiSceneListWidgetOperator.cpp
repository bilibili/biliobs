#include "BiLiSceneListWidgetOperator.hpp"
#include "bili-sceneitem-widget-item.hpp"
#include <QTabWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QPushButton>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdebug.h>
#include <QShortcut>
#include <QTime>

#include "RightListToolbar.h"

#include "BiLiApp.h"
#include "BiLiOBSMainWid.h"
#include "BiliNameDialog.hpp"
#include "BiLiPropertyDlg.h"
#include "BiLiMsgDlg.h"
#include "bili_area_cap.h"
#include "HotkeyTriggeredNotice.h"
#include "common/biliobs_paths.h"
#include "bili_obs_source_helper.h"

#include "HotkeyManager.h"

#include <list>
#include <iterator>

#include "barrage_history.h"
#include "oper_tip_dlg_factory.h"
#include "oper_tip_dlg.h"

#include "system_inquiry_dlg.h"


const char* BILI_HOTKEY_SWITCH_SCENE_NAME = "biliobs.switch-to-scene";

BiliSceneListWidgetOperator::BiliSceneListWidgetOperator(QTabWidget* tabWidget, QListWidget* listWidget, RightListToolbar* toolbar, BarrageHistory *history)
	: isInSelectedUpdating(false)
	, mTabWidget(tabWidget)
	, mSceneItemList(listWidget)
	, currentScene(0)
	, currentSceneSource(0)
	, mSceneListButton(0)
	, disableSceneMenuOnce(false)
	, mSceneListMenu(0)

	, history_wgt_(history)
{
	//OBS全局信号
	signal_handler_t* globalSignalHandler = obs_get_signal_handler();
	globalSignals.push_back(OBSSignalPtr(new OBSSignal(globalSignalHandler, "source_add", (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_source_add, this)));
	globalSignals.push_back(OBSSignalPtr(new OBSSignal(globalSignalHandler, "source_remove", (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_scene_remove, this)));
	globalSignals.push_back(OBSSignalPtr(new OBSSignal(globalSignalHandler, "source_rename", (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_source_rename, this)));

	//QT界面信号
	QObject::connect(mSceneItemList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(OnSceneListCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));
	QObject::connect(mSceneItemList, &QListWidget::itemClicked, this, &BiliSceneListWidgetOperator::OnSceneListItemClicked);
	QObject::connect(mSceneItemList, &QListWidget::itemDoubleClicked, this, &BiliSceneListWidgetOperator::OnSceneListItemDoubleClicked);

	mSceneListButton = qobject_cast<QPushButton*>(mTabWidget->tabBar()->tabButton(0, QTabBar::ButtonPosition::RightSide));
	assert(mSceneListButton);
	QObject::connect(mSceneListButton, SIGNAL(clicked()), this, SLOT(OnSceneListButtonClicked()));
	QObject::connect(mTabWidget->tabBar(), SIGNAL(tabBarClicked(int)), this, SLOT(OnSceneTabBarClicked(int)));

	mSceneItemList->installEventFilter(this);

	//工具栏信号
	QObject::connect(toolbar, &RightListToolbar::mvUpSignal, this, &BiliSceneListWidgetOperator::sceneEditMenuMoveUp);
	QObject::connect(toolbar, &RightListToolbar::mvDnSignal, this, &BiliSceneListWidgetOperator::sceneEditMenuMoveDown);
	QObject::connect(toolbar, &RightListToolbar::mvTopSignal, this, &BiliSceneListWidgetOperator::sceneEditMenuMoveTop);
	QObject::connect(toolbar, &RightListToolbar::mvBtmSignal, this, &BiliSceneListWidgetOperator::sceneEditMenuMoveBottom);
	QObject::connect(toolbar, &RightListToolbar::dltSignal, this, &BiliSceneListWidgetOperator::sceneEditMenuRemove);
	//QObject::connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnSceneTabCurrentChanged(int)));

	//auto ShortCut = new QShortcut(QKeySequence(Qt::Key_Delete), tabWidget);
	//QObject::connect(ShortCut, &QShortcut::activated, this, &BiliSceneListWidgetOperator::sceneEditMenuRemove);

	//快捷键
	HotkeyManager::GetInstance()->Register(BILI_HOTKEY_SWITCH_SCENE_NAME, CreateSourceHotkeyCallback(this, &BiliSceneListWidgetOperator::on_scene_change_hotkey_impl));
}

obs_hotkey_id BiliSceneListWidgetOperator::RegisterSwitchSceneHotkey(obs_source_t* scene)
{
	return HotkeyManager::GetInstance()->RegisterSource(scene, BILI_HOTKEY_SWITCH_SCENE_NAME);
}

void BiliSceneListWidgetOperator::OnSceneListButtonClicked()
{
	if (disableSceneMenuOnce)
	{
		disableSceneMenuOnce = false;
		return;
	}

	//获取场景列表
	mSceneListMenu = new QMenu();
	mSceneListMenu->setAttribute(Qt::WA_DeleteOnClose);
	mSceneListMenu->setStyleSheet("QMenu::item { width: 100px; height: 30px }");

	std::list<obs_source_t*> scenes;

	for (OBSSource& src : OBSEnumSources())
	{
		if (obs_source_get_type(src) == OBS_SOURCE_TYPE_INPUT)
		{
			if (strcmp(obs_source_get_id(src), "scene") == 0)
			{
				obs_source_addref(src);
				scenes.push_back(src);
			}
		}
	}

	obs_source_t* videoChannel = obs_get_output_source(0); //current output scene

#if 0
	//当前场景放在第一个
	scenes.push_front(videoChannel);
	for (auto i = ++scenes.begin(); i != scenes.end(); ++i)
	{
		if (*i == videoChannel)
		{
			scenes.erase(i);
			break;
		}
	}
#endif

	//添加到菜单，并连接行为
	for (obs_source_t* src : scenes)
	{
		QAction* sceneMenuItem = new QAction(obs_source_get_name(src), mSceneListMenu);
		sceneMenuItem->setData(qVPtr<obs_source_t>::toVariant(src));
		sceneMenuItem->setCheckable(true);
		if (src == videoChannel)
			sceneMenuItem->setChecked(true);
		QObject::connect(sceneMenuItem, SIGNAL(triggered()), this, SLOT(OnSceneListMenuItemClicked()));
		mSceneListMenu->addAction(sceneMenuItem);
	}

	obs_source_release(videoChannel);

	mSceneListButton->setStyleSheet("QPushButton { image: url(:/FucBtn/SceneUp); }");

	//菜单消失时候释放场景引用
	QObject::connect(mSceneListMenu, &QMenu::aboutToHide, std::bind(&BiliSceneListWidgetOperator::OnSceneListMenuAboutToHide, this, mSceneListMenu));
	int tabBarHeight = mTabWidget->tabBar()->height();
	mSceneListMenu->popup(mTabWidget->mapToGlobal(QPoint(0, tabBarHeight)));
}

void BiliSceneListWidgetOperator::OnSceneListMenuAboutToHide(QMenu* popupMenu)
{
	if (popupMenu == mSceneListMenu)
		mSceneListMenu = 0;
	for (QAction* act : popupMenu->actions())
	{
		obs_source_t* actsrc = qVPtr<obs_source_t>::toPtr(act->data());
		obs_source_release(actsrc);
	}
	mSceneListButton->setStyleSheet("");

	//如果是通过点击会弹出菜单的那两个东西来关闭菜单的话……
	QPoint mousePos = QCursor::pos();
	if (QApplication::mouseButtons() != 0)
	{
		if (mTabWidget->tabBar()->tabAt(mTabWidget->tabBar()->mapFromGlobal(mousePos)) == 0)
		{
			disableSceneMenuOnce = true;
		}
	}
}

void BiliSceneListWidgetOperator::OnSceneTabBarClicked(int index)
{
	if (index == 0 && mTabWidget->currentIndex() == 0)
		OnSceneListButtonClicked();
	else if (index == 1)
		history_wgt_->updateData();
}

void BiliSceneListWidgetOperator::OnSceneListMenuItemClicked()
{
	QAction* action = qobject_cast<QAction*>(sender());
	assert(action);
	obs_source_t* scenesrc = qVPtr<obs_source_t>::toPtr(action->data());
	assert(strcmp(obs_source_get_id(scenesrc), "scene") == 0);

	if (scenesrc != currentSceneSource)
	{
		obs_set_output_source(0, scenesrc);
		NotifyCurrentSceneChanged();
	}
}

void BiliSceneListWidgetOperator::OnSceneListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	//换没了
	if (current == 0)
		return;

	OnSceneListItemClicked(current);
}

void BiliSceneListWidgetOperator::OnSceneListItemClicked(QListWidgetItem *item)
{
	isInSelectedUpdating = true;

	//cancel all selected
	for (OBSSceneItem& item : OBSEnumSceneItems(currentScene))
	{
		obs_sceneitem_select(item, false);
	}

	//select
	auto x = customItemsMap.find(item);
	assert(x != customItemsMap.end());
	obs_sceneitem_t* selectedSceneItem = qVPtr<obs_sceneitem_t>::toPtr(x->second->property("obs_sceneitem"));
	assert(selectedSceneItem != 0);
	obs_sceneitem_select(selectedSceneItem, true);

	isInSelectedUpdating = false;
}

void BiliSceneListWidgetOperator::OnSceneListItemDoubleClicked(QListWidgetItem *item)
{
	sceneEditMenuProperty();
}


static const QDir getDefaultPicsDir()
{

#if 0
	std::wstring a = biliobs::GetRootPath();
	QString p = QString::fromStdWString(a);
	p.append("/data/img/");
	qDebug() << p << ;
#endif
	return QDir(QDir::currentPath().append("/data/img/"));

#if 0
	QStringList list = QCoreApplication::arguments();

	QFileInfo file(list[0]);
	
	QDir source = file.dir();
	
	bool ret;

//#ifdef NDEBUG
	ret = source.cdUp();
	assert(ret);
	ret = source.cdUp();
	assert(ret);

	source.cd("data");
	source.cd("img");
//#endif

	return source;
#endif
}

void BiliSceneListWidgetOperator::addDefaultItem(uint reso_w, uint reso_h)
{
	QString name = tr("DefaultSource");
	char const *actionName = "image_source";
	

	char const *default_pix_name = "640x480.png";
	switch (reso_w) {
	case 640:
		default_pix_name = "640x480.png";
		break;
	case 704:
		default_pix_name = "704x396.png";
		break;
    case 712:
        default_pix_name = "712x400.png";
        break;
	case 800:
		default_pix_name = "800x600.png";
		break;
	case 1280:
		default_pix_name = "1280x720.png";
		break;
    case 1920:
        default_pix_name = "1920x1080.png";
        break;
	default:
		default_pix_name = "640x480.png";
	}

	QDir def_dir = getDefaultPicsDir();
	QFileInfo file(def_dir, default_pix_name);


	obs_source_t* existedSource = obs_get_source_by_name(name.toUtf8());

	/*重名*/
	if (existedSource) {
		obs_source_release(existedSource);
		return;
	}

	obs_source_t* sceneSource = obs_get_output_source(0); //内部会增加source的引用
	if (!sceneSource)
		return;

	obs_scene_t* scene = obs_scene_from_source(sceneSource); //不会添加scene或者sceneSource的引用
	if (!scene) {
		obs_source_release(sceneSource);
		return;
	}
		


	obs_source_t* newSource = obs_source_create(OBS_SOURCE_TYPE_INPUT,
		actionName, name.toUtf8(), NULL, nullptr);

	obs_add_source(newSource);
	obs_sceneitem_t *newSceneItem = obs_scene_add(scene, newSource);

	//取消所有选择
	for (OBSSceneItem& item : OBSEnumSceneItems(scene))
	{
		obs_sceneitem_select(item, false);
	}

	obs_sceneitem_select(newSceneItem, true);

	obs_data_t* settings = obs_source_get_settings(newSource);

	obs_data_set_string(settings, "file", file.absoluteFilePath().toUtf8());

	obs_source_update(newSource, settings);

	obs_source_release(newSource);
	obs_source_release(sceneSource);
}

void BiliSceneListWidgetOperator::OnSceneTabCurrentChanged(int index)
{
	/*
	if (index >= 0 && index < SCENE_COUNT)
	{
		obs_scene_t* src = scenes[index];

		obs_set_output_source(0, obs_scene_get_source(src));
	}
	else
	{
		//tab count doesn't equal to scene count
		assert(0);
	}
	*/
}

bool BiliSceneListWidgetOperator::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == this)
		return false;

	//在预览窗口或者场景元素列表点击右键时出现的菜单
	if (event->type() == QEvent::ContextMenu)
	{
		QContextMenuEvent* contextMenuEvent = static_cast<QContextMenuEvent*>(event);
		QListWidget* listWidget = qobject_cast<QListWidget*>(watched);

		if (listWidget != 0 )
		{
			if (GetSelectedItem() != 0)
			{
				auto menu = createSceneEditMenu();
				auto menuAction = menu->exec(contextMenuEvent->globalPos());

				return true;
			}
		}
	}

	return false;
}

QMenu* BiliSceneListWidgetOperator::createSceneEditMenu()
{
	auto menu = new QMenu();
	if (menu->objectName().isEmpty())
        menu->setObjectName(tr("RightMenu"));

	struct {
		const char* itemName;
		const char* slot;
	} menuItems[] =
	{
		"1:1\xe6\x98\xbe\xe7\xa4\xba", SLOT(sceneEditMenuResetSize()), //1:1显示
		"\xe5\x85\xa8\xe5\xb1\x8f\xe6\x98\xbe\xe7\xa4\xba", SLOT(sceneEditMenuFullScreenSize()), //全屏显示
		"\xe5\x88\xa0\xe9\x99\xa4\xe5\xbd\x93\xe5\x89\x8d\xe9\xa1\xb9", SLOT(sceneEditMenuRemove()), //删除当前项
		"\xe7\xbd\xae\xe4\xba\x8e\xe9\xa1\xb6\xe5\xb1\x82", SLOT(sceneEditMenuMoveTop()), //置于顶层
		"\xe7\xbd\xae\xe4\xba\x8e\xe5\xba\x95\xe5\xb1\x82", SLOT(sceneEditMenuMoveBottom()), //置于底层
		"\xe4\xb8\x8a\xe7\xa7\xbb\xe4\xb8\x80\xe5\xb1\x82", SLOT(sceneEditMenuMoveUp()), //上移一层
		"\xe4\xb8\x8b\xe7\xa7\xbb\xe4\xb8\x80\xe5\xb1\x82", SLOT(sceneEditMenuMoveDown()), //下移一层
		"\xe9\x87\x8d\xe5\x91\xbd\xe5\x90\x8d", SLOT(sceneEditMenuRename()), //重命名
		"\xe5\xb1\x9e\xe6\x80\xa7", SLOT(sceneEditMenuProperty()) //属性
	};

	QAction* action;

	for (int i = 0; i < sizeof(menuItems) / sizeof(*menuItems); ++i)
	{
		QObject::connect(
			menu->addAction(menuItems[i].itemName),
			SIGNAL(triggered(bool)),
			this,
			menuItems[i].slot
			);
	}

	return menu;
}

obs_sceneitem_t* BiliSceneListWidgetOperator::GetSelectedItem()
{
	if (!currentSceneSource)
		return 0;

	obs_sceneitem_t* selectedItem = 0;
	for (OBSSceneItem& item : OBSEnumSceneItems(currentScene))
	{
		if (obs_sceneitem_selected(item))
		{
			selectedItem = item;
			break;
		}
	}

	return selectedItem;
}

void BiliSceneListWidgetOperator::loadImgOnNoItems(uint reso_w, uint reso_h)
{
	if (!currentSceneSource)
		return;

	assert(currentScene != 0);
	int itemCount = OBSEnumSceneItems(currentScene).size();

	if (itemCount == 0)
		addDefaultItem(reso_w, reso_h);
}

#if 0
void BiliSceneListWidgetOperator::on_scene_change_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed)
{
	if (pressed == false)
		return;

	BiliSceneListWidgetOperator* pThis = static_cast<BiliSceneListWidgetOperator*>(data);
	if (obs_hotkey_get_registerer_type(hotkey) == OBS_HOTKEY_REGISTERER_SOURCE)
	{
		obs_weak_source_t* weakSrc = static_cast<obs_weak_source_t*>(obs_hotkey_get_registerer(hotkey));
		obs_source_t* src = obs_weak_source_get_source(weakSrc);
		pThis->on_scene_change_hotkey_impl(src);
		obs_source_release(src);
	}
}
#endif

static void* ShowNotice(QString icon, QString text)
{
	(new HotkeyTriggeredNotice(icon, text))->show();
	return 0;
}

void BiliSceneListWidgetOperator::on_scene_change_hotkey_impl(obs_source_t* sceneSource, bool pressed)
{
	if (pressed)
	{
		BiliThreadWorker::TaskT showNoticeWidget;
		showNoticeWidget = std::bind(ShowNotice, QString(":/HotkeyTriggeredNotice/scene-switch"), tr("switch to ") + obs_source_get_name(sceneSource));
		App()->mGetMainWindow()->mInvokeProcdure(std::move(showNoticeWidget));

		if (sceneSource != currentSceneSource)
		{
			obs_set_output_source(0, sceneSource);
			QMetaObject::invokeMethod(this, "NotifyCurrentSceneChanged", Qt::QueuedConnection);
		}
	}
}

void BiliSceneListWidgetOperator::on_signal_source_add(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_source_t* sourceParam = (obs_source_t*)calldata_ptr(params, "source");

	obs_scene_t* scene = obs_scene_from_source(sourceParam);

	if (scene)
	{
		This->on_signal_scene_add_impl(scene);
	}
	else
	{
		This->on_signal_source_add_impl(sourceParam);
	}
}

void BiliSceneListWidgetOperator::on_signal_source_add_impl(obs_source_t* source)
{
	const char* srcid = obs_source_get_id(source);
	if (srcid)
	{
		if (strcmp(srcid, "text_ft2_source") == 0)
		{
			signal_handler_t* hSig = obs_source_get_signal_handler(source);
			if (hSig)
				signal_handler_connect(hSig, "ft2_height_changed", (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_ft2_source_height_changed, this);
		}
	}
}

void BiliSceneListWidgetOperator::on_signal_scene_add_impl(obs_scene_t* scene)
{
	int i;
	obs_source_t* source = obs_scene_get_source(scene);

	//连接场景的信号
	struct {
		const char* signame;
		signal_callback_t cbfunc;
	} sigdescs[] = {
		"item_add",       (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_sceneitem_add,
		"item_remove",    (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_sceneitem_remove,
		"item_select",    (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_sceneitem_select,
		"item_deselect",  (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_sceneitem_deselect,
		"reorder",        (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_sceneitem_reorder,
		"item_transform", (signal_callback_t)&BiliSceneListWidgetOperator::on_signal_sceneitem_transform
	};

	auto signalHandler = obs_source_get_signal_handler(source);
	for (i = 0; i < sizeof(sigdescs) / sizeof(*sigdescs); ++i)
	{
		SceneSignalPtr sig(new SceneSignal());
		sig->scene = scene;
		sig->signal.Connect(signalHandler, sigdescs[i].signame, sigdescs[i].cbfunc, this);
		sceneSignals.push_back(std::move(sig));
	}

	RegisterSwitchSceneHotkey(source);
}


void BiliSceneListWidgetOperator::on_signal_scene_remove(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_source_t* sourceParam = (obs_source_t*)calldata_ptr(params, "source");
	obs_scene_t* scene = obs_scene_from_source(sourceParam);

	if (scene != NULL)
		This->on_signal_scene_remove_impl(scene);
}

void BiliSceneListWidgetOperator::on_signal_scene_remove_impl(obs_scene_t* scene)
{
	//断开所有这个scene的信号
	{
		auto i = sceneSignals.begin();
		while (i != sceneSignals.end())
		{
			if ((*i)->scene == scene)
			{
				i = sceneSignals.erase(i);
			}
		}
	}
	
	//删除这个scene相关的查找表
	for (OBSSceneItem& item : OBSEnumSceneItems(scene))
	{
		auto x = obsSceneItemsMap.find(item);
		if (x != obsSceneItemsMap.end())
		{
			auto y = customItemsMap.find(x->second);
			if (y != customItemsMap.end())
			{
				delete y->first;
				delete y->second;
				customItemsMap.erase(y);
			}
			obsSceneItemsMap.erase(x);
		}
	}
}

BiliSceneWidgetItem* BiliSceneListWidgetOperator::createCustomWidgetItem(obs_scene_t* scene, obs_sceneitem_t* item)
{
	auto source = obs_sceneitem_get_source(item);
	auto sourceName = obs_source_get_name(source);
	auto listItemWidget = new BiliSceneWidgetItem(sourceName, scene, item);

	bool itemVisible = obs_sceneitem_visible(item);
	listItemWidget->setChecked(itemVisible);
	listItemWidget->setProperty("obs_sceneitem", qVPtr<obs_sceneitem_t>::toVariant(item));

	return listItemWidget;
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_add(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");
	obs_sceneitem_t* sceneItem = (obs_sceneitem_t*)calldata_ptr(params, "item");

	This->on_signal_sceneitem_add_impl(scene, sceneItem);
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_add_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem)
{
	if (currentScene == scene)
	{
		auto source = obs_sceneitem_get_source(sceneItem);
		auto sourceName = obs_source_get_name(source);
		auto listItem = new QListWidgetItem();
		auto listItemWidget = createCustomWidgetItem(scene, sceneItem);

		//listItem->setData(Qt::ItemDataRole::DisplayRole, QString(sourceName));
		listItem->setSizeHint(QSize(0, 28));

		mSceneItemList->insertItem(0, listItem);
		mSceneItemList->setItemWidget(listItem, listItemWidget);

		obsSceneItemsMap.insert(std::make_pair(sceneItem, listItem));
		customItemsMap.insert(std::make_pair(listItem, listItemWidget));
	}
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_remove(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");
	obs_sceneitem_t* sceneItem = (obs_sceneitem_t*)calldata_ptr(params, "item");

	This->on_signal_sceneitem_remove_impl(scene, sceneItem);
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_remove_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem)
{
	if (currentScene == scene)
	{
		auto x = obsSceneItemsMap.find(sceneItem);
		assert(x != obsSceneItemsMap.end());
		QListWidgetItem* widgetItem = x->second;
		mSceneItemList->removeItemWidget(widgetItem);
		delete widgetItem;

		obsSceneItemsMap.erase(x->first);
		customItemsMap.erase(widgetItem);
	}

	obs_source_remove(obs_sceneitem_get_source(sceneItem));
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_select(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	if (This->isInSelectedUpdating)
		return;

	obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");
	obs_sceneitem_t* sceneItem = (obs_sceneitem_t*)calldata_ptr(params, "item");

	This->on_signal_sceneitem_select_impl(scene, sceneItem);
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_select_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem)
{
	if (currentScene == scene)
	{
		auto x = obsSceneItemsMap.find(sceneItem);
		assert(x != obsSceneItemsMap.end());
		mSceneItemList->setCurrentItem(x->second);
	}
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_deselect(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	if (This->isInSelectedUpdating)
		return;

	obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");
	obs_sceneitem_t* sceneItem = (obs_sceneitem_t*)calldata_ptr(params, "item");

	This->on_signal_sceneitem_deselect_impl(scene, sceneItem);
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_deselect_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem)
{
	if (currentScene == scene)
	{
		auto x = obsSceneItemsMap.find(sceneItem);
		assert(x != obsSceneItemsMap.end());
		mSceneItemList->setItemSelected(x->second, false);
	}
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_reorder(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");

	This->on_signal_sceneitem_reorder_impl(scene);
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_reorder_impl(obs_scene_t* scene)
{
	if (scene == currentScene)
	{
		std::vector<obs_sceneitem_t*> newItemOrder;
		for (OBSSceneItem& item : OBSEnumSceneItems(scene))
		{
			newItemOrder.push_back(item);
		}

		//枚举出来是最底层在第一个，所以换顺序
		std::reverse(newItemOrder.begin(), newItemOrder.end());

		//重新排序
		int itemCount = newItemOrder.size();
		assert(mSceneItemList->count() == itemCount);

		bool hasChanged = false;

		for (int i = 0; i < itemCount; ++i)
		{
			QListWidgetItem* listItem = mSceneItemList->item(i);
			//检查第i项顺序是不是对的
			if (obsSceneItemsMap.at(newItemOrder[i]) == listItem)
				continue;

			hasChanged = true;

			BiliSceneWidgetItem* customItem = createCustomWidgetItem(scene, newItemOrder[i]);
			mSceneItemList->setItemWidget(listItem, customItem);

			obsSceneItemsMap.at(newItemOrder[i]) = listItem;
			customItemsMap.at(listItem) = customItem;
		}

		//重置当前选择项
		if (hasChanged)
		{
			for (auto& x : newItemOrder)
			{
				if (obs_sceneitem_selected(x))
				{
					auto listItem = obsSceneItemsMap.at(x);
					mSceneItemList->setCurrentItem(listItem);
					break;
				}
			}
		}
	}
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_transform(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");
	obs_sceneitem_t* sceneItem = (obs_sceneitem_t*)calldata_ptr(params, "item");

	This->on_signal_sceneitem_transform_impl(scene, sceneItem);
}

void BiliSceneListWidgetOperator::on_signal_sceneitem_transform_impl(obs_scene_t* scene, obs_sceneitem_t* sceneItem)
{
	obs_source_t* source = obs_sceneitem_get_source(sceneItem);
	if (source == nullptr)
		return;

	vec2 boundSize;
	obs_sceneitem_get_bounds(sceneItem, &boundSize);

	if (boundSize.x <= 0 || boundSize.y <= 0)
		return;

	//滚动filter的处理逻辑：
	//对于有滚动filter的源，一般来说bounds type不是none，是允许拖放出来的大小和源的大小比例不同的
	//现在用到的滚动只有横向滚动一种，所以可以直接把滚动filter的范围限制设置成bound的宽度

	//查找是否有滚动的filter
	obs_source_t* scrollFilter = nullptr;
	for (OBSSource& filter : OBSEnumFilters(source))
	{
		if (strcmp(obs_source_get_id(filter), "scroll_filter") == 0)
		{
			scrollFilter = filter;
			break;
		}
	};

	if (scrollFilter != 0)
	{
		if (strcmp(obs_source_get_id(scrollFilter), "scroll_filter") == 0)
		{
			int sourceWidth = obs_source_get_base_width(source);
			int sourceHeight = obs_source_get_base_height(source);
			double sourceRatio = double(sourceWidth) / sourceHeight;
			double boundRatio = double(boundSize.x) / boundSize.y;
			double scale = double(boundSize.y) / sourceHeight;
			obs_data_t* scrollConfig = obs_source_get_settings(scrollFilter);

			int boundOriginalCx = boundSize.x / scale;
			obs_data_set_int(scrollConfig, "cx", boundOriginalCx);

			obs_data_set_bool(scrollConfig, "limit_cx", true);

			obs_source_update(scrollFilter, scrollConfig);
			obs_data_release(scrollConfig);
		}
	}
}

void BiliSceneListWidgetOperator::on_signal_source_rename(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");

	This->on_signal_source_rename_impl(source);
}

void BiliSceneListWidgetOperator::on_signal_source_rename_impl(obs_source_t* source)
{
	obs_scene_t* sce = obs_scene_from_source(source);
	//如果重命名的是场景
	if (sce != 0)
	{
		if (currentSceneSource == source)
		{
			NotifyCurrentSceneChanged();
		}
	}
	//如果重命名的是场景元素
	else
	{
		//查找
		QListWidgetItem* affectedItem = 0;
		for (auto& x : obsSceneItemsMap)
		{
			obs_source_t* xsrc = obs_sceneitem_get_source(x.first);
			if (xsrc == source)
			{
				affectedItem = x.second;
				break;
			}
		}

		//找到之后改名
		if (affectedItem)
		{
			auto x = customItemsMap.find(affectedItem);
			assert(x != customItemsMap.end());
			BiliSceneWidgetItem* customItem = x->second;
			customItem->setName(obs_source_get_name(source));
		}
	}
}

void BiliSceneListWidgetOperator::on_signal_ft2_source_height_changed(BiliSceneListWidgetOperator* This, calldata_t* params)
{
	obs_source_t* src = (obs_source_t*) calldata_ptr(params, "source");
	int before_height = calldata_int(params, "before_height");
	int after_height = calldata_int(params, "after_height");
	This->on_signal_ft2_source_height_changed_impl(src, before_height, after_height);
}

void BiliSceneListWidgetOperator::on_signal_ft2_source_height_changed_impl(obs_source_t* source, int before_height, int after_height)
{
	if (before_height == 0)
		return;

	//遍历所有的scene
	for (OBSSource src : OBSEnumSources())
	{
		const char* srcid = obs_source_get_id(src);
		if (strcmp(srcid, "scene") == 0)
		{
			//遍历这个scene下所有item
			obs_scene_t* scene = obs_scene_from_source(src);
			for (OBSSceneItem item : OBSEnumSceneItems(scene))
			{
				//对source所属的sceneitem改变边框大小
				if (obs_sceneitem_get_source(item) == source)
				{
					obs_bounds_type bt = obs_sceneitem_get_bounds_type(item);
					if (bt == OBS_BOUNDS_SCALE_INNER || bt == OBS_BOUNDS_SCALE_OUTER)
					{
						vec2 bd;
						obs_sceneitem_get_bounds(item, &bd);
						bd.y = bd.y / before_height * after_height;
						obs_sceneitem_set_bounds(item, &bd);
					}
				}
			}
		}
	}
}

void BiliSceneListWidgetOperator::sceneEditMenuRemove()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem){
		//BiLiMsgDlg dlg;
		//dlg.mSetMsgTxtAndBtn(tr("Are you sure you want to remove it ?"));
		//dlg.mSetTitle(tr("Warning"));
		//dlg.exec();

        SystemInquiryDlg dlg;
        dlg.init(SystemInquiryDlg::STYLE_2);
        dlg.setInfo2(tr("Are you sure you want to remove it ?"));
        dlg.setInfo1(tr("Warning"));
        //dlg.setTitle("");
        dlg.exec();

		if (!dlg.result())
			return;
		obs_sceneitem_remove(selectedItem);
	}
}

void BiliSceneListWidgetOperator::sceneEditMenuMoveUp()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem)
		obs_sceneitem_set_order(selectedItem, OBS_ORDER_MOVE_UP);
}

void BiliSceneListWidgetOperator::sceneEditMenuMoveDown()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem)
		obs_sceneitem_set_order(selectedItem, OBS_ORDER_MOVE_DOWN);
}

void BiliSceneListWidgetOperator::sceneEditMenuMoveTop()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem)
		obs_sceneitem_set_order(selectedItem, OBS_ORDER_MOVE_TOP);
}

void BiliSceneListWidgetOperator::sceneEditMenuMoveBottom()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem)
		obs_sceneitem_set_order(selectedItem, OBS_ORDER_MOVE_BOTTOM);
}


void BiliSceneListWidgetOperator::sceneEditMenuResetSize()
{
	//设置scale为1
	//边框设置为原大小

	auto selectedItem = GetSelectedItem();
	if (selectedItem)
	{
		obs_video_info ovi;
		obs_get_video_info(&ovi);

		obs_source_t* selectedSource = obs_sceneitem_get_source(selectedItem); //不添加引用不需要释放
		int sourceWidth = obs_source_get_base_width(selectedSource);
		int sourceHeight = obs_source_get_base_height(selectedSource);

		obs_transform_info itemInfo;
		obs_sceneitem_get_info(selectedItem, &itemInfo);

		vec2_set(&itemInfo.scale, 1.0f, 1.0f);

		itemInfo.rot = 0.0f;

		vec2_set(&itemInfo.bounds,
			float(sourceWidth), float(sourceHeight));

		obs_sceneitem_set_info(selectedItem, &itemInfo);

		vec2 boundSize;
		boundSize.x = obs_source_get_base_width(selectedSource);
		boundSize.y = obs_source_get_base_height(selectedSource);
		obs_sceneitem_set_bounds(selectedItem, &boundSize);
	}
}

void BiliSceneListWidgetOperator::sceneEditMenuFullScreenSize()
{
	//保持比例的情况下把高或者宽设置成和屏幕一样
	//并且处在屏幕正中

	auto selectedItem = GetSelectedItem();
	if (selectedItem)
	{
		sceneEditMenuFullScreenSizeImpl(selectedItem);
	}
}

void BiliSceneListWidgetOperator::sceneEditMenuFullScreenSizeImpl(obs_sceneitem_t* sceneItem)
{
	obs_video_info ovi;
	obs_get_video_info(&ovi);

	//获取选中项的分高宽，计算比例
	obs_source_t* selectedSource = obs_sceneitem_get_source(sceneItem); //不添加引用不需要释放
	int sourceWidth = obs_source_get_base_width(selectedSource);
	int sourceHeight = obs_source_get_base_height(selectedSource);
	float sourceRatio = float(sourceWidth) / sourceHeight;

	//获取整个输出内容的高宽，计算比例
	int outputWidth = obs_source_get_base_width(currentSceneSource);
	int outputHeight = obs_source_get_base_height(currentSceneSource);
	float outputRatio = float(outputWidth) / outputHeight;

	//计算居中并放到最大时的高宽、缩放比例和所处位置
	float destWidth;
	float destHeight;
	float destX;
	float destY;
	float scale;
	if (sourceRatio > outputRatio)
	{
		destWidth = outputWidth;
		destHeight = destWidth / sourceRatio;
		destX = 0;
		destY = (outputHeight - destHeight) / 2;
		scale = destWidth / sourceWidth;
	}
	else
	{
		destHeight = outputHeight;
		destWidth = destHeight * sourceRatio;
		destY = 0;
		destX = (outputWidth - destWidth) / 2;
		scale = destWidth / sourceWidth;
	}

	//进行缩放
	obs_transform_info itemInfo;
	obs_sceneitem_get_info(sceneItem, &itemInfo);

	vec2_set(&itemInfo.pos, destX, destY);
	vec2_set(&itemInfo.scale, scale, scale);

	itemInfo.rot = 0.0f;

	vec2_set(&itemInfo.bounds,
		destWidth, destHeight);

	obs_sceneitem_set_info(sceneItem, &itemInfo);

	vec2 boundSize;
	boundSize.x = destWidth;
	boundSize.y = destHeight;
	obs_sceneitem_set_bounds(sceneItem, &boundSize);
}

void BiliSceneListWidgetOperator::mPopPropDlg(BiLiPropertyDlg *pDlg, obs_sceneitem_t *item) {
	QObject::connect(pDlg, &QDialog::finished, std::bind(&BiliSceneListWidgetOperator::mOnPropDlgFinished, this, item, std::placeholders::_1));
	pDlg->open();
}

void BiliSceneListWidgetOperator::mOnPropDlgFinished(obs_sceneitem_t* sceneItem, int ret)
{
	if (ret == 2)							//gif to media
		obs_sceneitem_remove(sceneItem);
}

void BiliSceneListWidgetOperator::sceneEditMenuProperty()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem)
	{
		QWidget* mainWnd = App()->mGetMainWindow();

		QString name = obs_source_get_name(obs_sceneitem_get_source(selectedItem));

		//BiLiPropertyDlg* propDlg = new BiLiPropertyDlg(obs_sceneitem_get_source(selectedItem), false, mainWnd);
		std::string sourceId = obs_source_get_id(obs_sceneitem_get_source(selectedItem));
		BiLiPropertyDialogFactory* dlgFactory = GetBiLiPropertyDialogFactory(sourceId.c_str());
		assert(dlgFactory);
		BiLiPropertyDlg* propDlg = dlgFactory->Create(name, selectedItem, false, mainWnd);
		//propDlg->setWindowFlags(Qt::FramelessWindowHint);
		propDlg->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

#if 1
		mPopPropDlg(propDlg, selectedItem);
#else
		//propDlg->show();
		if (propDlg->exec() == 2)
		{
			obs_sceneitem_remove(selectedItem);
		}
#endif
	}
}


void BiliSceneListWidgetOperator::sceneEditMenuRename()
{
	auto selectedItem = GetSelectedItem();
	if (selectedItem)
	{
		QWidget* mainWnd = App()->mGetMainWindow();

		auto selectedSource = obs_sceneitem_get_source(selectedItem);
		QString currentName = obs_source_get_name(selectedSource);
		BiliNameDialog nameDialog(currentName);
		nameDialog.exec();
		if (nameDialog.result() == QDialog::Accepted)
		{
			QByteArray nameData = currentName.toUtf8();
			obs_source_t* dupNameSrc = obs_get_source_by_name(nameData.data());
			if (dupNameSrc) //重名
			{
				obs_source_release(dupNameSrc);


#if 0
				//重名时候的处理
				QMessageBox errMsg(mainWnd);
				errMsg.setWindowTitle(tr("Error"));
				errMsg.setText(tr("Duplicated name!"));
				errMsg.exec();
#else
				//BiLiMsgDlg errDlg;
				//errDlg.mSetMsgTxtAndBtn(tr("Duplicated name!"), false);
				//errDlg.mSetTitle(tr("Error"));
				//errDlg.exec();

                OperTipDlg *errDlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::NAME_DUPLICATE);
                errDlg->exec();
                delete errDlg;

#endif
				return;
			}
			else //不重名
			{
				obs_source_set_name(selectedSource, currentName.toUtf8().data());
			}
		}
	}
}


void BiliSceneListWidgetOperator::NotifyCurrentSceneChanged()
{
	if (mSceneListMenu != 0)
	{
		mSceneListMenu->hide();
		mSceneListMenu->deleteLater();
		mSceneListMenu = 0;
	}

	obs_source_t* scenesrc = obs_get_output_source(0);
	obs_scene_t* scene = obs_scene_from_source(scenesrc);

	currentSceneSource = scenesrc;
	currentScene = scene;

	//更新list
	mSceneItemList->clear();
	obsSceneItemsMap.clear();
	customItemsMap.clear();

	for (OBSSceneItem& x : OBSEnumSceneItems(scene))
		on_signal_sceneitem_add_impl(scene, x);

	//标签名
	mTabWidget->tabBar()->setTabText(0, obs_source_get_name(currentSceneSource));
}
