#include "ChildActionCommand.h"

#include "Object.h"
#include "Const.h"

namespace Core
{

	ChildActionCommand::ChildActionCommand(Object *object, Action action)
		: AbstractObjectUndoCommand(object->parent())
	{
		initFields(object, action);
	}

	ChildActionCommand::ChildActionCommand(Object *object, Object *parent, Action action)
		: AbstractObjectUndoCommand(parent)
	{
		initFields(object, action);
	}

	ChildActionCommand::ChildActionCommand(Object *object, Object *oldParent)
		: AbstractObjectUndoCommand(object->parent())
	{
		if (nullptr == oldParent)
		{
			initFields(object, Add);
		} else
		{
			childMetaObject = object->metaObject();
			childObjectName = object->objectName();
			stateBits = object->getPropertyModifiedBits();
			action = Move;
			subCommand = new ChildActionCommand(object, oldParent, Delete);
			savedContents = subCommand->savedContents;
		}
	}

	ChildActionCommand::~ChildActionCommand()
	{
		if (nullptr != subCommand
		&&	savedContents == subCommand->savedContents)
		{
			subCommand->savedContents = nullptr;
		}

		delete savedContents;
		delete subCommand;
	}

	void ChildActionCommand::initFields(Object *object, Action action)
	{
		Q_ASSERT(nullptr != object);

		childMetaObject = object->metaObject();
		childObjectName = object->objectName();
		stateBits = object->getPropertyModifiedBits();
		this->action = action;
		savedContents = new QVariantMap;
		subCommand = nullptr;
		object->saveContents(*savedContents, Object::SaveStandalone);
	}

	int ChildActionCommand::id() const
	{
		return CHILD_ACTION_COMMAND;
	}

	QString ChildActionCommand::getAddCommandTextFor(Object *object)
	{
		Q_ASSERT(nullptr != object);
		return tr("Add object [%1]").arg(object->objectName());
	}

	QString ChildActionCommand::getMultiAddCommandText()
	{
		return tr("Add multiple objects");
	}

	QString ChildActionCommand::getDeleteCommandTextFor(Object *object)
	{
		Q_ASSERT(nullptr != object);
		return tr("Delete object [%1]").arg(object->objectName());
	}

	QString ChildActionCommand::getMultiDeleteCommandText()
	{
		return tr("Delete multiple objects");
	}

	void ChildActionCommand::doUndo()
	{
		switch (action)
		{
			case Add:
				del();
				break;

			case Move:
				del();
				subCommand->add();
				break;

			case Delete:
				add();
				break;
		}
	}

	void ChildActionCommand::doRedo()
	{
		switch (action)
		{
			case Add:
				add();
				break;

			case Move:
				subCommand->del();
				add();
				break;

			case Delete:
				del();
				break;
		}
	}

	void ChildActionCommand::add()
	{
		auto newChild = dynamic_cast<Object *>(childMetaObject->newInstance());
		Q_ASSERT(nullptr != newChild);

		auto object = dynamic_cast<Object *>(getObject());
		Q_ASSERT(nullptr != object);

		object->beginUndoStackUpdate();
		newChild->beginLoad();

		newChild->loadContents(*savedContents, true);
		newChild->setObjectName(childObjectName);
		newChild->setParent(getObject());

		newChild->endLoad();
		object->endUndoStackUpdate();

		newChild->setPropertyModifiedBits(stateBits);
	}

	void ChildActionCommand::del()
	{
		auto object = dynamic_cast<Object *>(getObject());
		Q_ASSERT(nullptr != object);

		object->beginUndoStackUpdate();

		auto toDelete = object->findChild<Object *>(childObjectName, Qt::FindDirectChildrenOnly);
		Q_ASSERT(nullptr != toDelete);
		delete toDelete;

		object->endUndoStackUpdate();
	}

}
