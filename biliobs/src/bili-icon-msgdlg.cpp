#include "bili-icon-msgdlg.h"
#include <QWidget>
#include <QDialog>
#include <QPixmap>
#include <QEvent>
#include <QPainter>
#include <QColor>
#include <assert.h>

#include "bili_move_frameless_window.hpp"

BiLiIconMsgDlg::BiLiIconMsgDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);

	ui.setupUi(this);

	QObject::connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancelButtonClicked()));
	QObject::connect(ui.xButton, SIGNAL(clicked()), this, SLOT(onCancelButtonClicked()));
}

BiLiIconMsgDlg::~BiLiIconMsgDlg()
{

}

void BiLiIconMsgDlg::paintEvent(QPaintEvent* pe)
{
	QPainter painter(this);
	QRect rect = geometry();
	
	int alphas[] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

	for (int i = 0; i < sizeof(alphas) / sizeof(*alphas); ++i)
	{
		QPainterPath radiusRect;
		radiusRect.addRoundRect(i, i, rect.width() - i * 2, rect.height() - i * 2, sizeof(alphas) / sizeof(*alphas));
		painter.fillPath(radiusRect, QColor(0, 0, 0, alphas[i]));
	}
	QDialog::paintEvent(pe);
}

void BiLiIconMsgDlg::mousePressEvent(QMouseEvent* e)
{
	moveHelper.mousePressEvent(this, ui.dlgWidget->contentsRect(), e);
}

void BiLiIconMsgDlg::mouseReleaseEvent(QMouseEvent* e)
{
	moveHelper.mouseReleaseEvent(this, e);
}

void BiLiIconMsgDlg::mouseMoveEvent(QMouseEvent* e)
{
	moveHelper.mouseMoveEvent(this, e);
}

void BiLiIconMsgDlg::SetOkButtonText(const QString& text)
{
	ui.okButton->setText(text);
	ui.okButton->adjustSize();
}

void BiLiIconMsgDlg::SetCancelButtonText(const QString& text)
{
	if (text.isEmpty())
	{
		ui.cancelButton->setHidden(true);
		//ui.buttonsLayout->removeWidget(ui.cancelButton);
		ui.buttonsLayout->removeItem(ui.betweenButtonSpacer);
	}
	else
	{
		if (ui.cancelButton->isHidden())
		{
			ui.buttonsLayout->insertSpacerItem(2, ui.betweenButtonSpacer);
		}
		ui.cancelButton->setText(text);
		ui.cancelButton->adjustSize();
		ui.cancelButton->setHidden(false);
	}
}

void BiLiIconMsgDlg::SetLargeIcon(const QString& url)
{
	if (url.isEmpty())
	{
		ui.iconLabel->hide();
		ui.subTitleLabel->setAlignment(Qt::AlignHCenter);
		ui.contentLabel->setAlignment(Qt::AlignHCenter);
	}
	else
	{
		SetLargeIcon(QPixmap(url));
	}
}

void BiLiIconMsgDlg::SetLargeIcon(const QPixmap& largeIcon)
{
	ui.iconLabel->setPixmap(largeIcon);
	ui.subTitleLabel->setAlignment(Qt::AlignLeft);
	ui.contentLabel->setAlignment(Qt::AlignLeft);
	ui.iconLabel->show();
}

void BiLiIconMsgDlg::setWindowTitle(const QString& title)
{
	QWidget::setWindowTitle(title);
	ui.titleLabel->setText(title);
	ui.titleLabel->adjustSize();
}

void BiLiIconMsgDlg::SetSubTitle(const QString& title)
{
	if (title.isEmpty())
	{
		ui.subTitleLabel->hide();
		ui.contentLayout->removeWidget(ui.subTitleLabel);
	}
	else
	{
		ui.subTitleLabel->setText(title);
		ui.subTitleLabel->adjustSize();
	}
}

void BiLiIconMsgDlg::SetText(const QString& text)
{
	ui.contentLabel->setText(text);
	ui.contentLabel->adjustSize();
}

void BiLiIconMsgDlg::onOkButtonClicked()
{
	done(QDialog::Accepted);
}

void BiLiIconMsgDlg::onCancelButtonClicked()
{
	done(QDialog::Rejected);
}
