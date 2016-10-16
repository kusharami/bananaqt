#pragma once

#include <QHeaderView>

namespace Banana
{

class CheckBoxHeader : public QHeaderView
{
	Q_OBJECT

public:
	CheckBoxHeader(Qt::Orientation orientation, QWidget* parent = nullptr);

	inline bool isChecked() const;
	void setIsChecked(bool val);

signals:
	void checkBoxStateChanged(bool state);

protected:
	virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
	virtual void mousePressEvent(QMouseEvent* event) override;

private:
	bool checked;
	QRect check_rect;
};

bool CheckBoxHeader::isChecked() const
{
	return checked;
}

}
