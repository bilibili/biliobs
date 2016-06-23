#ifndef BILI_MOVE_FRAMELESS_WINDOW
#define BILI_MOVE_FRAMELESS_WINDOW

#include <qevent.h>

template<class QWindowType>
class MovableFramelessWindowUtil
{
public:
	MovableFramelessWindowUtil()
		: isInMoving(false)
	{
	}

	virtual ~MovableFramelessWindowUtil()
	{
	}

	void mousePressEvent(QWindowType* window, const QRect& titleRect, QMouseEvent* e)
	{
		window;
		e;

		int relativeX = e->x();
		int relativeY = e->y();

		int titleX, titleY, titleH, titleW;

		titleRect.getRect(&titleX, &titleY, &titleW, &titleH);

		//if (relativeX > titleX && relativeX < titleX + titleW && relativeY > titleY && relativeY < titleY + titleH)
		//{
			moveMouseStartX = e->globalX();
			moveMouseStartY = e->globalY();
			moveWindowStartX = window->pos().x();
			moveWindowStartY = window->pos().y();
			isInMoving = true;
		//}
	}

	void mouseReleaseEvent(QWindowType* window, QMouseEvent* e)
	{
		window;
		e;

		isInMoving = false;
	}

	void mouseMoveEvent(QWindowType* window, QMouseEvent* e)
	{
		window;
		e;

		if (isInMoving)
		{
			int curX = e->globalX();
			int curY = e->globalY();

			int offsetX = curX - moveMouseStartX;
			int offsetY = curY - moveMouseStartY;

			int destX = moveWindowStartX + offsetX;
			int destY = moveWindowStartY + offsetY;

			window->move(destX, destY);
		}
	}
private:
	bool isInMoving;
	int moveMouseStartX;
	int moveMouseStartY;
	int moveWindowStartX;
	int moveWindowStartY;

private:
	MovableFramelessWindowUtil(const MovableFramelessWindowUtil&);
	MovableFramelessWindowUtil& operator =(const MovableFramelessWindowUtil&);
};

#endif
