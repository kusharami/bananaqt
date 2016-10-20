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

#include "AbstractObjectTreeModel.h"

#include "BananaCore/AbstractObjectGroup.h"
#include "BananaCore/Object.h"
#include "BananaCore/Core.h"

#include <QItemSelection>
#include <QItemSelectionModel>

namespace Banana
{

	AbstractObjectTreeModel::AbstractObjectTreeModel(QObject *parent)
		: QAbstractItemModel(parent)
		, noReset(0)
		, resetCount(0)
		, rootGroup(nullptr)
	{
	}

	QObject *AbstractObjectTreeModel::getRootGroup() const
	{
		return rootGroup;
	}

	void AbstractObjectTreeModel::setRootGroup(QObject *group)
	{
		if (rootGroup != group)
		{
			beginResetModel();

			disconnectObject(rootGroup);
			rootGroup = group;
			connectObject(rootGroup);

			endResetModel();
		}
	}

	QVariant AbstractObjectTreeModel::data(const QModelIndex &index, int role) const
	{
		if (index.isValid())
		{
			auto item = getItemAt(index);
			if (nullptr != item)
			{
				switch (role)
				{
					case Qt::DisplayRole:
					case Qt::EditRole:
						return getTextForItem(item, role);

					case Qt::DecorationRole:
						return getIconForItem(item);

					case Qt::FontRole:
					{
						QFont font(getFontForItem(item));

						font.setStrikeOut(!getIsValidForItem(item));

						return font;
					}

					case Qt::TextColorRole:
					{
						if (!getIsValidForItem(item))
							return QColor(Qt::red);

					}	break;

					case Qt::ToolTipRole:
						return getToolTipForItem(item);

					default:
						break;
				}
			}
		}
		return QVariant();
	}

	Qt::ItemFlags AbstractObjectTreeModel::flags(const QModelIndex &index) const
	{
		if (false == index.isValid())
			return Qt::ItemIsEnabled;

		Qt::ItemFlags result = QAbstractItemModel::flags(index);

		auto item = getItemAt(index);
		if (nullptr != item)
			result |= getItemFlagsForItem(item);

		return result;
	}

	QModelIndex AbstractObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
	{
		if (hasIndex(row, column, parent))
		{
			auto parentGroup = dynamic_cast<AbstractObjectGroup *>(getItemAt(parent));

			if (nullptr != parentGroup)
			{
				auto siblings = getGroupChildren(parentGroup);
				auto sibling = siblings.at(row);
				auto index = createIndex(row, column, siblings.at(row));

				(*const_cast<IndexMap *>(&indexMap))[sibling] = index;

				return index;
			}
		}

		return QModelIndex();
	}

	QModelIndex AbstractObjectTreeModel::parent(const QModelIndex &index) const
	{
		auto item = getItemAt(index);

		if (nullptr != item)
		{
			auto itemParent = item->parent();
			auto parentGroup = dynamic_cast<AbstractObjectGroup *>(itemParent);
			if (nullptr != parentGroup)
			{
				parentGroup = parentGroup->getRealGroup();
				Q_ASSERT(nullptr != parentGroup);
				itemParent = dynamic_cast<QObject *>(parentGroup);
				Q_ASSERT(nullptr != itemParent);
				auto parentParent = dynamic_cast<AbstractObjectGroup *>(itemParent->parent());
				if (nullptr != parentParent)
				{
					auto index = createIndex(getChildIndex(parentParent, itemParent), 0, itemParent);

					(*const_cast<IndexMap *>(&indexMap))[itemParent] = index;

					return index;
				}
			}
		}

		return QModelIndex();
	}

	int AbstractObjectTreeModel::rowCount(const QModelIndex &parent) const
	{
		auto parent_group = dynamic_cast<AbstractObjectGroup *>(getItemAt(parent));

		if (nullptr != parent_group)
		{
			return getGroupChildren(parent_group).count();
		}

		return 0;
	}

	int AbstractObjectTreeModel::columnCount(const QModelIndex &) const
	{
		return 1;
	}

