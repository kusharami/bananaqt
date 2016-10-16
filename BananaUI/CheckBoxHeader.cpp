#include "CheckBoxHeader.h"

#include <QPainter>
#include <QMouseEvent>

namespace Banana
{

CheckBoxHeader::CheckBoxHeader(Qt::Orientation orientation, QWidget* parent)
	: QHeaderView(orientation, parent)
	, checked(false)
{
}

void CheckBoxHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
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
	if (event->button() == Qt::LeftButton
	&&	check_rect.contains(event->localPos().x(), event->localPos().y(), true))
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
