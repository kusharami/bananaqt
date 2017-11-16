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

#include <QMetaProperty>
#include <QVariant>

#include <vector>
#include <map>

namespace Banana
{
class Object;

class ChangeValueCommand : public AbstractObjectUndoCommand
{
	Q_OBJECT

public:
	static QString getMultipleResetCommandTextFor(
		const QMetaObject *metaObject, const char *propertyName);
	static QString getMultipleResetCommandTextFor(
		const QMetaObject *metaObject, const QMetaProperty &metaProperty);
	static QString getResetCommandTextFor(
		Object *object, const char *propertyName);
	static QString getResetCommandTextFor(
		Object *object, const QMetaProperty &metaProperty);

	static QString getMultipleCommandTextFor(
		const QMetaObject *metaObject, const char *propertyName);
	static QString getMultipleCommandTextFor(
		const QMetaObject *metaObject, const QMetaProperty &metaProperty);

	static QString getCommandTextFor(Object *object, const char *propertyName);
	static QString getCommandTextFor(
		Object *object, const QMetaProperty &metaProperty);

	static QString resetCommandPattern();
	static QString changeValueCommandPattern();
	static QString setValueCommandPattern();
	static QString multipleObjectsStr();

public:
	ChangeValueCommand(
		Object *object, const QString &oldName, const QString &newName);
	ChangeValueCommand(Object *object, const QMetaProperty &metaProperty,
		const QVariant &oldValue);
	ChangeValueCommand(Object *object, int propertyId, bool oldState);
	ChangeValueCommand(Object *object, quint64 oldStateBits);

	virtual int id() const override;
	virtual bool mergeWith(const QUndoCommand *other) override;

protected:
	virtual void doUndo() override;
	virtual void doRedo() override;

private:
	void applyValues(bool redo);
	void applyStateBits(quint64 bits);

	struct EntryData
	{
		QMetaProperty metaProperty;

		QVariant oldValue;
		QVariant newValue;

		size_t index;
	};

	typedef std::vector<const EntryData *> OrderedEntries;

	void pushEntry(const EntryData &entryData);
	void pushEntry(const QMetaProperty &metaProperty, const QVariant &oldValue);
	void prepareOrderedEntries();
	void prepareOrderedEntries(OrderedEntries &orderedEntries) const;
	static bool entryIndexLess(const EntryData *a, const EntryData *b);

	OrderedEntries orderedEntries;
	std::map<int, EntryData> entries;

	quint64 oldStateBits;
	quint64 newStateBits;
};
}
