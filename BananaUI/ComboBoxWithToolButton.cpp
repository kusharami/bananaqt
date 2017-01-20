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

#include "ComboBoxWithToolButton.h"

#include "QtnProperty/Delegates/PropertyDelegate.h"
#include "QtnProperty/Delegates/PropertyEditorAux.h"

#include <QComboBox>
#include <QToolButton>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QAbstractItemView>

namespace Banana
{

class ComboBox : public QComboBox
{
public:
	ComboBox(QWidget *parent)
		: QComboBox(parent)
	{

	}

	virtual void showPopup() override
	{
		view()->setMinimumWidth(view()->sizeHintForColumn(0));
		QComboBox::showPopup();
	}

};

ComboBoxWithToolButton::ComboBoxWithToolButton(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	combo = new ComboBox(this);
	layout->addWidget(combo);

	toolButton = new QToolButton(this);
	toolButton->setText("...");
	toolButton->setFocusPolicy(Qt::StrongFocus);
	layout->addWidget(toolButton);

	setFocusProxy(combo);
	setAutoFillBackground(true);
}

QLineEdit *ComboBoxWithToolButton::getLineEdit() const
{
	return combo->lineEdit();
}

void ComboBoxWithToolButton::applyQtnInplaceInfo(QtnInplaceInfo *inplaceInfo)
{
	if (nullptr == inplaceInfo)
		return;

	if (combo->isEditable())
	{
		if (inplaceInfo->activationEvent->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent =
				static_cast<QKeyEvent *>(inplaceInfo->activationEvent);
			if (qtnAcceptForLineEdit(keyEvent))
			{
				combo->setEditText(keyEvent->text());
				return;
			}
		} else
		{
			auto lineEdit = combo->lineEdit();
			Q_ASSERT(nullptr != lineEdit);
			lineEdit->selectAll();
		}
	}
}

}
