#include "net_status_wgt.h"
#include "ui_net_status_wgt.h"

#include <QPainter>
#include <QPainterPath>

#include <Windows.h>

#include <QDesktopWidget>

#include "pix_replace_move_opt.h"
#include <qpainter.h>

#include <qdebug.h>

NetStatusWgt::NetStatusWgt(QWidget *parent) :
QWidget(parent),
ui(new Ui::NetStatusWgt),

position_(RIGHT),
desktop_index_(0)
{
	ui->setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_TranslucentBackground);
	//setAttribute(Qt::WA_TransparentForMouseEvents);

	HWND hwnd = (HWND)winId();
	LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
	SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT);

	updatePosition();
	originalPos_ = pos();

	has_header_ = false;

	movie_op_ = new PixReplaceMoveOpt(this);
	connect(movie_op_, &PixReplaceMoveOpt::onTransform, this, &NetStatusWgt::onPixTransform);
	connect(movie_op_, &PixReplaceMoveOpt::workFinished, this, &NetStatusWgt::onMovieFinish);

	/*10s for change*/
	startTimer(10000);
	pix_lbl_ = new QLabel(this);
	pix_lbl_->setGeometry(274, 68, 118, 19);
	pix_lbl_->hide();
	pix_lbl_->setStyleSheet("background:rgba(43, 43, 43, 255)");

	pix_tip_lbl_ = new QLabel(pix_lbl_);
	pix_tip_lbl_->setGeometry(6, 2, 36, 16);
	pix_tip_lbl_->setStyleSheet("QLabel { \
								    font-family: \"Microsoft YaHei\"; \
									font-size: 10pt; \
									color: #FFFFFF; \
																																                               }");
	pix_tip_lbl_->setAlignment(Qt::AlignCenter);

	pix_num_ = new QLabel(pix_lbl_);
	pix_num_->setGeometry(37, 2, 83, 16);
	pix_num_->setStyleSheet("QLabel { \
							    font-family: \"Microsoft YaHei\"; \
								font-size: 10pt; \
								color: #FFFFFF; \
							 }");
	pix_num_->setAlignment(Qt::AlignCenter);

	is_viewers_num_ = true;
}

NetStatusWgt::~NetStatusWgt()
{
	delete ui;
}

void NetStatusWgt::setPosition(NetStatusWgt::Position pos)
{
	if (pos != position_) {
		position_ = pos;

		updatePosition();
	}
}
NetStatusWgt::Position NetStatusWgt::position() const
{
	return position_;
}

void NetStatusWgt::setDesktop(int index)
{

	if (desktop_index_ != index) {
		desktop_index_ = index;

		updatePosition();
	}

}

int NetStatusWgt::desktop() const
{
	return desktop_index_;
}

void NetStatusWgt::setHeaderVisible(bool show)
{
	has_header_ = show;


	ui->header_text->setVisible(has_header_);

	update();
}
bool NetStatusWgt::headerVisble() const
{
	return has_header_;
}

void NetStatusWgt::setNetState(NetState state)
{
	switch (state)
	{
	case NetStatusWgt::NETWORK_GOOD:
		ui->color_lbl->setStyleSheet("background: rgb(73, 195, 32);");
		break;
	case NetStatusWgt::NETWORK_BAD:
		ui->color_lbl->setStyleSheet("background: red;");
		break;
	case NetStatusWgt::NETWORK_BROKEN:
		ui->color_lbl->setStyleSheet("background: gray;");
		break;
	default:
		ui->color_lbl->setStyleSheet("background: gray;");
		break;
	}
}

void NetStatusWgt::setNetUpSpeed(int speed)
{
	QString str;
	if (0 > speed) {
		str = QString("");
	}
	else {
		str = QString::number(speed);
	}

	str += "kbps";

	ui->net_up_speed_display->setText(str);
}

void NetStatusWgt::setFrameLostRate(float rate)
{
	QString str;
	if (0 > rate) {
		str = QString("--");
	}
	else {
		if (rate > 100)
			rate = 100;

		int tmp;

		if ((int)rate) {

			str = QString::number((int)rate);
			str += QString(".");

			tmp = (int)((rate - (int)rate) * 100);



		}
		else {
			str = QString("0.");

			tmp = (int)(rate * 100);
		}

		if (tmp < 10)
			str += QString::number(0) + QString::number(tmp);
		else
			str += QString::number(tmp);

		str += "%";
	}



	ui->frame_lose_rate_display->setText(str);
}

void NetStatusWgt::setNumOfViewers(int num)
{
	viewer_num_ = num;

	if (is_viewers_num_)
		displayViewerNum(viewer_num_);

	//ui->viewer_num_display->setText(str);
}

