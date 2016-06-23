#ifndef TASK_NS_H
#define TASK_NS_H
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QPoint>
#include <QRect>

namespace TKNS {

	enum DMTaskState{
		ADD_STATE = 1,
		SUB_STATE = 2,
		SHOW_STATE = 3,
		HIDE_STATE = 4
	};

	struct DMTask{
		uint dmIndex_;
		QString dmStr_;
		QPixmap *dmPix_;
		QPixmap dmPixBackup_;
		QSize dmPixSize_;
		QPoint dmCurCutPos_;
		QRect dmForDrawRect_;
		float dmOpacity_;
		DMTaskState dmTState_;
		DMTask() :dmStr_(QString())
			, dmCurCutPos_(QPoint(0, 0))
			, dmOpacity_(1.0)
			, dmTState_(DMTaskState::ADD_STATE)
			, dmPix_(nullptr){
		}
		//~DMTask(){
		//	delete dmPix_;
		//	dmPix_ = nullptr;
		//}
	};
}
#endif // TASK_NS_H