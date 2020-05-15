#ifndef BILIPROPERTYDLG_H
#define BILIPROPERTYDLG_H


#include <QDialog>
#include <QTimer>
#include <QGroupBox>
#include <memory>
#include <unordered_map>
#include <functional>
#include "ui_BiLiPropertyDlg.h"

//#include "obs.h"
#include "BiliUIConfigSync.hpp"
#include "BiliFilterUtility.hpp"

#include "obs.hpp"

class PropertyDlgVolSliderWid;
class bili_area_cap;


#ifdef VOLSLIDER_WIDTH
9 = 2;   //
#endif

#define VOLSLIDER_WIDTH	8
#define VOLSLIDER_WIDGET_WIDTH 228
#define VOLSLIDER_MIN	(0 + VOLSLIDER_WIDTH / 2.0)
#define VOLSLIDER_MAX (VOLSLIDER_WIDGET_WIDTH - VOLSLIDER_WIDTH / 2.0)


typedef void(*bili_source_props)(obs_properties_t *props, void *param);
class BiliFilterMgr : public QObject {

	Q_OBJECT
public:
	obs_source_t *mSrc;
	QMap<char *, obs_source_t *> mFilterMap;
	QMap<char *, obs_data_t *> mFilterSettingsMap;

	BiliFilterMgr(obs_source_t *src);
	~BiliFilterMgr();

	bool mFilterCompatible(uint32_t sourceFlags, uint32_t filterFlags, bool async = false);
	bool mAddFilter(char *filterName, bool isCreate = true);

	obs_source_t *mGetFilter(char *filterName);
	void mGetFilterProperties(char *filterName, bili_source_props callback, void *param);
	obs_data_t *mGetFilterSettings(char *filterName);

	static void mSourceFilterAdded(void *param, calldata_t *data){

		BiliFilterMgr *obj = reinterpret_cast<BiliFilterMgr *>(param);
		obs_source_t *filter = (obs_source_t *)calldata_ptr(data, "filter");

		QMetaObject::invokeMethod(obj, "mSltAddFilter",
			Q_ARG(OBSSource, OBSSource(filter)));
	}
	static void mSourceFilterRemoved(void *param, calldata_t *data){
		BiliFilterMgr *obj = reinterpret_cast<BiliFilterMgr *>(param);
		obs_source_t *filter = (obs_source_t *)calldata_ptr(data, "filter");

		QMetaObject::invokeMethod(obj, "mSltRemoveFilter",
			Q_ARG(OBSSource, OBSSource(filter)));
	}

public slots:
	void mSltAddFilter(OBSSource filter);
	void mSltRemoveFilter(OBSSource filter);

private:
	OBSSignal mAddSignal;
	OBSSignal mRemoveSignal;
};

class BiliPropChangeEventFilter : public QObject
{
	Q_OBJECT

private:
	QTimer applyChangeTimer;
	bool isSignalTriggered;

public:
	BiliPropChangeEventFilter();
	void Watch(const std::initializer_list<QWidget*>& beWatchedWidgets);
	bool IsChangedTriggered() const;

public slots:
	void OnChangedSlot();

	void OnTextChanged();
	void OnTextChanged(const QString&);
	void OnCheckBoxChanged();
	void OnToggled(bool checked);
	void OnComboBoxIndexChanged(int index);
	void OnSilderChanged(int val);

public:
	void OnChanged();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

signals:
	void OnChangedSignal();
};


class BiLiPropertyDlg : public QDialog {

	Q_OBJECT

public:
	BiLiPropertyDlg(QString &name, obs_sceneitem_t* pSceneItem, bool isNewSource, QWidget *parent = 0);
	virtual ~BiLiPropertyDlg();

	BiliFilterMgr *filterMgr = NULL;

