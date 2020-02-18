/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

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

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint)) |
		Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
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
				ui->iconLabel->setPixmap(
					QMessageBox::standardIcon((QMessageBox ::Icon) type));
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
