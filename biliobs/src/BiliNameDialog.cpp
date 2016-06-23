#include "BiliNameDialog.hpp"
#include "ui_BiliNameDialog.h"

BiliNameDialog::BiliNameDialog(QString& name, QWidget* parent) :
	QDialog(parent),
	biliUi(new Ui::BiliNameDialog),
	nameString(name)
{
	this->setWindowFlags(Qt::FramelessWindowHint);
	biliUi->setupUi(this);
	biliUi->okButton->setDefault(true);

	biliUi->nameEdit->setText(nameString);
}

BiliNameDialog::~BiliNameDialog()
{
	delete biliUi;
}

void BiliNameDialog::on_okButton_clicked()
{
	nameString = biliUi->nameEdit->text();
	done(QDialog::Accepted);
}

void BiliNameDialog::on_cancelButton_clicked()
{
	done(QDialog::Rejected);
}

void BiliNameDialog::on_xButton_clicked()
{
	on_cancelButton_clicked();
}

void BiliNameDialog::mousePressEvent(QMouseEvent* e)
{
	moveWindowUtil.mousePressEvent(this, biliUi->titleFrame->contentsRect(), e);
}

void BiliNameDialog::mouseReleaseEvent(QMouseEvent* e)
{
	moveWindowUtil.mouseReleaseEvent(this, e);
}

void BiliNameDialog::mouseMoveEvent(QMouseEvent* e)
{
	moveWindowUtil.mouseMoveEvent(this, e);
}
