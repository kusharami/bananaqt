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

#include "DetailedMessageDialog.h"

#include <QCoreApplication>
#include <QAbstractButton>

using namespace Banana;
#include "ui_DetailedMessageDialog.h"

namespace Banana
{

	DetailedMessageDialog::DetailedMessageDialog(QWidget *parent)
		: QDialog(parent)
		, ui(new Ui::DetailedMessageDialog)
		, mLastClickedButton(nullptr)
		, mType(None)
	{
		ui->setupUi(this);

		ui->iconLabel->hide();
		ui->detailsText->setReadOnly(true);

		setWindowTitle(QCoreApplication::applicationName());

		setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint))
					   | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);

		QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked,
						 this, &DetailedMessageDialog::onButtonClicked);
	}

	DetailedMessageDialog::~DetailedMessageDialog()
	{
		delete ui;
	}

	void DetailedMessageDialog::setMessage(const QString &text, Qt::TextFormat format)
	{
		ui->label->setTextFormat(format);
		ui->label->setText(text);
	}

	void DetailedMessageDialog::setDetailsText(const QString &text)
	{
		ui->detailsText->setPlainText(text);
	}

	void DetailedMessageDialog::setButtons(const StandardButtons &buttons)
	{
		ui->buttonBox->setStandardButtons(buttons);
	}

	void DetailedMessageDialog::setType(Type type)
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

	QDialogButtonBox *DetailedMessageDialog::buttonBox() const
	{
		return ui->buttonBox;
	}

	DetailedMessageDialog::StandardButton
	DetailedMessageDialog::standardButton(QAbstractButton *button) const
	{
		return ui->buttonBox->standardButton(button);
	}

	QAbstractButton *DetailedMessageDialog::clickedButton() const
	{
		return mLastClickedButton;
	}

	void DetailedMessageDialog::onButtonClicked(QAbstractButton *button)
	{
		mLastClickedButton = button;
		if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
			accept();
		else
			reject();
	}

}