	bool AbstractObjectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
	{
		if (Qt::EditRole == role)
		{
			auto item = getItemAt(index);
			if (nullptr != item)
			{
				setTextForItem(item, value.toString());
				return true;
			}
		}

		return false;
	}

	bool AbstractObjectTreeModel::insertColumns(int, int, const QModelIndex &)
	{
		return false;
	}

	bool AbstractObjectTreeModel::removeColumns(int, int, const QModelIndex &)
	{
		return false;
	}

	bool AbstractObjectTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
	{
		// TODO
		return false;
	}

	bool AbstractObjectTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
	{
		// TODO
		return false;
	}

	Qt::DropActions AbstractObjectTreeModel::supportedDropActions() const
	{
		return Qt::MoveAction;
	}

	Qt::DropActions AbstractObjectTreeModel::supportedDragActions() const
	{
		return Qt::MoveAction;
	}

	bool AbstractObjectTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
	{
		// TODO

		return false;
	}

	QMimeData *AbstractObjectTreeModel::mimeData(const QModelIndexList &indexes) const
	{
		// TODO

		return nullptr;
	}

	QStringList AbstractObjectTreeModel::mimeTypes() const
	{
		// TODO

		return QStringList();
	}

	bool AbstractObjectTreeModel::canDeleteItems(const QModelIndexList &indexes) const
	{
		for (auto &index : indexes)
		{
			if (!canDeleteItem(getItemAt(index)))
				return false;
		}

		return true;
	}

	QModelIndex AbstractObjectTreeModel::findModelIndex(QObject *item) const
	{
		if (nullptr != item)
		{
			auto it = indexMap.find(item);
			if (indexMap.end() != it)
				return it->second;

			auto itemParent = item->parent();
			if (nullptr != itemParent
			&&	item != rootGroup)
			{
				auto parentGroup = dynamic_cast<AbstractObjectGroup *>(itemParent);
				if (nullptr != parentGroup)
				{
					parentGroup = parentGroup->getRealGroup();
					Q_ASSERT(nullptr != parentGroup);
					itemParent = dynamic_cast<QObject *>(parentGroup);
					Q_ASSERT(nullptr != itemParent);
					return index(getChildIndex(parentGroup, item), 0,
								 findModelIndex(itemParent));
				}
			}
		}

		return QModelIndex();
	}

	void AbstractObjectTreeModel::doConnectObject(QObject *object)
	{
		auto obj = dynamic_cast<Object *>(object);
		if (nullptr != obj)
		{
			QObject::connect(obj, &Object::beforeDestroy,
							 this, &AbstractObjectTreeModel::onBeforeObjectDestroy);
		} else
		{
			QObject::connect(object, &QObject::destroyed,
							 this, &AbstractObjectTreeModel::onObjectDestroyed);
		}

		QObject::connect(object, &QObject::objectNameChanged,
						 this, &AbstractObjectTreeModel::onObjectNameChanged);

		if (nullptr != obj)
		{
			QObject::connect(obj, &Object::childAdded,
							 this, &AbstractObjectTreeModel::onChildAdded);
			QObject::connect(obj, &Object::childRemoved,
							 this, &AbstractObjectTreeModel::onChildRemoved);
		}

		for (auto child : object->children())
		{
			connectObject(child);
		}
	}

	void AbstractObjectTreeModel::doDisconnectObject(QObject *object)
	{
		auto obj = dynamic_cast<Object *>(object);
		if (nullptr != obj)
		{
			QObject::disconnect(obj, &Object::beforeDestroy,
							 this, &AbstractObjectTreeModel::onBeforeObjectDestroy);
		} else
		{
			QObject::disconnect(object, &QObject::destroyed,
							 this, &AbstractObjectTreeModel::onObjectDestroyed);
		}

		QObject::disconnect(object, &QObject::objectNameChanged,
							this, &AbstractObjectTreeModel::onObjectNameChanged);

		if (nullptr != obj)
		{
			QObject::disconnect(obj, &Object::childAdded,
								this, &AbstractObjectTreeModel::onChildAdded);
			QObject::disconnect(obj, &Object::childRemoved,
								this, &AbstractObjectTreeModel::onChildRemoved);
		}

		indexMap[object] = QModelIndex();

		for (auto child : object->children())
		{
			disconnectObject(child, true);
		}
	}

