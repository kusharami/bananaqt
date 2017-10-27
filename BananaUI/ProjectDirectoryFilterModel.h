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
	virtual bool filterAcceptsColumn(
		int source_column, const QModelIndex &source_parent) const override;
	virtual bool filterAcceptsRow(
		int source_row, const QModelIndex &source_parent) const override;
	virtual bool lessThan(const QModelIndex &source_left,
		const QModelIndex &source_right) const override;

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
