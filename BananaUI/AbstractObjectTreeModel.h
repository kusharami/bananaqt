#pragma once

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
		virtual QModelIndex index(int row, int column,
						  const QModelIndex &parent = QModelIndex()) const override;
		virtual QModelIndex parent(const QModelIndex &index) const override;
		virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
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
							  int row, int column, const QModelIndex &parent) override;
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

		void selectItems(const std::set<QObject *> &items);

	signals:
		void shouldClearSelection();
		void shouldSelect(const QItemSelection &selection);
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