	void AbstractObjectTreeModel::selectItems(const QObjectSet &items)
	{
		QItemSelection selection;
		for (auto item : items)
		{
			auto index = findModelIndex(item);
			if (index.isValid())
			{
				selection.select(index, index);
			}
		}

		emit shouldSelect(selection);
	}

	void AbstractObjectTreeModel::onBeforeObjectDestroy(QObject *object)
	{
		beginResetModel();

		auto parentGroup = dynamic_cast<AbstractObjectGroup *>(object->parent());
		if (nullptr != parentGroup)
			parentGroup->resetChildren();

		doDisconnectObject(object);

		endResetModel();

		if (object == rootGroup)
		{
			rootGroup = nullptr;
		}
	}

	void AbstractObjectTreeModel::onObjectDestroyed(QObject *object)
	{
		if (object == rootGroup)
		{
			rootGroup = nullptr;
		}

		beginResetModel();
		endResetModel();
	}

	void AbstractObjectTreeModel::onChildAdded(QObject *object)
	{
		connectObject(object);
	}

	void AbstractObjectTreeModel::onChildRemoved(QObject *object)
	{
		disconnectObject(object);
	}

	void AbstractObjectTreeModel::onObjectNameChanged()
	{
		beginResetModel();
		endResetModel();
	}

	void AbstractObjectTreeModel::connectObject(QObject *object)
	{
		if (isSupportedItem(object))
		{
			beginResetModel();

			doConnectObject(object);

			endResetModel();
		}
	}

	void AbstractObjectTreeModel::disconnectObject(QObject *object, bool parent_disconnect)
	{
		if (isSupportedItem(object))
		{
			if (!parent_disconnect)
			{
				beginResetModel();
			}

			doDisconnectObject(object);

			if (!parent_disconnect)
			{
				endResetModel();
			}
		}
	}

	int AbstractObjectTreeModel::getChildIndex(AbstractObjectGroup *group, QObject *child) const
	{
		if (0 == getFiltersCount())
			return group->getChildIndex(child);

		auto children = getGroupChildren(group);

		auto it = std::find(children.begin(), children.end(), child);

		if (children.end() != it)
			return it - children.begin();

		return -1;
	}

	QObjectList AbstractObjectTreeModel::getGroupChildren(AbstractObjectGroup *group) const
	{
		auto no_const = const_cast<AbstractObjectTreeModel *>(this);
		no_const->noReset++;
		auto result = group->filterChildren(0 == getFiltersCount() ? nullptr : this);
		no_const->noReset--;
		return result;
	}

	void AbstractObjectTreeModel::beforeChangeFilters()
	{
		beginResetModel();
	}

	void AbstractObjectTreeModel::afterChangeFilters()
	{
		endResetModel();
	}

	QObject *AbstractObjectTreeModel::getItemAt(const QModelIndex &index) const
	{
		if (index.isValid())
			return static_cast<QObject *>(index.internalPointer());

		return rootGroup;
	}

	void AbstractObjectTreeModel::beginResetModel()
	{
		if (noReset == 0)
		{
			indexMap.clear();
			if (resetCount == 0)
			{
				emit beforeModelReset();
				QAbstractItemModel::beginResetModel();
			}
			resetCount++;
		}
	}

	void AbstractObjectTreeModel::endResetModel()
	{
		if (noReset == 0)
		{
			Q_ASSERT(resetCount > 0);
			if (--resetCount == 0)
			{
				QAbstractItemModel::endResetModel();
				emit afterModelReset();
			}
		}
	}

	UndoStack *AbstractObjectTreeModel::getUndoStack() const
	{
		return nullptr;
	}

}
