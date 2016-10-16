#include "ChangeContentsCommand.h"

#include "Object.h"
#include "Const.h"

namespace Core
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
		object->beginLoad();
		object->loadContents(contents, true);
		object->endLoad();
		object->endUndoStackUpdate();
	}

}
