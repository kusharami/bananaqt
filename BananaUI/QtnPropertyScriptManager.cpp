/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "QtnPropertyScriptManager.h"

#include "QtnProperty/Delegates/PropertyEditorHandler.h"
#include "QtnProperty/Delegates/PropertyEditorAux.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include "QtnProperty/QObjectPropertySet.h"

#include "BananaScript/ScriptManager.h"

#include "ScriptManagerDialog.h"

namespace Banana
{
class QtnPropertyScriptManagerButtonHandler
	: public QtnPropertyEditorBttnHandler<QtnPropertyScriptManager,
		  QtnLineEditBttn>
{
	QtnPropertyScriptManager &m_prop;

public:
	QtnPropertyScriptManagerButtonHandler(
		QtnPropertyScriptManager &property, QtnLineEditBttn &editor);

protected:
	virtual void onToolButtonClick() override;
	virtual void updateEditor() override;

private:
	void onToolButtonClicked(bool);
};

QtnPropertyScriptManager::QtnPropertyScriptManager(
	QObject *object, const QMetaProperty &metaProperty)
	: QtnProperty(nullptr)
	, object(object)
	, metaProperty(metaProperty)
{
}

ScriptManager *QtnPropertyScriptManager::getScriptManager() const
{
	auto var = metaProperty.read(object);
	if (var.type() == QVariant::UserType &&
		var.userType() == qMetaTypeId<ScriptManager *>())
	{
		return var.value<ScriptManager *>();
	}

	return nullptr;
}

QString QtnPropertyScriptManager::getPlaceholderStr()
{
	return tr("(Scripted Actions)");
}

void QtnPropertyScriptManager::Register()
{
	qtnRegisterMetaPropertyFactory(qMetaTypeId<ScriptManager *>(),
		[](QObject *object,
			const QMetaProperty &metaProperty) -> QtnProperty * {
			return new QtnPropertyScriptManager(object, metaProperty);
		});

	QtnPropertyDelegateFactory::staticInstance().registerDelegateDefault(
		&QtnPropertyScriptManager::staticMetaObject,
		&qtnCreateDelegate<QtnPropertyDelegateScriptManager,
			QtnPropertyScriptManager>);
}

QtnPropertyDelegateScriptManager::QtnPropertyDelegateScriptManager(
	QtnPropertyScriptManager &owner)
	: QtnPropertyDelegateTyped<QtnPropertyScriptManager>(owner)
{
}

void QtnPropertyDelegateScriptManager::drawValueImpl(QStylePainter &painter,
	const QRect &rect, const QStyle::State &state, bool *needTooltip) const
{
	QPen oldPen = painter.pen();
	painter.setPen(Qt::darkGray);

	QtnPropertyDelegateTyped<QtnPropertyScriptManager>::drawValueImpl(
		painter, rect, state, needTooltip);
	painter.setPen(oldPen);
}

QWidget *QtnPropertyDelegateScriptManager::createValueEditorImpl(
	QWidget *parent, const QRect &rect, QtnInplaceInfo *inplaceInfo)
{
	auto editor = new QtnLineEditBttn(parent);
	editor->setGeometry(rect);

	new QtnPropertyScriptManagerButtonHandler(
		*static_cast<QtnPropertyScriptManager *>(&owner()), *editor);

	qtnInitLineEdit(editor->lineEdit, inplaceInfo);

	return editor;
}

bool QtnPropertyDelegateScriptManager::propertyValueToStr(
	QString &strValue) const
{
	strValue = QtnPropertyScriptManager::getPlaceholderStr();
	return true;
}

QtnPropertyScriptManagerButtonHandler::QtnPropertyScriptManagerButtonHandler(
	QtnPropertyScriptManager &property, QtnLineEditBttn &editor)
	: QtnPropertyEditorHandlerType(property, editor)
	, m_prop(property)
{
	editor.lineEdit->setReadOnly(true);
	editor.toolButton->setEnabled(true);

	updateEditor();

	editor.lineEdit->installEventFilter(this);
	QObject::connect(editor.toolButton, &QToolButton::clicked, this,
		&QtnPropertyScriptManagerButtonHandler::onToolButtonClicked);
}

void QtnPropertyScriptManagerButtonHandler::onToolButtonClick()
{
	onToolButtonClicked(false);
}

void QtnPropertyScriptManagerButtonHandler::updateEditor()
{
	auto edit = editor().lineEdit;
	edit->setText(QString());
	edit->setPlaceholderText(QtnPropertyScriptManager::getPlaceholderStr());
}

void QtnPropertyScriptManagerButtonHandler::onToolButtonClicked(bool)
{
	auto scriptManager = m_prop.getScriptManager();
	Q_ASSERT(nullptr != scriptManager);

	auto dialog = new ScriptManagerDialog(scriptManager, editorBase());
	auto dialogContainer = connectDialog(dialog);

	dialog->show();
	dialog->raise();
	dialog->exec();

	Q_UNUSED(dialogContainer);
}
}
