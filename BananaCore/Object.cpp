/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "Object.h"

#include "Core.h"
#include "Utils.h"
#include "ChangeValueCommand.h"
#include "ChangeContentsCommand.h"
#include "ChildActionCommand.h"
#include "PropertyDef.h"
#include "UndoStack.h"

#include <QChildEvent>
#include <QMetaProperty>
#include <QDebug>

namespace Banana
{
const char szOBJECT_NAME_KEY[] = PROP(objectName);
const char szCLASS_NAME_KEY[] = "__CLASS_NAME";
const char szCHILDREN_KEY[] = "__CHILDREN";

Object::Object()
	: prototype(nullptr)
	, childPrototype(nullptr)
	, reloadCounter(0)
	, protoReloadCounter(0)
	, loadCounter(0)
	, macroCounter(0)
	, blockCounter(0)
	, undoStackUpdate(0)
	, undoStack(nullptr)
	, ownUndoStack(false)
	, modified(false)
	, deleted(false)
{
	(void) QT_TRANSLATE_NOOP("ClassName", "Banana::Object");

	QObject::connect(
		this, &QObject::objectNameChanged, this, &Object::onObjectNameChanged);
}

Object::~Object()
{
	if (nullptr != undoStack)
	{
		disconnectUndoStack();
		Q_ASSERT(undoStackUpdate == 0);
		Q_ASSERT(macroCounter == 0);
		Q_ASSERT(blockCounter == 0);

		if (ownUndoStack)
			delete undoStack;

		undoStack = nullptr;
	}
	deleted = true;
	emit beforeDestroy(this);
}

void Object::beforePrototypeChange()
{
	if (!isLoading())
	{
		beginReload();
		if (reloadCounter == 1)
			removeAllChildrenInternal();
	}
}

void Object::afterPrototypeChange()
{
	if (!isLoading())
		endReload();
}

bool Object::shouldSwapModifiedFieldsFor(QObject *source) const
{
	return (nullptr != prototype && source != prototype && isLoading());
}

void Object::beginReload()
{
	if (reloadCounter++ == 0)
	{
		if (nullptr != undoStack)
		{
			undoStack->clear();
		}

		emit reloadStarted();
	}
}

void Object::endReload()
{
	Q_ASSERT(reloadCounter > 0);

	if (--reloadCounter == 0)
		emit reloadFinished();
}

void Object::beginUndoStackUpdate()
{
	if (nullptr != undoStack)
		undoStack->beginUpdate();

	undoStackUpdate++;
}

void Object::endUndoStackUpdate()
{
	Q_ASSERT(undoStackUpdate > 0);
	undoStackUpdate--;

	if (nullptr != undoStack)
		undoStack->endUpdate();
}

QObject *Object::getDescendant(const QStringList &path) const
{
	return Utils::GetDescendant(this, path);
}

QStringList Object::getNamesChain(const QObject *topAncestor) const
{
	return Utils::GetNamesChain(topAncestor, this);
}

bool Object::setPropertyModified(int propertyId, bool modified)
{
	bool oldModified = isPropertyModified(propertyId);
	if (oldModified != modified)
	{
		modifiedSet.set(propertyId, modified);

		if (canPushUndoCommand())
			undoStack->push(
				new ChangeValueCommand(this, propertyId, oldModified));

		emit modifiedSetChanged();

		return true;
	}

	return false;
}

void Object::setPropertyModifiedBits(quint64 propertyIdBits)
{
	auto oldBits = getPropertyModifiedBits();
	if (oldBits != propertyIdBits)
	{
		modifiedSet = ModifiedSet(propertyIdBits);
		if (canPushUndoCommand())
			undoStack->push(new ChangeValueCommand(this, oldBits));

		emit modifiedSetChanged();
	}
}

void Object::deprototype()
{
	disconnectPrototype();
	disconnectChildPrototype();

	for (auto child : children())
	{
		auto object = dynamic_cast<Object *>(child);
		if (nullptr != object)
			object->deprototype();
	}
}

void Object::setPrototype(Object *prototype)
{
	if (nullptr != prototype && prototype->deleted)
		prototype = nullptr;

	if (this->prototype != prototype)
	{
		QVariantMap oldContents;
		bool canPushUndoCommand = this->canPushUndoCommand();
		if (canPushUndoCommand)
		{
			saveContents(oldContents, SaveStandalone);
		}

		blockMacro();
		beforePrototypeChange();

		internalSetPrototype(prototype, false, false);

		afterPrototypeChange();
		unblockMacro();

		if (canPushUndoCommand)
		{
			undoStack->push(new ChangeContentsCommand(this, oldContents));
		}
	}
}

bool Object::loadContents(
	const QVariantMap &source, QObject *destination, bool skipObjectName)
{
	if (nullptr != destination)
	{
		auto metaObject = destination->metaObject();
		auto it = source.find(szCLASS_NAME_KEY);
		if (it != source.end() &&
			0 ==
				strcmp(metaObject->className(),
					it.value().toString().toLatin1().data()))
		{
			int count = metaObject->propertyCount();

			for (int i = 0; i < count; i++)
			{
				QMetaProperty property = metaObject->property(i);

				if (property.isWritable() && !property.isConstant())
				{
					if (skipObjectName)
					{
						if (0 == strcmp(property.name(), szOBJECT_NAME_KEY))
							continue;
					}

					bool setDefault = true;
					it = source.find(property.name());
					if (it != source.end())
					{
						if (property.type() == QVariant::UserType)
						{
							property.write(destination,
								Banana::ConvertToUserVariant(
									property.userType(), it.value()));
							setDefault = false;
						} else
						{
							QVariant value(it.value());

							if (property.type() ==
									(QVariant::Type) QMetaType::QVariant ||
								value.canConvert((int) property.type()))
							{
								property.write(destination, value);
								setDefault = false;
							}
						}
					}

					if (setDefault && property.isStored(destination))
					{
						if (property.isResettable())
							property.reset(destination);
						else if (property.type() == QVariant::UserType)
							property.write(destination,
								Banana::ConvertToUserVariant(
									property.userType(), QVariant()));
						else
							property.write(
								destination, QVariant(property.type()));
					}
				}
			}

			return true;
		}
	}

	return false;
}

void Object::saveContents(
	const QObject *source, QVariantMap &destination, QObject *prototype)
{
	if (nullptr != source)
	{
		auto metaObject = source->metaObject();

		destination.insert(szCLASS_NAME_KEY, metaObject->className());

		int count = metaObject->propertyCount();

		for (int i = 0; i < count; i++)
		{
			QMetaProperty property = metaObject->property(i);

			if (property.isStored(source) && property.isReadable() &&
				property.isWritable() && !property.isConstant())
			{
				auto value =
					Banana::ConvertFromUserVariant(property.read(source));
				if (nullptr != prototype &&
					0 != strcmp(property.name(), szOBJECT_NAME_KEY))
				{
					if (value ==
						Banana::ConvertFromUserVariant(
							property.read(prototype)))
						continue;
				}
				destination.insert(property.name(), value);
			}
		}
	}
}

bool Object::loadContents(const QVariantMap &source, bool skipObjectName)
{
	beginReload();
	assign(nullptr);

	loadCounter++;

	bool ok = false;

	auto childrenValue =
		Utils::ValueFrom(source, szCHILDREN_KEY, QVariantList());
	if (childrenValue.type() == QVariant::List)
	{
		auto childrenSource = childrenValue.toList();
		childrenValue.clear();

		ok = true;

		std::vector<QObject *> childObjects;

		for (auto it = childrenSource.begin(); it != childrenSource.end(); ++it)
		{
			ok = false;

			if (it->type() == QVariant::Map)
			{
				QVariantMap childSource(it->toMap());
				auto classNameValue =
					Utils::ValueFrom(childSource, szCLASS_NAME_KEY, QString());
				if (classNameValue.type() == QVariant::String)
				{
					QString className(classNameValue.toString());

					auto metaObject = getStorableChildType(className);
					if (nullptr != metaObject)
					{
						auto newObject = metaObject->newInstance();
						Q_ASSERT(nullptr != newObject);

						childObjects.push_back(newObject);

						auto newChild = dynamic_cast<Object *>(newObject);

						if ((nullptr != newChild &&
								newChild->loadContents(childSource, false)) ||
							(nullptr == newChild &&
								loadContents(childSource, newChild, false)))
						{
							ok = true;
						} else
							qDebug() << "Object::loadContents - Child object "
										"load failed";

					} else
						qDebug() << QString(
							"Object::loadContents - Unknown class name '%1'")
										.arg(className);
				} else
					qDebug()
						<< "Object::loadContents - Bad child object format";
			} else
				qDebug() << "Object::loadContents - Bad child object type";

			if (!ok)
				break;
		}

		if (ok && loadContents(source, this, skipObjectName))
		{
			if (nullptr != prototype)
			{
				for (auto sourceChild : childObjects)
				{
					auto object = dynamic_cast<Object *>(sourceChild);
					if (nullptr != object)
						object->beginLoad();

					if (assignChild(sourceChild, false))
						delete sourceChild;
					else
					{
						sourceChild->setParent(this);

						if (nullptr != object)
							object->endLoad();
					}
				}
			} else
			{
				for (auto child : childObjects)
				{
					auto object = dynamic_cast<Object *>(child);
					if (nullptr != object)
						object->beginLoad();

					child->setParent(this);

					if (nullptr != object)
						object->endLoad();
				}
			}
		} else
		{
			for (auto child : childObjects)
			{
				delete child;
			}
		}
	}

	loadCounter--;

	endReload();
	return ok;
}

void Object::saveContents(QVariantMap &destination, SaveMode saveMode) const
{
	bool ignorePrototype;
	switch (saveMode)
	{
		case SavePrototyped:
			ignorePrototype = false;
			break;

		case SaveStandaloneInheritedChild:
			ignorePrototype = isInheritedChild() && !isPrototypedRoot();
			break;

		case SaveStandalone:
			ignorePrototype = true;
			break;
	}

	QVariantList childList;
	for (auto child : children())
	{
		if (isStorableChildType(child->metaObject()))
		{
			QVariantMap childMap;
			auto object = dynamic_cast<Object *>(child);
			if (nullptr != object)
				object->saveContents(childMap, saveMode);
			else
				saveContents(child, childMap, nullptr);

			childList.push_back(childMap);
		}
	}

	if (!childList.isEmpty())
		destination.insert(szCHILDREN_KEY, childList);

	saveContents(this, destination, ignorePrototype ? nullptr : prototype);
}

QVariantMap Object::backupContents() const
{
	QVariantMap result;

	saveContents(result, SaveStandalone);

	return result;
}

void Object::applyContents(const QVariantMap &source)
{
	QVariantMap oldContents;
	bool canPushUndoCommand = this->canPushUndoCommand();
	if (canPushUndoCommand)
	{
		saveContents(oldContents, SaveStandalone);
	}

	blockMacro();
	beginReload();

	loadContents(source, true);

	endReload();
	unblockMacro();

	if (canPushUndoCommand)
	{
		undoStack->push(new ChangeContentsCommand(this, oldContents));
	}
}

void Object::setUndoStack(UndoStack *undoStack, bool own)
{
	if (this->undoStack != undoStack)
	{
		disconnectUndoStack();

		Q_ASSERT(undoStackUpdate == 0);
		Q_ASSERT(macroCounter == 0);
		Q_ASSERT(blockCounter == 0);

		if (ownUndoStack)
			delete this->undoStack;
		this->undoStack = undoStack;

		ownUndoStack = own;

		connectUndoStack();
	}
	if (ownUndoStack != own)
	{
		disconnectUndoStack();

		ownUndoStack = own;

		connectUndoStack();
	}
	for (auto child : children())
	{
		auto object = dynamic_cast<Object *>(child);
		if (nullptr != object)
			object->setUndoStack(undoStack, false);
	}
}

void Object::beginMacro(const QString &text)
{
	if (nullptr != undoStack)
		undoStack->beginMacro(text);

	macroCounter++;
}

void Object::endMacro()
{
	Q_ASSERT(macroCounter > 0);
	macroCounter--;

	if (nullptr != undoStack)
		undoStack->endMacro();
}

void Object::blockMacro()
{
	if (nullptr != undoStack)
		undoStack->blockMacro();

	blockCounter++;
}

void Object::unblockMacro()
{
	Q_ASSERT(blockCounter > 0);
	blockCounter--;

	if (nullptr != undoStack)
		undoStack->unblockMacro();
}

bool Object::undoStackIsUpdating() const
{
	return (nullptr != undoStack && undoStack->isUpdating());
}

void Object::assign(QObject *source)
{
	if (source != this)
	{
		QVariantMap oldContents;
		bool canPushUndoCommand = this->canPushUndoCommand();
		if (canPushUndoCommand)
		{
			saveContents(oldContents, SaveStandalone);
		}

		blockMacro();
		beginReload();

		removeAllChildrenInternal();
		internalAssign(source, true, true);

		endReload();
		unblockMacro();

		if (canPushUndoCommand)
		{
			undoStack->push(new ChangeContentsCommand(this, oldContents));
		}
	}
}

void Object::removeAllChildren()
{
	QVariantMap oldContents;
	bool canPushUndoCommand = this->canPushUndoCommand();
	if (canPushUndoCommand)
	{
		saveContents(oldContents, SaveStandalone);
	}

	blockMacro();
	beginReload();

	removeAllChildrenInternal();

	endReload();
	unblockMacro();

	if (canPushUndoCommand)
	{
		undoStack->push(new ChangeContentsCommand(this, oldContents));
	}
}

void Object::setModified(bool value)
{
	if (modified != value)
	{
		modified = value;

		if (!modified && ownUndoStack && nullptr != undoStack)
			undoStack->setClean();

		doFlagsChanged();

		auto parent = dynamic_cast<Object *>(this->parent());
		if (nullptr != parent)
			parent->setModified(value);
	}
}

void Object::modifyObject(
	QObject *object, bool modified, bool signalize, bool children)
{
	if (nullptr != object)
	{
		auto obj = dynamic_cast<Object *>(object);
		if (nullptr != obj)
		{
			if (signalize)
				obj->setModified(modified);
			else
				obj->modified = modified;
		}

		if (children)
		{
			foreach (QObject *child, object->children())
			{
				modifyObject(child, modified, signalize, true);
			}
		}
	}
}

void Object::onPrototypeChildAdded(QObject *protoChild)
{
	newChildFrom(protoChild, true);
}

void Object::onPrototypeChildRemoved(QObject *protoChild)
{
	auto child = findChild<Object *>(
		protoChild->objectName(), Qt::FindDirectChildrenOnly);
	if (nullptr != child)
		child->onPrototypeDestroyed(protoChild);
}

void Object::removeAllChildrenInternal()
{
	auto children = this->children();

	for (auto child : children)
	{
		delete child;
	}
}

bool Object::isAncestorOf(const QObject *object) const
{
	if (nullptr != object)
	{
		forever
		{
			object = object->parent();

			if (nullptr == object)
				break;

			if (object == this)
				return true;
		}
	}

	return false;
}

bool Object::canBeUsedAsPrototype(Object *object) const
{
	return (nullptr != object && this != object && !isDescendantOf(object) &&
		!isAncestorOf(object) && !object->checkPrototypeCycling(this));
}

void Object::onPrototypeDestroyed(QObject *object)
{
	if (deleted)
	{
		if (object == childPrototype)
			childPrototype = nullptr;

		if (object == prototype)
			prototype = nullptr;
		return;
	}

	if (object == childPrototype)
	{
		delete this;
		return;
	}

	if (object == prototype)
	{
		beforePrototypeChange();

		prototype = nullptr;
		if (nullptr != childPrototype)
			internalSetPrototype(childPrototype, false, false);

		afterPrototypeChange();
	}
}

void Object::onPrototypeReloadStarted()
{
	if (sender() != childPrototype || prototype == childPrototype)
	{
		beginReload();
		if (reloadCounter == 1)
		{
			loadCounter++;
			disconnectPrototypeChildLifeCycle();
			beforePrototypeReloadStarted();
		}
	}

	protoReloadCounter++;
}

void Object::onPrototypeReloadFinished()
{
	if (protoReloadCounter == 0)
		return;

	protoReloadCounter--;

	if (sender() != childPrototype || prototype == childPrototype)
	{
		if (reloadCounter == 1)
		{
			loadCounter--;
			internalAssign(prototype, true, true);
			connectPrototypeChildLifeCycle();
			setModified(true);
		}
		endReload();
	}
}

void Object::onLinkedObjectNameChanged(const QString &name)
{
	setObjectName(name);
}

void Object::onObjectNameChanged(const QString &newName)
{
	if (canPushUndoCommand())
		undoStack->push(new ChangeValueCommand(this, oldName, newName));

	oldName = newName;
	setModified(true);
}

void Object::onUndoStackCleanChanged(bool clean)
{
	setModified(!clean);
}

bool Object::canPushUndoCommand() const
{
	return (0 == reloadCounter && nullptr != undoStack &&
		undoStack->canPushForMacro());
}

void Object::addChildCommand(QObject *child)
{
	auto object = dynamic_cast<Object *>(child);
	if (nullptr != object && object->parent() == this)
	{
		if (canPushUndoCommand())
			undoStack->push(
				new ChildActionCommand(object, ChildActionCommand::Add));
	}
}

void Object::moveChildCommand(QObject *child, QObject *oldParent)
{
	auto object = dynamic_cast<Object *>(child);
	if (nullptr != object && object->parent() == this)
	{
		if (canPushUndoCommand())
			undoStack->push(new ChildActionCommand(
				object, dynamic_cast<Object *>(oldParent)));
	}
}

void Object::deleteChildCommand(QObject *child)
{
	auto object = dynamic_cast<Object *>(child);
	if (nullptr != object && object->parent() == this)
	{
		if (canPushUndoCommand())
			undoStack->push(
				new ChildActionCommand(object, ChildActionCommand::Delete));
	}
}

void Object::pushUndoCommandInternal(
	const char *propertyName, const QVariant &oldValue)
{
	undoStack->push(new ChangeValueCommand(this,
		Utils::GetMetaPropertyByName(metaObject(), propertyName), oldValue));
}

Object *Object::getMainPrototype() const
{
	if (nullptr == childPrototype)
		return prototype;

	return childPrototype;
}

void Object::internalAssign(QObject *source, bool fresh, bool top)
{
	if (fresh)
		loadCounter++;

	bool swap = assignBegin(source, top);
	assignProperties(source);
	assignChildren(source);
	assignEnd(source, top);

	if (swap)
	{
		auto object = dynamic_cast<Object *>(source);
		if (nullptr != object)
			modifiedSet = object->modifiedSet;
	}

	if (fresh)
		loadCounter--;
}

void Object::internalSetPrototype(Object *prototype, bool child, bool fresh)
{
	disconnectPrototype();
	if (child)
		disconnectChildPrototype();

	if (!canBeUsedAsPrototype(prototype))
		prototype = nullptr;

	this->prototype = prototype;
	if (child)
		childPrototype = prototype;

	fresh = fresh || isLoading();

	internalAssign(prototype, fresh, !child && !fresh);

	connectPrototype();
	if (child)
		connectChildPrototype();
}

void Object::connectChildPrototypeDestroy()
{
	if (nullptr != childPrototype && childPrototype != prototype)
	{
		QObject::connect(childPrototype, &QObject::destroyed, this,
			&Object::onPrototypeDestroyed);
	}
}

void Object::disconnectChildPrototypeDestroy()
{
	if (nullptr != childPrototype && childPrototype != prototype)
	{
		QObject::disconnect(childPrototype, &QObject::destroyed, this,
			&Object::onPrototypeDestroyed);
	}
}

void Object::connectPrototypeChildLifeCycle()
{
	if (nullptr != prototype)
	{
		QObject::connect(prototype, &Object::childAdded, this,
			&Object::onPrototypeChildAdded);
		QObject::connect(prototype, &Object::childRemoved, this,
			&Object::onPrototypeChildRemoved);
	}
}

void Object::disconnectPrototypeChildLifeCycle()
{
	if (nullptr != prototype)
	{
		QObject::disconnect(prototype, &Object::childAdded, this,
			&Object::onPrototypeChildAdded);
		QObject::disconnect(prototype, &Object::childRemoved, this,
			&Object::onPrototypeChildRemoved);
	}
}

void Object::connectPrototype()
{
	connectPrototypeChildLifeCycle();
	connectChildPrototypeDestroy();

	if (nullptr != prototype)
	{
		doConnectPrototype();
	}
}

void Object::disconnectPrototype()
{
	disconnectPrototypeChildLifeCycle();
	disconnectChildPrototypeDestroy();

	if (nullptr != prototype)
	{
		doDisconnectPrototype();
		prototype = nullptr;
	}
}

void Object::connectChildPrototype()
{
	if (nullptr != childPrototype)
	{
		setObjectName(childPrototype->objectName());
		QObject::connect(childPrototype, &QObject::objectNameChanged, this,
			&Object::onLinkedObjectNameChanged);
		QObject::connect(this, &QObject::objectNameChanged, childPrototype,
			&Object::onLinkedObjectNameChanged);
	}
}

void Object::disconnectChildPrototype()
{
	if (nullptr != childPrototype)
	{
		QObject::disconnect(childPrototype, &QObject::objectNameChanged, this,
			&Object::onLinkedObjectNameChanged);
		QObject::disconnect(this, &QObject::objectNameChanged, childPrototype,
			&Object::onLinkedObjectNameChanged);
		childPrototype = nullptr;
	}
}

bool Object::checkPrototypeCycling(const Object *object) const
{
	if (prototype == object)
		return true;

	foreach (QObject *child, children())
	{
		auto childObject = dynamic_cast<Object *>(child);

		if (nullptr != childObject &&
			childObject->checkPrototypeCycling(object))
			return true;
	}

	return false;
}

bool Object::isDescendantOf(const QObject *ancestor, const QObject *object)
{
	if (nullptr != object)
	{
		if (object->parent() == ancestor)
			return true;

		if (nullptr != ancestor)
		{
			foreach (QObject *child, ancestor->children())
			{
				if (isDescendantOf(child, object))
					return true;
			}
		}
	}

	return false;
}

bool Object::assignChild(QObject *sourceChild, bool isPrototype)
{
	auto child = findChild<Object *>(
		sourceChild->objectName(), Qt::FindDirectChildrenOnly);
	if (nullptr != child)
	{
		if (isPrototype && nullptr != prototype)
		{
			if (child->prototype != sourceChild)
				child->internalSetPrototype(
					dynamic_cast<Object *>(sourceChild), true, isLoading());
		} else
		{
			child->internalAssign(sourceChild, isLoading(), false);
		}

		return true;
	}

	return false;
}

void Object::beforePrototypeReloadStarted()
{
	for (auto child : children())
	{
		auto child_object = dynamic_cast<Object *>(child);
		if (nullptr != child_object)
		{
			child_object->beforeChildPrototypeReloadStarted();
		}
	}
}

void Object::beforeChildPrototypeReloadStarted()
{
	if (childPrototype == prototype)
		disconnectPrototype();
	else
		disconnectChildPrototypeDestroy();

	disconnectChildPrototype();
	beforePrototypeReloadStarted();
}

bool Object::canAssignPropertyFrom(QObject *source, int propertyId) const
{
	return (nullptr == prototype || (source != prototype && !isLoading()) ||
		!isPropertyModified(propertyId));
}

void Object::doConnectPrototype()
{
	QObject::connect(
		prototype, &QObject::destroyed, this, &Object::onPrototypeDestroyed);
	if (nullptr == childPrototype || prototype == childPrototype)
	{
		QObject::connect(prototype, &Object::reloadStarted, this,
			&Object::onPrototypeReloadStarted);
		QObject::connect(prototype, &Object::reloadFinished, this,
			&Object::onPrototypeReloadFinished);
	}
}

void Object::doDisconnectPrototype()
{
	QObject::disconnect(
		prototype, &QObject::destroyed, this, &Object::onPrototypeDestroyed);
	if (nullptr == childPrototype || prototype == childPrototype)
	{
		QObject::disconnect(prototype, &Object::reloadStarted, this,
			&Object::onPrototypeReloadStarted);
		QObject::disconnect(prototype, &Object::reloadFinished, this,
			&Object::onPrototypeReloadFinished);
	}
}

bool Object::assignBegin(QObject *source, bool)
{
	if (shouldSwapModifiedFieldsFor(source))
	{
		auto srcObj = dynamic_cast<Object *>(source);
		if (nullptr != srcObj)
		{
			modifiedSet = ~srcObj->modifiedSet;
			return true;
		}
	}

	return false;
}

void Object::assignEnd(QObject *, bool)
{
	// do nothing
}

void Object::assignChildren(QObject *source)
{
	if (nullptr != source)
	{
		if (nullptr != prototype)
		{
			if (source == prototype)
			{
				// assign prototype children
				for (auto sourceChild : source->children())
				{
					if (!assignChild(sourceChild))
						newChildFrom(sourceChild, true);
				}
			}

			// add missing prototype children
			for (auto protoChild : prototype->children())
			{
				auto child = findChild<Object *>(
					protoChild->objectName(), Qt::FindDirectChildrenOnly);
				if (nullptr == child)
					newChildFrom(protoChild, true);
			}

			if (source != prototype)
			{
				// assign prototype children
				for (auto sourceChild : source->children())
				{
					if (!assignChild(sourceChild, false))
						newChildFrom(sourceChild, false);
				}
			}
		} else
		{
			for (auto sourceChild : source->children())
			{
				newChildFrom(sourceChild, false);
			}
		}
	}
}

void Object::assignProperties(QObject *) {}

void Object::newChildFrom(QObject *source, bool childProto)
{
	auto newChild = source->metaObject()->newInstance();
	if (nullptr != newChild)
	{
		Q_ASSERT(nullptr != dynamic_cast<Object *>(newChild));
		auto newChildObject = static_cast<Object *>(newChild);

		if (nullptr == prototype || !childProto)
		{
			newChildObject->setObjectName(source->objectName());
			newChildObject->internalAssign(source, true, false);
		} else
		{
			newChildObject->internalSetPrototype(
				dynamic_cast<Object *>(source), true, true);
		}

		newChildObject->loadCounter++;
		newChildObject->setParent(this);
		newChildObject->loadCounter--;
	}
}

const QMetaObject *Object::getStorableChildType(const QString &) const
{
	return nullptr;
}

bool Object::isStorableChildType(const QMetaObject *) const
{
	return false;
}

void Object::childEvent(QChildEvent *event)
{
	QObject::childEvent(event);

	if (event->added() || event->removed())
	{
		auto object = dynamic_cast<Object *>(event->child());

		if (nullptr != object)
		{
			object->beginLoad();
			QObjectList descendants;
			getDescendants(object, descendants);
			for (auto desc : descendants)
			{
				auto obj = dynamic_cast<Object *>(desc);
				if (nullptr != obj)
					obj->beginLoad();
			}
			object->doParentChange();
			for (auto desc : descendants)
			{
				auto obj = dynamic_cast<Object *>(desc);
				if (nullptr != obj)
					obj->endLoad();
			}
			object->endLoad();
		}

		setModified(true);

		if (event->added())
			doAddChild(event->child());
		else if (event->removed())
			doRemoveChild(event->child());
	}
}

void Object::doParentChange()
{
	auto parent = dynamic_cast<Object *>(this->parent());
	if (nullptr != parent)
	{
		auto undoStack = parent->getUndoStack();
		if (nullptr != undoStack)
			setUndoStack(undoStack);
	}
	emit parentChanged();
}

void Object::doFlagsChanged()
{
	emit modifiedFlagChanged(modified);
}

void Object::doAddChild(QObject *object)
{
	emit childAdded(object);
}

void Object::doRemoveChild(QObject *object)
{
	emit childRemoved(object);
}

void Object::connectUndoStack()
{
	if (nullptr != undoStack && ownUndoStack)
	{
		QObject::connect(undoStack, &QUndoStack::cleanChanged, this,
			&Object::onUndoStackCleanChanged);
	}
}

void Object::disconnectUndoStack()
{
	if (nullptr != undoStack && ownUndoStack)
	{
		QObject::disconnect(undoStack, &QUndoStack::cleanChanged, this,
			&Object::onUndoStackCleanChanged);
	}
}

void Object::getDescendants(QObject *obj, QObjectList &out)
{
	Q_ASSERT(nullptr != obj);
	for (auto child : obj->children())
	{
		out.push_back(child);
		getDescendants(child, out);
	}
}
}
