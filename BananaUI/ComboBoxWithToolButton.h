#pragma once

#include <QWidget>

class QToolButton;
class QComboBox;
class QtnInplaceInfo;
class QLineEdit;

namespace Banana
{

class ComboBoxWithToolButton : public QWidget
{
public:
	ComboBoxWithToolButton(QWidget *parent = nullptr);

	QLineEdit *getLineEdit() const;
	inline QComboBox *getComboBox() const;
	inline QToolButton *getToolButton() const;

	void applyQtnInplaceInfo(QtnInplaceInfo *inplaceInfo);

private:
	QComboBox *combo;
	QToolButton *toolButton;
};

QComboBox *ComboBoxWithToolButton::getComboBox() const
{
	return combo;
}

QToolButton *ComboBoxWithToolButton::getToolButton() const
{
	return toolButton;
}
}
