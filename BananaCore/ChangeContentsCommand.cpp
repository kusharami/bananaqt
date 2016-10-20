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

#include "ChangeContentsCommand.h"

#include "Object.h"
#include "Const.h"

namespace Banana
{

	ChangeContentsCommand::ChangeContentsCommand(Object *object, const QVariantMap &oldContents)
		: AbstractObjectUndoCommand(object)
		, oldContents(oldContents)
	{
		object->saveContents(newContents, Object::SaveStandalone);
	}

	int ChangeContentsCommand::id() const
	{
		return CHANGE_CONTENTS_COMMAND;
	}

	void ChangeContentsCommand::doUndo()
	{
		applyContents(oldContents);
	}

	void ChangeContentsCommand::doRedo()
	{
		applyContents(newContents);
	}

	void ChangeContentsCommand::applyContents(const QVariantMap &contents)
	{
		auto object = dynamic_cast<Object *>(getObject());
		Q_ASSERT(nullptr != object);

		object->beginUndoStackUpdate();
		object->blockMacro();
		object->beginLoad();
		object->beginReload();

		object->loadContents(contents, true);

		object->endReload();
		object->endLoad();
		object->unblockMacro();
		object->endUndoStackUpdate();
	}

}
