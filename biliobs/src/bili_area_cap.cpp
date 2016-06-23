#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <Windows.h>
#include <QMouseEvent>
#include <QPushButton>
#include <QShortcut>

#include <tchar.h>
#include <string.h>

#include "bili_area_cap.h"
#include "bili_area_tool.h"

#define CAP_OUTSCREEN_AREA	0

QVector<QRect> WidsRectV;
QVector<HWND> WidsHwndV;
bool CALLBACK EnumCurWids(HWND hwnd, LPARAM lParam);

#define BORDER_WIDTH	3

bili_area_cap::bili_area_cap(BiLiPropertyDlg* fromPropDlg/*, int screen_index*/, QWidget *parent)
	: QWidget(parent),
	mIsPressSelected(false),
	mHasSelected(false),
	mMouseShape(-1),
	mBelongTo(fromPropDlg)
{
	mSetupUI();
	setMouseTracking(true);

	WidsRectV.clear();
	WidsHwndV.clear();

#if CAP_OUTSCREEN_AREA	
	struct {
		int w;
		int h;
	}_DKSize = {
		mWidth,
		mHeight
	};
	EnumWindows((WNDENUMPROC)EnumCurWids, (LPARAM)&_DKSize);
#else
	EnumWindows((WNDENUMPROC)EnumCurWids, 0);
#endif

	mShortCut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
	QObject::connect(mShortCut, &QShortcut::activated, this, &bili_area_cap::onEscapeKeyPressed);
}

bili_area_cap::~bili_area_cap() {

}

void bili_area_cap::onEscapeKeyPressed()
{
	emit mSglSelectComplite(false);
	hide();
}

void bili_area_cap::onButtonClicked(int btnId)
{
	mHasSelected = false;
	if (btnId == 0){
		if (mMouseState == MouseState::Selected)
			mHasSelected = true;
	}
	emit mSglSelectComplite(mHasSelected);
	hide();
}

void bili_area_cap::mSetupUI() {

	QDesktopWidget *dk = QApplication::desktop();
	int screenCount = dk->screenCount();
	mDesktopPixmaps.resize(screenCount);
	mWidths.resize(screenCount);
	mHeights.resize(screenCount);
	for (int i = 0; i < screenCount; ++i)
	{
		QWidget* currentScreen = dk->screen(i);
		mDesktopPixmaps[i] = QPixmap::grabWindow(currentScreen->winId(), currentScreen->pos().rx(), currentScreen->pos().ry(), currentScreen->width(), currentScreen->height());
		mWidths[i] = mDesktopPixmaps[i].width();
		mHeights[i] = mDesktopPixmaps[i].height();
	}
	update_current_screen();

	QPoint mouseAbPos = QCursor::pos();
	QRect currentScreenRect = QApplication::desktop()->screenGeometry(mouseAbPos);
    mCurrentScreen = QApplication::desktop()->screenNumber(mouseAbPos);
	move(currentScreenRect.topLeft());

	mAreaTool = new bili_area_tool(this);
	mAreaTool->hide();
	QObject::connect(mAreaTool, &bili_area_tool::mSglBtnClicked, this, &bili_area_cap::onButtonClicked);

	mAreaZoom = new bili_area_zoom(std::bind(&bili_area_cap::mDesktopPixmap, this), this);
	mAreaZoom->mMove(QCursor::pos(), mWidth(), mHeight());
	mAreaZoom->mSetZoomPos(QCursor::pos());
}

void bili_area_cap::mMoveAreaTool() {

	int bottomSpace = mHeight() - mSelectedWidRect.bottomRight().y();
	int topSpace = mSelectedWidRect.topRight().y() ;

	QPoint basePos;
	QPoint toolPos;
	int areaToolHeight = mAreaTool->height();
	if ((bottomSpace < areaToolHeight) && (topSpace < areaToolHeight)) {
		basePos = mSelectedWidRect.topRight();
		toolPos = QPoint(basePos.x()-mAreaTool->width(), basePos.y());
	}
	else if (bottomSpace < areaToolHeight){
		basePos = mSelectedWidRect.topRight();
		toolPos = QPoint(basePos.x() - mAreaTool->width(), basePos.y() - areaToolHeight);
	}
	else {
		basePos = mSelectedWidRect.bottomRight();
		toolPos = QPoint(basePos.x() - mAreaTool->width(), basePos.y());
	}
	mAreaTool->move(toolPos);
}

