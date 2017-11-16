/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#pragma once

#include <QtGlobal>
#include <QVariantMap>

namespace Banana
{
class Object;
class UndoStack;
struct IUndoCommand;
struct IUndoStack
{
	virtual ~IUndoStack() {}

	typedef void (Object::*CleanChanged)(bool);

	virtual UndoStack *qundoStack() = 0;

	virtual void clear() = 0;
	virtual void beginMacro(const QString &text) = 0;
	virtual void endMacro() = 0;
	virtual void blockMacro() = 0;
	virtual void unblockMacro() = 0;
	virtual void beginUpdate() = 0;
	virtual void endUpdate() = 0;
	virtual bool canPushForMacro() const = 0;
	virtual bool isUpdating() const = 0;
	virtual void setClean() = 0;
	virtual void connectCleanChanged(Object *object, CleanChanged cb) = 0;
	virtual void disconnectCleanChanged(Object *object, CleanChanged cb) = 0;

	virtual void pushCommand(IUndoCommand *command) = 0;
	virtual void pushChangeName(
		Object *object, const QString &oldName, const QString &newName) = 0;
	virtual void pushAddChild(Object *child) = 0;
	virtual void pushMoveChild(Object *child, Object *oldParent) = 0;
	virtual void pushDeleteChild(Object *child) = 0;
	virtual void pushValueChange(Object *object,
		const QMetaProperty &metaProperty, const QVariant &oldValue) = 0;
	virtual void pushPropertyChange(
		Object *object, int propertyId, bool oldState) = 0;
	virtual void pushMultiPropertyChange(
		Object *object, quint64 oldStateBits) = 0;
	virtual void pushContentsChange(
		Object *object, const QVariantMap &oldContents) = 0;
};
}
