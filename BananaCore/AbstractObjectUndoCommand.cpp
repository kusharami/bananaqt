#include "AbstractObjectUndoCommand.h"

namespace Core
{

	AbstractObjectUndoCommand::AbstractObjectUndoCommand(QObject *object)
		: object(object)
		, skipRedoOnPush(true)
	{
		connectObject();
	}

	void AbstractObjectUndoCommand::undo()
	{
		fetchObject();

		doUndo();
	}

	void AbstractObjectUndoCommand::redo()
	{
		if (skipRedoOnPush)
		{
			skipRedoOnPush = false;
			return;
		}

		fetchObject();

		doRedo();
	}

	QObject *AbstractObjectUndoCommand::getObject() const
	{
		if (!objectPath.empty())
			return nullptr;

		return object;
	}

	void AbstractObjectUndoCommand::onObjectDestroyed()
	{
		object = objectParent;
		objectPath.push_back(objectName);
		if (nullptr != object)
			connectObject();
	}

	void AbstractObjectUndoCommand::fetchObject()
	{
		if (!objectPath.isEmpty())
		{
			disconnectObject();

			for (int i = objectPath.count() - 1; i >= 0; i--)
			{
				objectParent = object;
				object = objectParent->findChild<QObject *>(objectPath.at(i), Qt::FindDirectChildrenOnly);
				Q_ASSERT(nullptr != object);
			}

			objectPath.clear();

			connectObject();
		}

		Q_ASSERT(nullptr != object);
	}

	void AbstractObjectUndoCommand::connectObject()
	{
		Q_ASSERT(nullptr != object);

		objectParent = object->parent();
		objectName = object->objectName();

		QObject::connect(object, &QObject::destroyed,
						 this, &AbstractObjectUndoCommand::onObjectDestroyed);
	}

	void AbstractObjectUndoCommand::disconnectObject()
	{
		Q_ASSERT(nullptr != object);

		QObject::disconnect(object, &QObject::destroyed,
							this, &AbstractObjectUndoCommand::onObjectDestroyed);
	}

}
