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

#pragma once

#include <QDialog>
#include <QMessageBox>

namespace Ui
{
class ListMessageDialog;
}

namespace Banana
{

class ListMessageDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ListMessageDialog(QWidget *parent = nullptr);
	virtual ~ListMessageDialog();

	enum Type
	{
		None = QMessageBox::NoIcon,
		Information = QMessageBox::Information,
		Warning = QMessageBox::Warning,
		Error = QMessageBox::Critical,
		Question = QMessageBox::Question
	};

	void setText(const QString &text, Qt::TextFormat format = Qt::PlainText);

	inline Type type() const;
	void setType(Type type);

	void addListItem(const QString &text, const QString &hint);

private:
	Ui::ListMessageDialog *ui;

	Type mType;
};

ListMessageDialog::Type ListMessageDialog::type() const
{
	return mType;
}

}