void bili_area_cap::mShow(){

	showFullScreen();
}

void bili_area_cap::paintEvent(QPaintEvent *e) {
	QPainter p(this);
	p.drawPixmap(0, 0, mDesktopPixmap());
	p.fillRect(mDesktopPixmap().rect(), QColor(66, 66, 66, 166));

	if ((mMouseState&MouseState::Optional) == MouseState::Optional){
		p.drawPixmap(mSelectedWidRect.left(), mSelectedWidRect.top(), mDesktopPixmap(), mSelectedWidRect.left(), mSelectedWidRect.top(), mSelectedWidRect.width(), mSelectedWidRect.height());
		QPen penBorder;
		penBorder.setWidth(3);
		penBorder.setColor(QColor(105, 165, 205, 255));
		p.setPen(penBorder);
		p.drawRect(mSelectedWidRect);
	}
	else if ( mMouseState == (MouseState::Pressed | MouseState::Move ) ){
		QRect DynamicSelectedRect(mPosLeftTop, mPosRightBottom);
		p.drawPixmap(DynamicSelectedRect.left(), DynamicSelectedRect.top(), mDesktopPixmap(), DynamicSelectedRect.left(), DynamicSelectedRect.top(), DynamicSelectedRect.width(), DynamicSelectedRect.height());
		QPen penBorder;
		penBorder.setWidth(3);
		penBorder.setColor(QColor(105, 165, 205, 255));
		p.setPen(penBorder);
		p.drawRect(DynamicSelectedRect);
	}
	else if ((mMouseState&MouseState::Selected) == MouseState::Selected){
		p.fillRect(mDesktopPixmap().rect(), QColor(66, 66, 66, 199));
		p.drawPixmap(mSelectedWidRect.left(), mSelectedWidRect.top(), mDesktopPixmap(), mSelectedWidRect.left(), mSelectedWidRect.top(), mSelectedWidRect.width(), mSelectedWidRect.height());
		QPen penBorder;
		penBorder.setWidth(3);
		penBorder.setColor(QColor(105, 165, 205, 255));
		p.setPen(penBorder);
		p.drawRect(mSelectedWidRect);

        QPoint p = pos();
        p = p;
	}
	QWidget::paintEvent(e);
}

Qt::CursorShape bili_area_cap::mGetCurPosCursorShape(QPoint pos){

	int minRecognitionPix = 3;
	Qt::CursorShape shape = Qt::ArrowCursor;

	if (qAbs(pos.y() - mSelectedWidRect.top()) < minRecognitionPix) {
		if (qAbs(pos.x() - mSelectedWidRect.left()) < minRecognitionPix) {
			shape =  Qt::SizeFDiagCursor;
			mMouseShape =  1;
		}
		else if (qAbs(pos.x() - mSelectedWidRect.right()) < minRecognitionPix) {
			shape =  Qt::SizeBDiagCursor;
			mMouseShape =  3;
		}
	}
	else if (qAbs(pos.y() - mSelectedWidRect.bottom()) < minRecognitionPix) {
		if (qAbs(pos.x() - mSelectedWidRect.right()) < minRecognitionPix) {
			shape =  Qt::SizeFDiagCursor;
			mMouseShape =  5;
		}
		else if (qAbs(pos.x() - mSelectedWidRect.left()) < minRecognitionPix) {
			shape = Qt::SizeBDiagCursor;
			mMouseShape = 7;
		}
	}
	else if (mSelectedWidRect.contains(pos)) {
		shape = Qt::SizeAllCursor;
		mMouseShape = 0;
	}else
		mMouseShape = -1;

	return shape;
}

