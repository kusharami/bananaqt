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
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(inplaceInfo->activationEvent);
			if (qtnAcceptForLineEdit(keyEvent))
			{
				combo->setEditText(keyEvent->text());
				return;
			}
		}
		else
		{
			auto lineEdit = combo->lineEdit();
			Q_ASSERT(nullptr != lineEdit);
			lineEdit->selectAll();
		}
	}
}
}
