#pragma once

#include <QTreeView>

#include <set>

namespace Banana
{
	class AbstractObjectTreeModel;


class BaseTreeView : public QTreeView
{
	Q_OBJECT

public:
	explicit BaseTreeView(Banana::AbstractObjectTreeModel *model,
						  QWidget *parent = nullptr);

	void select(QObject *item, bool expand = false);
	void expandItem(QObject *item);

	QObject *getCurrentItem() const;

	bool hasItems() const;

	void cutToClipboard();
	void copyToClipboard();
	void pasteFromClipboard();
	void deleteSelectedItems();

	bool canDeleteItem(QObject *item) const;
	bool canDeleteSelectedItems() const;

	void preventReselect(bool prevent);

protected slots:
	virtual void onAfterModelReset();

private slots:
	void onShouldSelect(const QItemSelection &selection);
	void onExpanded(const QModelIndex &index);
	void onExpandedItemDestroyed(QObject *item);
	void onSelectedItemDestroyed(QObject *item);
	void onCollapsed(const QModelIndex &index);
	void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
	Banana::AbstractObjectTreeModel *treeModel;
	typedef std::set<QObject *> ItemSet;
	ItemSet expandedItems;
	ItemSet selectedItems;

	unsigned preventReselectCounter;
};
}
