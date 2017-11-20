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

#include "AbstractObjectUndoCommand.h"

namespace Banana
{
AbstractObjectUndoCommand::AbstractObjectUndoCommand(QObject *object)
	: object(object)
	, fetchIndex(-1)
	, skipRedoOnPush(true)
{
	while (nullptr != object)
	{
		objectPath.push_back(object->objectName());
		object = object->parent();
	}

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

QUndoCommand *AbstractObjectUndoCommand::qundoCommand()
{
	return this;
}

QObject *AbstractObjectUndoCommand::getObject() const
{
	if (fetchIndex >= 0)
		return nullptr;

	return object;
}

void AbstractObjectUndoCommand::onObjectDestroyed()
{
	class ObjectHack : public QObject
	{
	public:
		bool wasDeleted() const
		{
			return d_ptr->wasDeleted;
		}
	};

	while (nullptr != object && static_cast<ObjectHack *>(object)->wasDeleted())
	{
		fetchIndex++;
		object = object->parent();
	}

	if (nullptr != object)
		connectObject();
}

void AbstractObjectUndoCommand::fetchObject()
{
	if (fetchIndex >= 0)
	{
		disconnectObject();

		for (int i = fetchIndex; i >= 0; i--)
		{
			object = object->findChild<QObject *>(
				objectPath.at(i), Qt::FindDirectChildrenOnly);
			Q_ASSERT(nullptr != object);
		}

		fetchIndex = -1;

		connectObject();
	}

	Q_ASSERT(nullptr != object);
}

void AbstractObjectUndoCommand::connectObject()
{
	Q_ASSERT(nullptr != object);

	QObject::connect(object, &QObject::destroyed, this,
		&AbstractObjectUndoCommand::onObjectDestroyed);
}

void AbstractObjectUndoCommand::disconnectObject()
{
	Q_ASSERT(nullptr != object);

	QObject::disconnect(object, &QObject::destroyed, this,
		&AbstractObjectUndoCommand::onObjectDestroyed);
}
}
