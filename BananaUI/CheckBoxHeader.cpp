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

#include "CheckBoxHeader.h"

#include <QPainter>
#include <QMouseEvent>

namespace Banana
{
CheckBoxHeader::CheckBoxHeader(Qt::Orientation orientation, QWidget *parent)
	: QHeaderView(orientation, parent)
	, checked(false)
{
}

void CheckBoxHeader::paintSection(
	QPainter *painter, const QRect &rect, int logicalIndex) const
{
	painter->save();
	QHeaderView::paintSection(painter, rect, logicalIndex);
	painter->restore();
	if (logicalIndex == 0)
	{
		auto thiz = const_cast<CheckBoxHeader *>(this);
		thiz->check_rect = QRect(1, (rect.height() - 20) / 2, 20, 20);

		QStyleOptionButton option;

		option.rect = check_rect;

		option.state = QStyle::State_Enabled | QStyle::State_Active;

		if (checked)
			option.state |= QStyle::State_On;
		else
			option.state |= QStyle::State_Off;

		option.state |= QStyle::State_Off;

		style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
	}
}

void CheckBoxHeader::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton &&
		check_rect.contains(event->localPos().x(), event->localPos().y(), true))
	{
		setIsChecked(!isChecked());
	}
}

void CheckBoxHeader::setIsChecked(bool val)
{
	if (checked != val)
	{
		checked = val;

		viewport()->update();

		emit checkBoxStateChanged(checked);
	}
}
}