void bili_area_cap::mousePressEvent(QMouseEvent *e) {

	QPoint mouseRePos = e->pos(); //相对于当前显示器屏幕左上角的坐标（因为是full screen）

	if (e->button() != Qt::LeftButton)
		return QWidget::mousePressEvent(e);

	if (mMouseState == MouseState::Selected){
		mIsPressSelected = true;
		mPosPressSelected = mouseRePos;
		mPosLeftTop = mSelectedWidRect.topLeft();
		if (mMouseShape == 0)
			mAreaTool->show();
		else{
			if (mGetCurPosCursorShape(mouseRePos) != Qt::ArrowCursor) {
				mAreaTool->hide();
				mAreaZoom->show();
			}
		}
	}
	else{
		mPosLeftTop = mouseRePos;
		mPressStartPos = mouseRePos;
        if ((mMouseState & MouseState::Optional) == MouseState::Optional || MouseState::Default == mMouseState) {
            mMouseState = MouseState::Pressed | MouseState::Optional;
            screen_pos_ = QApplication::desktop()->screenGeometry(QCursor::pos()).topLeft();
        } else
            mMouseState = MouseState::Pressed;
            
	}
	QWidget::mousePressEvent(e);
}

void bili_area_cap::mouseMoveEvent(QMouseEvent *e) {
	QPoint mouseAbPos = QCursor::pos();
    QPoint mouseRePos;
    
    if (MouseState::Default == mMouseState) 
        mouseRePos = mouseAbPos - QApplication::desktop()->screenGeometry(mouseAbPos).topLeft();
    else
        mouseRePos = mouseAbPos - screen_pos_;

	//移动放大镜窗口
	mAreaZoom->mMove(mouseRePos, mWidth(), mHeight());
	mAreaZoom->mSetZoomPos(mouseRePos);

	if (mMouseState == MouseState::Selected){
		if (mIsPressSelected) {
			switch (mMouseShape){
				case 0:{
					mMoveSelectRect(mouseRePos);
					mMoveAreaTool();
					break;
				}
				default:{
					mZoomSelectRect(mouseRePos);
					break;
				}
			}
			update();
		}
		else
			setCursor(mGetCurPosCursorShape(mouseRePos));
	}
	else{
		if ((mMouseState&MouseState::Pressed) != MouseState::Pressed){
			mMouseState = MouseState::Move;
			if (mMouseState == MouseState::Move) {
				for (int i = 0; i < WidsRectV.size(); i++){
					if (WidsRectV[i].contains(mouseAbPos.x(), mouseAbPos.y())){
						//获得窗口绝对坐标后，计算窗口矩形相对于所属显示器左上角的坐标
						QRect monitorRect = QApplication::desktop()->screenGeometry(mouseAbPos);
						mSelectedWidRect = WidsRectV[i];
						mSelectedWidRectOrgin = monitorRect.topLeft();
						mSelectedWidRect.moveTo(mSelectedWidRect.topLeft() - monitorRect.topLeft());

						mMouseState = MouseState::Move | MouseState::Optional;
						break;
					}
				}
			}
		}
		else{
			mPosRightBottom = mGetDragBottomRight(mouseRePos);
            mMouseState = MouseState::Pressed | MouseState::Move;
            
		}
		update();
	}

	QWidget::mouseMoveEvent(e);
}

void bili_area_cap::mouseReleaseEvent(QMouseEvent *e) {

	if (e->button() != Qt::LeftButton)
        return QWidget::mouseReleaseEvent(e);

	mIsPressSelected = false;
	if (mMouseState != MouseState::Selected){
		if ((mMouseState & MouseState::Optional) == MouseState::Optional) 
			mMouseState = MouseState::Selected;
		else if ( mMouseState == (MouseState::Pressed | MouseState::Move) ){
			mSelectedWidRect = QRect(mPosLeftTop, mPosRightBottom);
			mMouseState = MouseState::Selected;
		}
		update();
	}
	else{
		mPosLeftTop = mSelectedWidRect.topLeft();
		mPosRightBottom = mSelectedWidRect.bottomRight();
	}
	if (mMouseState == MouseState::Selected){
		mMoveAreaTool();
		mAreaTool->show();
		mAreaZoom->hide();
	}
	QWidget::mouseReleaseEvent(e);
}

void bili_area_cap::mouseDoubleClickEvent(QMouseEvent *e) {

	if (e->button() == Qt::LeftButton) {
		mHasSelected = false;
		if(mMouseState == MouseState::Selected) 
			mHasSelected = true;
		emit mSglSelectComplite(mHasSelected);
		hide();
	}
	QWidget::mouseDoubleClickEvent(e);
}

