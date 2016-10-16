#pragma once

#include "AbstractObjectUndoCommand.h"

#include <QVariantMap>

namespace Banana
{
	class Object;

	class ChangeContentsCommand : public AbstractObjectUndoCommand
	{
	public:
		ChangeContentsCommand(Object *object, const QVariantMap &oldContents);

		virtual int id() const override;

	protected:
		virtual void doUndo() override;
		virtual void doRedo() override;

	private:
		void applyContents(const QVariantMap &contents);

		QVariantMap oldContents;
		QVariantMap newContents;
	};
}
