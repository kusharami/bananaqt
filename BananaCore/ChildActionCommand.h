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