QPoint bili_area_cap::mGetDragBottomRight(QPoint pos){

	int movePX = pos.x();
	int movePY = pos.y();
	QPoint br = pos;
	if (movePX > mPressStartPos.x()) {
		if (movePY > mPressStartPos.y()){
			br = pos;
		}
		else if (movePY<mPressStartPos.y()){
			mPosLeftTop = QPoint(mPressStartPos.x(), pos.y());
			br = QPoint(pos.x(), mPressStartPos.y());
		}
	}
	else if ( movePX < mPressStartPos.x() ){
		if (movePY > mPressStartPos.y()){
			mPosLeftTop = QPoint(pos.x(), mPressStartPos.y());
			br = QPoint(mPressStartPos.x(), pos.y());
		}
		else if (movePY<mPressStartPos.y()){
			mPosLeftTop = pos;
			br = mPressStartPos;
		}
	}
	if (mPosLeftTop.x() <= 0)
		mPosLeftTop.setX(0);
	if (mPosLeftTop.y() <= 0)
		mPosLeftTop.setY(0);
	if (br.x() >= mWidth())
		br.setX(mWidth());
	if (br.y() >= mHeight())
		br.setY(mHeight());
	return br;
}

void bili_area_cap::mZoomSelectRect(QPoint pos){

	if (mMouseShape == -1)
		return;
	int topY = mPosLeftTop.y();
	int leftX = mPosLeftTop.x();
	int bottomY = mPosRightBottom.y();
	int rightX = mPosRightBottom.x();

	QPoint midPos = mPosLeftTop;
	switch (mMouseShape){
		case 1:{						//top left
			midPos = mPosRightBottom;
			break;
		}
		case 3:{						//top right
			midPos = QPoint(leftX, bottomY);
			break;
		}
		case 5:{						//bottom right
			midPos = mPosLeftTop;
			break;
		}
		case 7:{						//bottom left
			midPos = QPoint(rightX, topY);
			break;
		}
	}

	if (pos.x() > midPos.x()){
		if (pos.y() > midPos.y()){
			mSelectedWidRect.setTopLeft(midPos);
			mSelectedWidRect.setBottomRight(pos);
		}
		else if (pos.y() < midPos.y()) {
			mSelectedWidRect.setTopLeft(QPoint(midPos.x(), pos.y()));
			mSelectedWidRect.setBottomRight(QPoint(pos.x(), midPos.y()));
		}
	}
	else if (pos.x() < midPos.x()){
		if (pos.y() > midPos.y()){
			mSelectedWidRect.setTopLeft(QPoint(pos.x(), midPos.y()));
			mSelectedWidRect.setBottomRight(QPoint(midPos.x(), pos.y()));
		}
		else if (pos.y() < midPos.y()) {
			mSelectedWidRect.setTopLeft(pos);
			mSelectedWidRect.setBottomRight(midPos);
		}
	}
}

void bili_area_cap::mMoveSelectRect(QPoint pos){

	QPoint p = pos - mPosPressSelected;
	QPoint newTopLeft = mPosLeftTop + p;

	if (newTopLeft.x() <= 0)
		newTopLeft.setX(0);
	else if (newTopLeft.x() >= (mWidth() - mSelectedWidRect.width()))
		newTopLeft.setX(mWidth() - mSelectedWidRect.width());
	if (newTopLeft.y() <= 0)
		newTopLeft.setY(0);
	else if (newTopLeft.y() >= (mHeight() - mSelectedWidRect.height()))
		newTopLeft.setY(mHeight() - mSelectedWidRect.height());

	mSelectedWidRect.moveTo(newTopLeft);
}

void bili_area_cap::leaveEvent(QEvent* e)
{
	//拿到鼠标位置
	//根据鼠标位置得知现在在哪个屏幕上
	//移动捕捉窗口到新屏幕
	if (mMouseState & MouseState::Selected)
	{
		QWidget::leaveEvent(e);
		return;
	}

    /*当鼠标点按widget离开时， leave先于release发生*/
    if (MouseState::Move == mMouseState || MouseState::Default == mMouseState || (MouseState::Move | MouseState::Optional) == mMouseState) {
        QPoint mouseAbPos = QCursor::pos();
        QRect currentScreenRect = QApplication::desktop()->screenGeometry(mouseAbPos);
        move(currentScreenRect.topLeft());
        resize(currentScreenRect.size());
        update_current_screen();
    }
	    

	QWidget::leaveEvent(e);
}

