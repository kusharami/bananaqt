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

#include "ChangeValueCommand.h"

#include "Utils.h"
#include "Const.h"
#include "Object.h"
#include "PropertyDef.h"

#include <QCoreApplication>

namespace Banana
{

ChangeValueCommand::ChangeValueCommand(
	Object *object, const QString &oldName, const QString &newName)
	: AbstractObjectUndoCommand(object)
{
	objectPath[0] = oldName;

	newStateBits = object->getPropertyModifiedBits();
	oldStateBits = newStateBits;

	auto metaProperty = Utils::GetMetaPropertyByName(this, "objectName");

	pushEntry({ metaProperty, oldName, newName });
}

ChangeValueCommand::ChangeValueCommand(
	Object *object, const QMetaProperty &metaProperty,
	const QVariant &oldValue)
	: AbstractObjectUndoCommand(object)
{
	newStateBits = object->getPropertyModifiedBits();
	oldStateBits = newStateBits;

	pushEntry(metaProperty, oldValue);
}

ChangeValueCommand::ChangeValueCommand(
	Object *object, int propertyId, bool oldState)
	: AbstractObjectUndoCommand(object)
{
	Q_ASSERT(propertyId >= 0);
	Q_ASSERT(propertyId < 64);

	newStateBits = object->getPropertyModifiedBits();
	oldStateBits = newStateBits;

	quint64 flag = 1ULL << propertyId;
	if (oldState)
		oldStateBits |= flag;
	else
		oldStateBits &= ~flag;
}

ChangeValueCommand::ChangeValueCommand(Object *object, quint64 oldStateBits)
	: AbstractObjectUndoCommand(object)
	, oldStateBits(oldStateBits)
	, newStateBits(object->getPropertyModifiedBits())
{
}

int ChangeValueCommand::id() const
{
	return CHANGE_VALUE_COMMAND;
}

bool ChangeValueCommand::mergeWith(const QUndoCommand *other)
{
	auto otherCommand = dynamic_cast<const ChangeValueCommand *>(other);

	if (nullptr != otherCommand)
	{
		fetchObject();

		if (getObject() == otherCommand->getObject())
		{
			OrderedEntries orderedEntries;
			auto orderedEntriesPtr = &otherCommand->orderedEntries;
			if (orderedEntriesPtr->empty())
			{
				otherCommand->prepareOrderedEntries(orderedEntries);
				orderedEntriesPtr = &orderedEntries;
			}
			for (auto entry : *orderedEntriesPtr)
			{
				pushEntry(*entry);
			}

			newStateBits = otherCommand->newStateBits;

			return true;
		}
	}

	return false;
}

void ChangeValueCommand::doUndo()
{
	applyValues(false);
	applyStateBits(oldStateBits);
}

void ChangeValueCommand::doRedo()
{
	applyValues(true);
	applyStateBits(newStateBits);
}

bool ChangeValueCommand::entryIndexLess(const EntryData *a, const EntryData *b)
{
	return a->index < b->index;
}

void ChangeValueCommand::prepareOrderedEntries(
	OrderedEntries &orderedEntries) const
{
	if (!entries.empty() && orderedEntries.empty())
	{
		for (auto &it : entries)
		{
			orderedEntries.push_back(&it.second);
		}

		std::sort(
			orderedEntries.begin(), orderedEntries.end(),
			ChangeValueCommand::entryIndexLess);
	}
}

void ChangeValueCommand::prepareOrderedEntries()
{
	prepareOrderedEntries(orderedEntries);
}

void ChangeValueCommand::applyValues(bool redo)
{
	auto object = dynamic_cast<Object *>(getObject());
	Q_ASSERT(nullptr != object);

	prepareOrderedEntries();

	object->beginUndoStackUpdate();
	object->blockMacro();
	object->beginLoad();
	object->beginReload();

	if (redo)
	{
		for (auto entry : orderedEntries)
		{
			entry->metaProperty.write(object, entry->newValue);
		}
	} else
	{
		for (auto rit = orderedEntries.rbegin();
			 rit != orderedEntries.rend();
			 ++rit)
		{
			auto entry = *rit;
			entry->metaProperty.write(object, entry->oldValue);
		}
	}

	object->endReload();
	object->endLoad();
	object->unblockMacro();
	object->endUndoStackUpdate();
}

void ChangeValueCommand::applyStateBits(quint64 bits)
{
	auto object = dynamic_cast<Object *>(getObject());
	Q_ASSERT(nullptr != object);

	object->setPropertyModifiedBits(bits);
}

void ChangeValueCommand::pushEntry(
	const QMetaProperty &metaProperty, const QVariant &oldValue)
{
	pushEntry({ metaProperty, oldValue, metaProperty.read(getObject()) });
}

void ChangeValueCommand::pushEntry(const EntryData &entryData)
{
	auto propertyIndex = entryData.metaProperty.propertyIndex();
	auto it = entries.find(propertyIndex);
	if (it == entries.end())
	{
		auto index = entries.size();
		entries[propertyIndex] = {
			entryData.metaProperty,
			entryData.oldValue,
			entryData.newValue,
			index
		};
	} else
	{
		it->second.newValue = entryData.newValue;
	}

	orderedEntries.clear();
}

QString ChangeValueCommand::getMultipleResetCommandTextFor(
	const QMetaObject *metaObject, const char *propertyName)
{
	Q_ASSERT(nullptr != metaObject);
	Q_ASSERT(nullptr != propertyName);

	return resetCommandPattern().arg(
		multipleObjectsStr(),
		QCoreApplication::translate(
			metaObject->className(), propertyName));
}

QString ChangeValueCommand::getMultipleResetCommandTextFor(
	const QMetaObject *metaObject, const QMetaProperty &metaProperty)
{
	Q_ASSERT(nullptr != metaObject);

	return resetCommandPattern().arg(
		multipleObjectsStr(),
		QCoreApplication::translate(
			metaObject->className(), metaProperty.name()));
}

QString ChangeValueCommand::getResetCommandTextFor(
	Object *object, const char *propertyName)
{
	Q_ASSERT(nullptr != object);
	Q_ASSERT(nullptr != propertyName);

	auto metaProperty = Utils::GetMetaPropertyByName(object, propertyName);
	return getResetCommandTextFor(object, metaProperty);
}

QString ChangeValueCommand::getResetCommandTextFor(
	Object *object, const QMetaProperty &metaProperty)
{
	Q_ASSERT(nullptr != object);

	auto metaObject = Utils::GetMetaObjectForProperty(metaProperty);
	Q_ASSERT(nullptr != metaObject);

	auto objectName = object->objectName();
	if (objectName.isEmpty())
		objectName = QCoreApplication::translate(
				"ClassName",
				object->metaObject()->className());

	return resetCommandPattern().arg(
		objectName,
		QCoreApplication::translate(
			metaObject->className(), metaProperty.name()));
}

QString ChangeValueCommand::getMultipleCommandTextFor(
	const QMetaObject *metaObject, const char *propertyName)
{
	Q_ASSERT(nullptr != metaObject);

	QString propertyNameTr(QCoreApplication::translate(
							   metaObject->className(), propertyName));

	return changeValueCommandPattern().arg(
		multipleObjectsStr(), propertyNameTr);
}

QString ChangeValueCommand::getMultipleCommandTextFor(
	const QMetaObject *metaObject, const QMetaProperty &metaProperty)
{
	return getMultipleCommandTextFor(metaObject, metaProperty.name());
}

QString ChangeValueCommand::getCommandTextFor(
	Object *object, const char *propertyName)
{
	Q_ASSERT(nullptr != object);
	Q_ASSERT(nullptr != propertyName);

	return getCommandTextFor(
		object,
		Utils::GetMetaPropertyByName(
			object, propertyName));
}

QString ChangeValueCommand::getCommandTextFor(
	Object *object, const QMetaProperty &metaProperty)
{
	Q_ASSERT(nullptr != object);

	auto metaObject = Utils::GetMetaObjectForProperty(metaProperty);
	Q_ASSERT(nullptr != metaObject);

	QString objectName(object->objectName());

	if (objectName.isEmpty())
		objectName = QCoreApplication::translate(
				"ClassName", object->metaObject()->className());

	QString propertyName(QCoreApplication::translate(
							 metaObject->className(), metaProperty.name()));

	return changeValueCommandPattern().arg(objectName, propertyName);
}

QString ChangeValueCommand::resetCommandPattern()
{
	return tr("Reset value of <%2> [%1]");
}

QString ChangeValueCommand::changeValueCommandPattern()
{
	return tr("Change value of <%2> [%1]");
}

QString ChangeValueCommand::multipleObjectsStr()
{
	return tr("Multiple objects");
}

}
