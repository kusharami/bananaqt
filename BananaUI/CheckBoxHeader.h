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
