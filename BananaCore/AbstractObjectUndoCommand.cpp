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

#include "AbstractObjectUndoCommand.h"

namespace Banana
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