	bool IsLimitRectSelected() const { return mIsLimitRectSelected; }
	void SetIsLimitRectSelected(bool val) { mIsLimitRectSelected = val; }

public slots:
	void InitUI();

signals:
	void mSglScreenShotState(int state);

protected:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);

	virtual void setupSourcePropertiesUI() = 0;
	virtual int acceptSourceProperties() = 0;

	QGroupBox* CreateTipGroupBox(const QString& text);

	Ui::BiLiPropertyDlg ui;
	bool mIsPressed;
	QPoint mPoint;

	QLineEdit *mFileNameEdit;

	bool mIsNewSource;
	obs_sceneitem_t* mSceneItem;
	obs_source_t* mSrc;

	//开始-各种备份数据
	OBSData mBackupSettings;
	vec2 mBackupItemPos;
	vec2 mBackupItemScale;

	obs_bounds_type mBackupBoundsType;
	vec2 mBackupBounds;

	std::unique_ptr<BiliFiltersBackup> mBackupFilters;
	//结束-各种备份数据

	std::unique_ptr<BiliPropChangeEventFilter> mChangeEvnetFilter;

	void mSetupConnect();
	void mSetupPropertyUI();
	QString mGetMethodName(QString prefixStr, QString suffixStr);
	void mAddGifToMedia(QString gifPathStr);

	QString mSourceName;

	bool mCheckSourceNameLegal();

	QAction *mAcceptButtonAction;
	void mInitAcceptButtonAction();

	bool mIsLimitRectSelected;

private slots:
	void mOnAcceptButtonActionTriggered();

public slots:
	void mSltOnSettingChanged();
	void mOnNameEditChanged();

private slots:
	void mSltAcceptedBtn();
	void mSltRejected();
	void mSltBrowserBtn();

									//volumn
private:
	QLayout *createVolSliderLayout();

	bool volumnAccess();

	PropertyDlgVolSliderWid *vol_slider_;

};

#ifdef DEFINE_IMPL_PROPDLG

#define IMPL_PROPDLG(ClassName)\
	class ClassName : public BiLiPropertyDlg { \
		public: \
			ClassName(QString &name, obs_sceneitem_t* pSceneItem, bool isNewSource, QWidget *parent = 0) \
				: BiLiPropertyDlg(name, pSceneItem, isNewSource, parent) {} \
			~ClassName(); \
			static QString tr(const char* s) { return QApplication::translate(#ClassName, s, 0); } \
		protected: \
			void setupSourcePropertiesUI() override; \
			int acceptSourceProperties() override

#define END_IMPL_PROPDLG(ClassName, StrSourceId) }; \
	static class ClassName##Factory : public BiLiPropertyDialogFactory { \
		public: \
			ClassName##Factory() { \
				if (g_BiLiPropertyDialogFactoryList == 0) \
					g_BiLiPropertyDialogFactoryList = this; \
				if (g_curBiLiPropertyDialogFactoryList == 0) \
					g_curBiLiPropertyDialogFactoryList = g_BiLiPropertyDialogFactoryList; \
				else {\
					g_curBiLiPropertyDialogFactoryList->mNextFactory = this; \
					g_curBiLiPropertyDialogFactoryList = this; \
				} \
			}\
			~ClassName##Factory() {} \
			BiLiPropertyDlg* Create(QString &name, obs_sceneitem_t* pSceneItem, bool isNewSource, QWidget *parent = 0) override { \
				BiLiPropertyDlg* r = new ClassName(name, pSceneItem, isNewSource, parent); \
				r->InitUI();\
				return r; \
			} \
			bool CheckId(const char* sourceId) override {\
				return strcmp(sourceId, StrSourceId) == 0; \
			}\
	} s_ClassName##Factory
#endif

class BiLiPropertyDialogFactory
{
public:
	virtual ~BiLiPropertyDialogFactory() {} //所有的factory对象都是static的，所以不要释放
public:
	BiLiPropertyDialogFactory() : mNextFactory(0) {}

	virtual BiLiPropertyDlg* Create(QString &name, obs_sceneitem_t* pSceneItem, bool isNewSource, QWidget *parent = 0) = 0;
	virtual bool CheckId(const char* sourceId) = 0;

	BiLiPropertyDialogFactory* mNextFactory;
};

extern BiLiPropertyDialogFactory* g_BiLiPropertyDialogFactoryList;
extern BiLiPropertyDialogFactory* g_curBiLiPropertyDialogFactoryList; //别访问这两个，构建过程中用的

BiLiPropertyDialogFactory* GetBiLiPropertyDialogFactory(const char* sourceId);

#endif // BILIPROPERTYDLG_H
