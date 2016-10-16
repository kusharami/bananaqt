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

			SearchPathsDialog dlg(delegate->getProjectTreeModel(),
								  dynamic_cast<QWidget *>(editorBase()->parent()));

			dlg.show();
			dlg.raise();
			dlg.exec();
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