void bili_area_cap::update_current_screen()
{
	QPoint mouseAbPos = QCursor::pos();
	mCurrentScreen = QApplication::desktop()->screenNumber(mouseAbPos);
}

const QPixmap& bili_area_cap::mDesktopPixmap() const
{
	return mDesktopPixmaps[mCurrentScreen];
}

int bili_area_cap::mWidth() const
{
	return mWidths[mCurrentScreen];
}

int bili_area_cap::mHeight() const
{
	return mHeights[mCurrentScreen];
}

bool CALLBACK EnumCurWids(HWND hwnd, LPARAM lParam){
	if (IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd)) {

		if (!GetWindowTextLength(hwnd))
			return true;
		TCHAR arr[256] = { 0 };
		GetClassName(hwnd, arr, sizeof(arr) / sizeof(*arr));
		if (!_tcslen(arr))
			return true;
		memset(arr, 0, sizeof(arr));
		GetWindowText(hwnd, arr, sizeof(arr) / sizeof(*arr));
		if (!_tcslen(arr))
			return true;

		RECT curRect;
		GetWindowRect(hwnd, &curRect);
		curRect.left = (curRect.left < BORDER_WIDTH)?BORDER_WIDTH:curRect.left;
		curRect.top = (curRect.top < BORDER_WIDTH)?BORDER_WIDTH:curRect.top;

#if CAP_OUTSCREEN_AREA	
		int *pSize = (int *)lParam;
		int w = *pSize;
		int h = *(pSize + 1);
		curRect.right = (curRect.right > w)?(w-BORDER_WIDTH):curRect.right;
		curRect.bottom = (curRect.bottom > h)?(h-BORDER_WIDTH):curRect.bottom;

		WidsRectV.push_back(QRect(QPoint(curRect.left, curRect.top),
			QPoint(curRect.right, curRect.bottom)));
		WidsHwndV.push_back(hwnd);

		EnumChildWindows(hwnd, (WNDENUMPROC)EnumCurWids, lParam);
#else
		WidsRectV.push_back(QRect(QPoint(curRect.left, curRect.top),
			QPoint(curRect.right, curRect.bottom)));
		WidsHwndV.push_back(hwnd);

		EnumChildWindows(hwnd, (WNDENUMPROC)EnumCurWids, 0);
#endif
	}
	return true;
}

HWND bili_area_cap::mGuessSelectedWindow() const
{
	const int gridSize = 6;

	QRect selectedRect = mSelectedWidRect;
	selectedRect.moveTo(selectedRect.topLeft() + mSelectedWidRectOrgin);

	int windowCount = WidsRectV.size();
	std::vector<int> dotCount(windowCount);
	std::fill(dotCount.begin(), dotCount.end(), 0);

	for (int x = selectedRect.left() + gridSize / 2; x <= selectedRect.right(); x += gridSize)
	{
		for (int y = selectedRect.top() + gridSize / 2; y <= selectedRect.bottom(); y += gridSize)
		{
			QPoint dot(x, y);

			for (int i = 0; i < windowCount; ++i)
			{
				//虽然widrectv里面既有主窗口又有子窗口
				//但是因为主窗口总是在子窗口前面
				//又因为会落在子窗口上的点是会落在主窗口上的
				//所以不存在能在循环中通过主窗口那一次判断而死在子窗口那一次判断的情况
				if (WidsRectV[i].contains(dot))
				{
					++dotCount[i];
					break;
				}
			}
		}
	}

	//找点数最多的
	if (windowCount > 0)
	{
		int i;
		int maxIndex = 0;
		for (i = 1; i < windowCount; ++i)
		{
			if (dotCount[i] > dotCount[maxIndex])
				maxIndex = i;
		}

		return WidsHwndV[maxIndex];
	}
	else
		return 0;
}
