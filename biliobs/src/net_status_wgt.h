#ifndef NET_STATUS_WGT_H
#define NET_STATUS_WGT_H

#include <QWidget>
class PixReplaceMoveOpt;
class QPixmap;
class QLabel;

namespace Ui {
	class NetStatusWgt;
}

class NetStatusWgt : public QWidget
{
	Q_OBJECT

public:
	enum Position {
		LEFT = 0,
		RIGHT,
	};

	enum NetState {
		NETWORK_GOOD,
		NETWORK_BAD,
		NETWORK_BROKEN
	};


public:
	explicit NetStatusWgt(QWidget *parent = 0);
	~NetStatusWgt();

	void setPosition(Position pos);
	Position position() const;

	void setDesktop(int index);
	int desktop() const;

	void setHeaderVisible(bool show);
	bool headerVisble() const;

	QPoint originalPos_;

public slots:
	void setNetState(NetState);
	void setNetUpSpeed(int speed);
	void setFrameLostRate(float rate);

public slots:
	void setNumOfViewers(int num);
private:
	void displayViewerNum(int num);
	int viewer_num_;
	public slots:
	void setNumberOfPeopleConcerned(int num);
private:
	void displayFansNum(int num);
	int fans_num_;
private:
	bool is_viewers_num_;

private:
	void paintEvent(QPaintEvent *e) override;

	void timerEvent(QTimerEvent *e) override;

private:
	void updatePosition();

private:
	Ui::NetStatusWgt *ui;

private:
	void startMovie();
	void doOnEndMovie();
	private slots:
	void onMovieFinish();
	void onPixTransform(QPixmap pix);
private:
	PixReplaceMoveOpt *movie_op_;
	QLabel *pix_lbl_;
	QLabel *pix_tip_lbl_;
	QLabel *pix_num_;

private:
	Position position_;
	int desktop_index_;

	bool has_header_;
};

#endif // NET_STATUS_WGT_H