void NetStatusWgt::displayViewerNum(int num)
{
	QString str;
	if (0 > num) {
		str = QString("");
	}
	else {
		str = QString::number(num);
	}
	str += tr("persons");


	ui->viewer_num_display->setText(str);
	ui->viewer_num_lbl->setText(tr("audience:"));
}

void NetStatusWgt::setNumberOfPeopleConcerned(int num)
{
	fans_num_ = num;

	if (!is_viewers_num_)
		displayFansNum(fans_num_);



	//ui->focus_num_display->setText(str);
}

void NetStatusWgt::displayFansNum(int num)
{
	QString str;
	if (0 > num) {
		str = QString("");
	}
	else {
		str = QString::number(num);
	}

	ui->viewer_num_display->setText(str);
	ui->viewer_num_lbl->setText(tr("fans:"));
}

void NetStatusWgt::paintEvent(QPaintEvent *e)
{

	const int rect_diameter = 8;

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	if (has_header_) {
		painter.fillRect(0, 0, 406, 60, QColor(43, 43, 43, 204));
	}

	painter.fillRect(0, 60, 406, 38, QColor(43, 43, 43, 255));


	//    painter.fillRect(0, 0,  10, 10, QColor(0, 0, 0, 100));


	return;

	/*以下为添加圆角*/

	painter.setCompositionMode(QPainter::CompositionMode_Clear);
	/*left top*/
	QPainterPath clear_path(QPointF(0, 0));
	clear_path.arcTo(0,
		0,
		rect_diameter,
		rect_diameter,
		90,
		90);

	/*right top*/
	clear_path.moveTo(width(), 0);
	clear_path.arcTo(width() - rect_diameter,
		0,
		rect_diameter,
		rect_diameter,
		0,
		90);

	/*right bottom*/
	clear_path.moveTo(width(), height());
	clear_path.arcTo(width() - rect_diameter,
		height() - rect_diameter,
		rect_diameter, rect_diameter,
		-90,
		90);

	/*left bottom*/
	clear_path.moveTo(0, height());
	clear_path.arcTo(0,
		height() - rect_diameter,
		rect_diameter,
		rect_diameter,
		180,
		90);

	painter.fillPath(clear_path, Qt::white);

	//SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void NetStatusWgt::timerEvent(QTimerEvent *e)
{
	if (isVisible()) {
		startMovie();
	}
}


void NetStatusWgt::updatePosition()
{
	QDesktopWidget *desktop = QApplication::desktop();

	QRect rect = desktop->availableGeometry(desktop_index_);

	if (LEFT == position_)
		move(rect.bottomLeft() + QPoint(0, -98));
	else
		move(rect.right() - width(),
		rect.bottom() - 98);
}



void NetStatusWgt::startMovie()
{

	ui->viewer_num_display->hide();
	ui->viewer_num_lbl->hide();

	QPixmap pix_old(pix_lbl_->size());
	pix_old.fill(Qt::transparent);
	QPixmap pix_replace(pix_lbl_->size());
	pix_replace.fill(Qt::transparent);

	pix_tip_lbl_->show();
	pix_tip_lbl_->setText(ui->viewer_num_lbl->text());
	pix_num_->show();
	pix_num_->setText(ui->viewer_num_display->text());
	pix_lbl_->render(&pix_old, QPoint(0, 0), pix_lbl_->rect());

	is_viewers_num_ = !is_viewers_num_;
	if (is_viewers_num_)
		displayViewerNum(viewer_num_);
	else
		displayFansNum(fans_num_);


	pix_tip_lbl_->setText(ui->viewer_num_lbl->text());
	pix_num_->setText(ui->viewer_num_display->text());
	pix_lbl_->render(&pix_replace, QPoint(0, 0), pix_lbl_->rect());

	//static QLabel *ll = new QLabel();
	//ll->show();
	//ll->setPixmap(pix_old);

	qDebug() << pix_lbl_->geometry();

	pix_lbl_->setPixmap(pix_old);

	movie_op_->setPixmaps(pix_old, pix_replace);

	pix_tip_lbl_->hide();
	pix_num_->hide();
	pix_lbl_->show();
	movie_op_->startMovie();
}

void NetStatusWgt::doOnEndMovie()
{
	pix_lbl_->hide();

	ui->viewer_num_display->show();
	ui->viewer_num_lbl->show();


}

void NetStatusWgt::onMovieFinish()
{
	doOnEndMovie();
}
void NetStatusWgt::onPixTransform(QPixmap pix)
{
	if (!isVisible()) {
		movie_op_->endMovie();

		doOnEndMovie();
	}
	else {
		//static QLabel *ll = new QLabel();
		//ll->show();

		//ll->setPixmap(pix);

		pix_lbl_->setPixmap(pix);
	}
}