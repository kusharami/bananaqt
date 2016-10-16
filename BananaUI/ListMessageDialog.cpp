#include "ListMessageDialog.h"

#include <QCoreApplication>

using namespace Banana;
#include "ui_ListMessageDialog.h"

namespace Banana
{

ListMessageDialog::ListMessageDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ListMessageDialog)
	, mType(None)
{
	ui->setupUi(this);

	ui->iconLabel->hide();

	setWindowTitle(QCoreApplication::applicationName());

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint))
				   | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
}

ListMessageDialog::~ListMessageDialog()
{
	delete ui;
}

void ListMessageDialog::setText(const QString &text, Qt::TextFormat format)
{
	ui->label->setTextFormat(format);
	ui->label->setText(text);
}

void ListMessageDialog::setType(Type type)
{
	if (mType != type)
	{
		mType = type;
		switch (type)
		{
			case Information:
			case Warning:
			case Error:
			case Question:
				ui->iconLabel->setPixmap(QMessageBox::standardIcon((QMessageBox::Icon) type));
				ui->iconLabel->show();
				break;

			default:
				ui->iconLabel->hide();
				break;
		}
	}
}

void ListMessageDialog::addListItem(const QString &text, const QString &hint)
{
	auto item = new QListWidgetItem(ui->listWidget);
	item->setText(text);
	item->setToolTip(hint);
}
}
