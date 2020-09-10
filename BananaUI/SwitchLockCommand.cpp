#include "SwitchLockCommand.h"

#include "ChangeValueCommand.h"

#include "BananaCore/Utils.h"
#include "BananaCore/Const.h"
#include "BananaCore/Object.h"

#include <QCoreApplication>

namespace Banana
{
QString SwitchLockCommand::getMultipleCommandTextFor(
	const QMetaObject *metaObject, const char *propertyName, bool locked)
{
	return commandPattern(locked).arg(ChangeValueCommand::multipleObjectsStr(),
		QCoreApplication::translate(metaObject->className(), propertyName));
}

QString SwitchLockCommand::getCommandTextFor(
	Object *object, const QMetaProperty &metaProperty, bool locked)
{
	auto metaObject = Utils::GetMetaObjectForProperty(metaProperty);
	Q_ASSERT(nullptr != metaObject);

	auto objectName = object->objectName();
	if (objectName.isEmpty())
	{
		objectName = QCoreApplication::translate(
			"ClassName", object->metaObject()->className());
	}

	return commandPattern(locked).arg(objectName,
		QCoreApplication::translate(
			metaObject->className(), metaProperty.name()));
}

SwitchLockCommand::SwitchLockCommand(
	Object *object, const QMetaProperty &metaProperty, bool locked)
	: AbstractObjectUndoCommand(object)
	, metaProperty(metaProperty)
	, locked(locked)
{
}

int SwitchLockCommand::id() const
{
	return SWITCH_LOCKED_COMMAND;
}

bool SwitchLockCommand::mergeWith(const QUndoCommand *other)
{
	auto otherCommand = dynamic_cast<const SwitchLockCommand *>(other);
	if (!otherCommand)
		return false;

	if (getObject() != otherCommand->getObject())
	{
		return false;
	}

	if (metaProperty.propertyIndex() !=
		otherCommand->metaProperty.propertyIndex())
	{
		return false;
	}

	return true;
}

void SwitchLockCommand::doUndo()
{
	switchLock(!locked);
}

void SwitchLockCommand::doRedo()
{
	switchLock(locked);
}

void SwitchLockCommand::switchLock(bool locked)
{
	auto object = qobject_cast<Object *>(getObject());
	Q_ASSERT(nullptr != object);

	object->beginUndoStackUpdate();
	object->blockMacro();
	object->beginLoad();
	object->beginReload();

	object->setPropertyLocked(metaProperty, locked);

	object->endReload();
	object->endLoad();
	object->unblockMacro();
	object->endUndoStackUpdate();
}

QString SwitchLockCommand::commandPattern(bool locked)
{
	if (locked)
	{
		return tr("Lock <%2> [%1]");
	}
	return tr("Unlock <%2> [%1]");
}
}
