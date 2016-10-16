#pragma once

#include <QSortFilterProxyModel>

#include <vector>

namespace Banana
{
	class ProjectDirectoryModel;
	class AbstractProjectFile;
	class ProjectDirectoryFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT

	public:
		explicit ProjectDirectoryFilterModel(QObject *parent = nullptr);

		virtual void setSourceModel(QAbstractItemModel *sourceModel) override;
		virtual QVariant data(const QModelIndex &index, int role) const override;

		bool shouldHideIgnored() const;
		void setHideIgnored(bool hide);

		const QRegExp &getIgnoreFilter() const;
		void setIgnoreFilter(const QRegExp &re);

		bool isReadOnly() const;
		void setReadOnly(bool yes);

		void showDirectoriesOnly(bool show);
		void setFileTypeFilter(const QMetaObject *meta_object);
		void setFilterRegExp(const QRegExp &re);

	private slots:
		void onSourceModelDestroyed();
		void onProjectFileDestroyed();
		void applyProjectFile();
		void applyFilters();

	protected:
		virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
		virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
		virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

		virtual Qt::DropActions supportedDropActions() const override;
		virtual Qt::DropActions supportedDragActions() const override;

	private:
		void connectSourceModel();
		void disconnectSourceModel();
		void connectProjectFile();
		void disconnectProjectFile();

		std::vector<const char *> show_extensions;
		ProjectDirectoryModel *project_tree_model;
		AbstractProjectFile *project_file;
		QRegExp ignore;
		bool hideIgnored;
		bool readOnly;
		bool showDirsOnly;
	};
}
