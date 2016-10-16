#pragma once

#include "AbstractObjectUndoCommand.h"

#include <QVariantMap>

namespace Banana
{
	class Object;

	class ChildActionCommand : public AbstractObjectUndoCommand
	{
		Q_OBJECT

	public:
		enum Action
		{
			Add,
			Move,
			Delete
		};

		ChildActionCommand(Object *object, Action action);
		ChildActionCommand(Object *object, Object *oldParent);
		virtual ~ChildActionCommand();

		virtual int id() const override;

	public:
		static QString getAddCommandTextFor(Object *object);
		static QString getMultiAddCommandText();
		static QString getDeleteCommandTextFor(Object *object);
		static QString getMultiDeleteCommandText();

	protected:
		virtual void doUndo() override;
		virtual void doRedo() override;

	private:
		ChildActionCommand(Object *object, Object *parent, Action action);
		void initFields(Object *object, Action action);

		void add();
		void del();

		const QMetaObject *childMetaObject;
		QString childObjectName;

		QVariantMap *savedContents;

		quint64 stateBits;
		Action action;

		ChildActionCommand *subCommand;
	};
}
