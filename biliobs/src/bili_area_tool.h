#ifndef BILI_AREA_TOOL_H
#define BILI_AREA_TOOL_H

#include <QWidget>
#include "ui_bili_area_tool.h"

#include <functional>

class bili_area_zoom : public QWidget{

	Q_OBJECT
public:
	bili_area_zoom(std::function<const QPixmap&()>&& bkPix, QWidget *parent = 0);
	~bili_area_zoom();

	void mSetZoomPos(QPoint pos);
	void mMove(QPoint pos, int w, int h);

	std::function<const QPixmap&()> mBKPix;
protected:
	virtual void paintEvent(QPaintEvent *e);

private:
	QPixmap mPaintPix;
};


class bili_area_tool : public QWidget
{
	Q_OBJECT

public:
	bili_area_tool(QWidget *parent = 0);
	~bili_area_tool();

signals:
	void mSglBtnClicked(int btnId);

private:
	Ui::bili_area_tool ui;

};

#endif // BILI_AREA_TOOL_H
