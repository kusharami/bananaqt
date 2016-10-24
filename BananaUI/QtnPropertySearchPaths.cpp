/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "QtnPropertySearchPaths.h"

#include "QtnProperty/Delegates/PropertyEditorHandler.h"
#include "QtnProperty/Delegates/PropertyEditorAux.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"

#include "QtnProperty/QObjectPropertySet.h"

#include "SearchPathsDialog.h"
#include "ProjectDirectoryModel.h"

#include "BananaCore/ProjectGroup.h"
#include "BananaCore/IProjectGroupDelegate.h"
#include "BananaCore/SearchPaths.h"

namespace Banana
{

	QtnPropertySearchPaths::QtnPropertySearchPaths(QObject *object,
												   const QMetaProperty &metaProperty)
		: QtnProperty(nullptr)
		, object(object)
		, metaProperty(metaProperty)
	{

	}

	SearchPaths *QtnPropertySearchPaths::getSearchPaths() const
	{
		auto var = metaProperty.read(object);
		if (var.type() == QVariant::UserType
		&&	var.userType() == qMetaTypeId<SearchPaths *>())
		{
			return var.value<SearchPaths *>();
		}

		return nullptr;
	}

	QString QtnPropertySearchPaths::getPlaceholderStr()
	{
		return tr("(Search Paths)");
	}

	void QtnPropertySearchPaths::Register()
	{
		qtnRegisterMetaPropertyFactory(qMetaTypeId<SearchPaths *>(),
		[](QObject *object, const QMetaProperty &metaProperty) -> QtnProperty *
		{
			return new QtnPropertySearchPaths(object, metaProperty);
		});

		QtnPropertyDelegateFactory::staticInstance()
			.registerDelegateDefault(&QtnPropertySearchPaths::staticMetaObject
			, &qtnCreateDelegate<QtnPropertyDelegateSearchPaths,
							QtnPropertySearchPaths>);
	}

	class QtnPropertySearchPathsButtonHandler
		: public QtnPropertyEditorBttnHandler<QtnPropertySearchPaths, QtnLineEditBttn>
	{
	public:
		QtnPropertySearchPathsButtonHandler(QtnPropertySearchPaths &property,
											QtnLineEditBttn &editor)
			: QtnPropertyEditorHandlerType(property, editor)
			, m_prop(property)
		{
			editor.lineEdit->setReadOnly(true);
			editor.toolButton->setEnabled(true);

			updateEditor();

			editor.lineEdit->installEventFilter(this);
			QObject::connect(editor.toolButton, &QToolButton::clicked,
				this, &QtnPropertySearchPathsButtonHandler::onToolButtonClicked);

		}

	protected:
		virtual void onToolButtonClick() override { onToolButtonClicked(false); }
		virtual void updateEditor() override
		{
			auto edit = editor().lineEdit;
			edit->setText(QString());
			edit->setPlaceholderText(QtnPropertySearchPaths::getPlaceholderStr());
		}

	private:
		void onToolButtonClicked(bool)
		{
			auto search_paths = m_prop.getSearchPaths();
			Q_ASSERT(nullptr != search_paths);

			auto project_dir = search_paths->getProjectDirectory();
			Q_ASSERT(nullptr != project_dir);

			auto project_group = project_dir->getProjectGroup();
			Q_ASSERT(nullptr != project_group);

			auto delegate = project_group->getDelegate();
			Q_ASSERT(nullptr != delegate);

			auto dialog = new SearchPathsDialog(delegate->getProjectTreeModel(), editorBase());
			auto dialogContainer = connectDialog(dialog);

			dialog->show();
			dialog->raise();
			dialog->exec();

			Q_UNUSED(dialogContainer);
		}

		QtnPropertySearchPaths& m_prop;
	};

	QtnPropertyDelegateSearchPaths::QtnPropertyDelegateSearchPaths(QtnPropertySearchPaths &owner)
		: QtnPropertyDelegateTyped<QtnPropertySearchPaths>(owner)
	{

	}

	void QtnPropertyDelegateSearchPaths::drawValueImpl(QStylePainter &painter,
														const QRect &rect,
														const QStyle::State &state,
														bool *needTooltip) const
	{
		QPen oldPen = painter.pen();
		painter.setPen(Qt::darkGray);

		QtnPropertyDelegateTyped<QtnPropertySearchPaths>::drawValueImpl(painter, rect, state, needTooltip);
		painter.setPen(oldPen);
	}

	QWidget *QtnPropertyDelegateSearchPaths::createValueEditorImpl(QWidget *parent,
																	const QRect &rect,
																	QtnInplaceInfo *inplaceInfo)
	{
		auto editor = new QtnLineEditBttn(parent);
		editor->setGeometry(rect);

		new QtnPropertySearchPathsButtonHandler(
			*static_cast<QtnPropertySearchPaths *>(&owner()), *editor);

		qtnInitLineEdit(editor->lineEdit, inplaceInfo);

		return editor;
	}

	bool QtnPropertyDelegateSearchPaths::propertyValueToStr(QString &strValue) const
	{
		strValue = QtnPropertySearchPaths::getPlaceholderStr();
		return true;
	}


}
