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

#include "BananaCore/ContainerTypes.h"
#include "BananaCore/ChildFilter.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QFont>
#include <QColor>
#include <QObjectList>

#include <map>
#include <set>

struct QMetaObject;
class QItemSelection;

namespace Banana
{
class UndoStack;
class AbstractObjectGroup;

class AbstractObjectTreeModel : public QAbstractItemModel, public ChildFilter
{
	Q_OBJECT

public:
	explicit AbstractObjectTreeModel(QObject *parent = nullptr);

	QObject *getRootGroup() const;
	void setRootGroup(QObject *group);

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QModelIndex index(
		int row, int column, const QModelIndex &parent =
			QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex &index) const override;
	virtual int rowCount(
		const QModelIndex &parent =
			QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value,
						 int role = Qt::EditRole) override;
	virtual bool insertColumns(int, int, const QModelIndex &) override;
	virtual bool removeColumns(int, int, const QModelIndex &) override;
	virtual bool insertRows(int position, int rows,
							const QModelIndex &parent = QModelIndex()) override;
	virtual bool removeRows(int position, int rows,
							const QModelIndex &parent = QModelIndex()) override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::DropActions supportedDragActions() const override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
							  int row, int column,
							  const QModelIndex &parent) override;
	virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
	virtual QStringList mimeTypes() const override;

	virtual void copyToClipboard(const QModelIndexList &indexes, bool cut) = 0;
	virtual void pasteFromClipboard(const QModelIndexList &indexes) = 0;
	virtual void deleteItems(const QModelIndexList &indexes) = 0;
	virtual bool canDeleteItem(QObject *item) const = 0;
	bool canDeleteItems(const QModelIndexList &indexes) const;

	QModelIndex findModelIndex(QObject *item) const;
	QObject *getItemAt(const QModelIndex &index) const;
	QObjectList getGroupChildren(AbstractObjectGroup *group) const;

	virtual void beginResetModel();
	virtual void endResetModel();

	virtual UndoStack *getUndoStack() const;

protected:
	virtual void beforeChangeFilters() override;
	virtual void afterChangeFilters() override;

	virtual bool isSupportedItem(QObject *item) const = 0;
	virtual Qt::ItemFlags getItemFlagsForItem(QObject *item) const = 0;
	virtual bool getIsValidForItem(QObject *item) const = 0;
	virtual QFont getFontForItem(QObject *item) const = 0;
	virtual QIcon getIconForItem(QObject *item) const = 0;
	virtual QString getToolTipForItem(QObject *item) const = 0;
	virtual QString getTextForItem(QObject *item, int role) const = 0;
	virtual void setTextForItem(QObject *item, const QString &text) = 0;

	virtual void doConnectObject(QObject *object);
	virtual void doDisconnectObject(QObject *object);

	void selectItems(const QObjectSet &items);

signals:
	void dropSuccess();
	void shouldClearSelection();
	void shouldSelect(const QItemSelection &selection);
	void beforeModelReset();
	void afterModelReset();

private slots:
	void onBeforeObjectDestroy(QObject *object);
	void onObjectDestroyed(QObject *object);
	void onChildAdded(QObject *object);
	void onChildRemoved(QObject *object);
	void onObjectNameChanged();

private:
	int getChildIndex(AbstractObjectGroup *group, QObject *child) const;

protected:
	void connectObject(QObject *object);
	void disconnectObject(QObject *object, bool parent_disconnect = false);

	typedef std::map<QObject *, QModelIndex> IndexMap;

	unsigned noReset;
	unsigned resetCount;
	QObject *rootGroup;
	IndexMap indexMap;
};

}
