#ifndef BILI_AREA_CAP_H
#define BILI_AREA_CAP_H

#include <QWidget>
#include <vector>

class QShortcut;
class QPushButton;
class bili_area_tool;
class bili_area_zoom;
class BiLiPropertyDlg;

class bili_area_cap : public QWidget {

	Q_OBJECT

public:
	bili_area_cap(BiLiPropertyDlg* fromPropDlg, QWidget *parent = 0);
	~bili_area_cap();

	void mShow();
	QRect mSelectedWidRect;			//相对于当前显示器左上角的坐标
	QPoint mSelectedWidRectOrgin;	//上面那个矩形的原点的绝对坐标
	
	HWND mGuessSelectedWindow() const;

	BiLiPropertyDlg* getStartupPropDlg() const { return mBelongTo; }

private slots:
	void onEscapeKeyPressed();
	void onButtonClicked(int btnId);

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent *e);
	virtual void leaveEvent(QEvent* e) override;

signals:
	void mSglSelectComplite(bool hasSelect);

private:
	BiLiPropertyDlg* mBelongTo;

	//每个显示器一个，所以有多个
	std::vector<QPixmap> mDesktopPixmaps;
	std::vector<int> mWidths;
	std::vector<int> mHeights;

	void update_current_screen();
	int mCurrentScreen;

	//就是上面那个每个显示器一个的，返回的是当前显示器的
	const QPixmap& mDesktopPixmap() const;
	int mWidth() const;
	int mHeight() const;

	bool mHasSelected;
													//自动选择时的状态
	enum MouseState {
		Default = 1<<0,
		Pressed = 1<<1,
		Move = 1<<2,
		Released = 1<<3,
		Optional = 1<<4,
		Selected = 1<<5
	};

	int mMouseState = MouseState::Default;

	//这三个坐标都是相对当前屏幕左上角的
	QPoint mPosLeftTop;
	QPoint mPressStartPos;
	QPoint mPosRightBottom;
    QPoint screen_pos_;
													//自定义选择时的状态
	bool mIsPressSelected;
	QPoint mPosPressSelected;						//相对当前显示器左上角的坐标
	int mMouseShape;

	//参数是相对屏幕左上角的坐标
	Qt::CursorShape mGetCurPosCursorShape(QPoint pos);

													//变换已经选择的区域
	void mMoveSelectRect(QPoint pos);
	void mZoomSelectRect(QPoint pos);

	//参数是相对屏幕左上角的坐标
	QPoint mGetDragBottomRight(QPoint pos);

	QShortcut *mShortCut;

	//确定和取消按钮
	bili_area_tool *mAreaTool;
	void mMoveAreaTool();
	void mSetupUI();

	//放大镜部分
	bili_area_zoom *mAreaZoom;
};

#endif // BILI_AREA_CAP_H
