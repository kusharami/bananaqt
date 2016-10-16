#include "SearchPathsDialog.h"

#include "FileSelectDialog.h"
#include "CheckBoxHeader.h"
#include "ProjectDirectoryModel.h"
#include "Config.h"
#include "Utils.h"

#include "BananaCore/Directory.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/SearchPaths.h"

#include <QMessageBox>
#include <QCoreApplication>
#include <QMenu>
#include <QClipboard>
#include <QMimeData>
#include <QFileInfo>
#include <QUrl>
#include <QMouseEvent>

using namespace Banana;
#include "ui_SearchPathsDialog.h"

namespace Banana
{

static const QString kDragDropSearchPaths("Banana/DragDropSearchPaths");
static const QStringList kDragDropMimeTypes(
{
	"text/plain",
	"text/uri-list",
	kDragDropSearchPaths
});

SearchPathsDialog::SearchPathsDialog(ProjectDirectoryModel *project_dir_model,
									 QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::SearchPathsDialog)
	, project_dir_model(project_dir_model)
	, paths_model(new SearchPathsTableModel(project_dir_model, this))
	, popup(false)
{
	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint))
				   | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);

	QObject::connect(project_dir_model, &ProjectDirectoryModel::projectDirectoryChanged,
					 this, &SearchPathsDialog::onProjectDirectoryChanged);

	QObject::connect(paths_model, &SearchPathsTableModel::shouldReselect,
					 this, &SearchPathsDialog::applySavedPathSelection);

	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableView->setModel(paths_model);

	QObject::connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
					 this, &SearchPathsDialog::onSelectionChanged);

#ifdef Q_OS_MAC
	Utils::addShortcutForAction(this, QKeySequence(Qt::Key_Backspace), ui->actionUnregisterSelected);
#else
	Utils::addShortcutForAction(this, ui->actionUnregisterSelected->shortcut(), ui->actionUnregisterSelected);
#endif
}

SearchPathsDialog::~SearchPathsDialog()
{
	delete ui;
}

void SearchPathsDialog::accept()
{
	if (popup)
		return;

	QDialog::accept();
}

void SearchPathsDialog::reject()
{
	if (popup)
		return;

	QDialog::reject();
}

void SearchPathsDialog::onProjectDirectoryChanged()
{
	ui->tableView->setModel(nullptr);
	delete paths_model;
	paths_model = nullptr;

	reject();
}

void SearchPathsDialog::onSelectionChanged()
{
	auto sel_model = ui->tableView->selectionModel();
	bool enabled = (nullptr != sel_model && sel_model->selectedIndexes().count() > 0);
	ui->unregisterButton->setEnabled(enabled);
	ui->actionUnregisterSelected->setEnabled(enabled);
}

void SearchPathsDialog::applySavedPathSelection()
{
	auto sel_model = ui->tableView->selectionModel();
	if (sel_model != nullptr)
	{
		QItemSelection selection;
		paths_model->getSavedSelection(selection);
		auto indexes = selection.indexes();
		if (indexes.count() > 0)
		{
			ui->tableView->setCurrentIndex(selection.indexes().at(0));
			sel_model->select(selection, QItemSelectionModel::Select);
		}
	}
}

void SearchPathsDialog::on_registerButton_clicked()
{
	FileSelectDialog dialog(project_dir_model, this);

	if (dialog.executeMultiSelect(&Directory::staticMetaObject))
	{
		if (nullptr != paths_model)
		{
			paths_model->registerPaths(dialog.getSelectedFilePathList());
		}
	}
}

void SearchPathsDialog::on_unregisterButton_clicked()
{
	ui->actionUnregisterSelected->trigger();
}

void SearchPathsDialog::on_closeButton_clicked()
{
	reject();
}

SearchPathsTableModel::SearchPathsTableModel(ProjectDirectoryModel *project_dir_model,
										   QObject *parent)
	: QAbstractTableModel(parent)
	, project_dir_model(project_dir_model)
	, internalDrop(false)
{
	load();
}

SearchPathsTableModel::~SearchPathsTableModel()
{
	clear();
}

int SearchPathsTableModel::rowCount(const QModelIndex &) const
{
	return static_cast<int>(dirs.size());
}

int SearchPathsTableModel::columnCount(const QModelIndex &) const
{
	return 1;
}

QVariant SearchPathsTableModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid())
	{
		auto it = dirs.begin() + index.row();
		Directory *dir = *it;
		Q_ASSERT(nullptr != dir);

		switch (role)
		{
			case Qt::DisplayRole:
				return dir->getFilePath(project_dir_model->getProjectDirectory());

			case Qt::ToolTipRole:
				return dir->getFilePath();

			default:
				break;
		}
	}

	return QVariant();
}

void SearchPathsTableModel::registerPaths(const QStringList &paths, int order)
{
	beginResetModel();

	if (order < 0)
		order = static_cast<int>(dirs.size());

	disconnectDirs();
	dirs_to_select.clear();

	if (nullptr != project_dir_model)
	{
		auto project_dir = project_dir_model->getProjectDirectory();
		if (nullptr != project_dir)
		{
			for (auto &path : paths)
			{
				QFileInfo info(path);
				if (info.exists())
				{
					if (!info.isDir())
						info.setFile(info.path());
					auto dir = project_dir->addSearchPath(info.filePath(), order, false);
					if (nullptr != dir && dir != project_dir)
					{
						auto found_it = std::find(dirs.begin(), dirs.end(), dir);

						int insert_order = order;
						if (found_it == dirs.end())
							order++;
						else
						{
							if (found_it - dirs.begin() < insert_order)
								insert_order--;
							dirs.erase(found_it);
						}

						dirs.insert(dirs.begin() + insert_order, dir);
						dirs_to_select.push_back(dir);
					}
				}
			}
		}
	}

	connectDirs();

	endResetModel();

	emit shouldReselect();

	dirs_to_select.clear();
}

Qt::ItemFlags SearchPathsTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled;
}

void SearchPathsTableModel::unregisterPaths(const QModelIndexList &indexes)
{
	beginResetModel();

	disconnectDirs();

	for (auto &index : indexes)
	{
		auto dir = dirs.at(index.row());
		dir->setSearched(false);
		if (dir->children().empty())
		{
			if (nullptr != dynamic_cast<RootDirectory *>(dir))
				delete dir;
		}
	}

	dirs.clear();

	load();
	endResetModel();
}

Directory *SearchPathsTableModel::getDirectoryAtIndex(const QModelIndex &index) const
{
	if (index.isValid())
		return dirs.at(index.row());

	return nullptr;
}

bool SearchPathsTableModel::dropMimeData(const QMimeData *data,
										 Qt::DropAction action,
										 int row,
										 int,
										 const QModelIndex &parent)
{
	switch (action)
	{
		case Qt::IgnoreAction:
			return true;

		case Qt::MoveAction:
		case Qt::CopyAction:
		{
			if (data->hasFormat(kDragDropSearchPaths))
			{
				internalDrop = true;
				auto bytes = data->data(kDragDropSearchPaths);

				int items_count = bytes.size() / sizeof(uintptr_t);

				if (items_count > 0)
				{
					if (row == -1)
						row = rowCount();

					uintptr_t *item_ptr = (uintptr_t *) bytes.data();
					uintptr_t *end_ptr = &item_ptr[items_count];

					dirs_to_select.clear();

					beginInsertRows(parent, row, row + items_count - 1);

					for (auto ptr = item_ptr; ptr < end_ptr; ptr++, row++)
					{
						auto dir = static_cast<Directory *>(reinterpret_cast<void *>(*ptr));
						dirs.insert(dirs.begin() + row, dir);

					}

					endInsertRows();

					return true;
				}
			} else
			{
				QStringList paths;
				if (data->hasUrls())
				{
					for (auto &url : data->urls())
					{
						if (url.isLocalFile())
							paths.push_back(url.toLocalFile());
					}
				} else
				if (data->hasText())
				{
					paths = data->text().split(QRegExp("[\n\r]+"), QString::SkipEmptyParts);
				}

				if (!paths.isEmpty())
				{
					registerPaths(paths, row);
				}
			}
		}	break;

		default:
			break;
	}

	return false;
}

Qt::DropActions SearchPathsTableModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions SearchPathsTableModel::supportedDragActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

QMimeData *SearchPathsTableModel::mimeData(const QModelIndexList &indexes) const
{
	auto thiz = const_cast<SearchPathsTableModel *>(this);
	thiz->internalDrop = false;

	auto result = new QMimeData;

	QByteArray bytes;

	auto sorted_indexes = indexes;
	std::sort(sorted_indexes.begin(), sorted_indexes.end(),
	[](const QModelIndex &a, const QModelIndex &b) -> bool
	{
		return a.row() < b.row();
	});

	int items_count = sorted_indexes.size();
	bytes.resize(items_count* sizeof(uintptr_t));

	uintptr_t *item_ptr = (uintptr_t *) bytes.data();
	QStringList paths;

	for (int i = 0; i < items_count; i++)
	{
		auto &index = sorted_indexes.at(i);

		auto dir = dirs.at(index.row());
		*item_ptr++ = (uintptr_t) dir;
		paths.push_back(dir->getFilePath());
	}

	result->setData(kDragDropSearchPaths, bytes);
	result->setText(paths.join('\n'));

	return result;
}

QStringList SearchPathsTableModel::mimeTypes() const
{
	return kDragDropMimeTypes;
}

void SearchPathsTableModel::getSavedSelection(QItemSelection &selection) const
{
	for (auto dir : dirs_to_select)
	{
		auto it = std::find(dirs.begin(), dirs.end(), dir);

		if (it != dirs.end())
		{
			auto index = this->index(it - dirs.begin(), 0);
			selection.select(index, index);
		}
	}
}

void SearchPathsTableModel::onDirectoryDestroyed(QObject *dir)
{
	beginResetModel();

	auto it = std::find(dirs.begin(), dirs.end(), static_cast<Directory *>(dir));
	if (it != dirs.end())
		dirs.erase(it);

	endResetModel();
}

bool SearchPathsTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (count > 0 && internalDrop)
	{
		dirs_to_select.clear();

		beginRemoveRows(parent, row, row + count - 1);

		auto first_it = dirs.begin() + row;
		auto last_it = first_it + count;
		for (auto it = first_it; it != last_it; ++it)
			dirs_to_select.push_back(*it);

		dirs.erase(first_it, last_it);

		for (size_t i = 0, count = dirs.size(); i < count; i++)
		{
			auto dir = dirs[i];
			dir->setSearchOrder(i);
		}

		endRemoveRows();
		emit shouldReselect();
	}

	dirs_to_select.clear();

	return true;
}

void SearchPathsTableModel::load()
{
	dirs.clear();
	if (nullptr != project_dir_model)
	{
		auto project_dir = project_dir_model->getProjectDirectory();
		if (nullptr != project_dir)
		{
			auto project_file = project_dir->getProjectFile();
			if (nullptr != project_file)
			{
				auto search_paths = project_dir->getProjectFile()->getSearchPaths();
				dirs = search_paths->getDirectoryList();
			}
		}
	}

	connectDirs();
}

void SearchPathsTableModel::clear()
{
	disconnectDirs();

	dirs.clear();
	dirs_to_select.clear();
}

void SearchPathsTableModel::connectDirs()
{
	for (size_t i = 0, count = dirs.size(); i < count; i++)
		connectDirectory(dirs.at(i), i);
}

void SearchPathsTableModel::connectDirectory(Directory *dir, int order)
{
	Q_ASSERT(nullptr != dir);

	QObject::connect(dir, &QObject::destroyed, this, &SearchPathsTableModel::onDirectoryDestroyed);
	dir->setSearchOrder(order);
}

void SearchPathsTableModel::disconnectDirectory(Directory *dir)
{
	Q_ASSERT(nullptr != dir);

	QObject::disconnect(dir, &QObject::destroyed, this, &SearchPathsTableModel::onDirectoryDestroyed);
}

void SearchPathsTableModel::disconnectDirs()
{
	for (auto dir : dirs)
		disconnectDirectory(dir);
}

void SearchPathsDialog::on_tableView_customContextMenuRequested(const QPoint &pos)
{
	auto view = ui->tableView;
	auto sel_model = view->selectionModel();

	if (nullptr == sel_model)
		return;

	auto indexes = sel_model->selectedIndexes();

	bool shouldPopup = false;
	if (indexes.count() > 0)
	{
		for (auto &index : indexes)
		{
			if (view->visualRect(index).contains(pos, true))
				shouldPopup = true;
		}
	}

	if (shouldPopup)
	{
		QMenu menu(this);

		if (indexes.count() == 1)
		{
			ui->actionCopyAbsolutePath->setText(tr("Copy Absolute Path"));
			ui->actionCopyDirName->setText(tr("Copy Directory Name"));
		} else
		{
			ui->actionCopyAbsolutePath->setText(tr("Copy Selected Absolute Paths"));
			ui->actionCopyDirName->setText(tr("Copy Names of Selected Directories"));
		}

		menu.addAction(ui->actionCopyAbsolutePath);
		menu.addAction(ui->actionCopyDirName);
		menu.addSeparator();
		menu.addAction(ui->actionUnregisterSelected);

		popup = true;
		menu.exec(ui->tableView->mapToGlobal(pos));
		popup = false;
	}
}

void SearchPathsDialog::on_actionCopyAbsolutePath_triggered()
{
	auto dirs = getSelectedDirs();
	if (!dirs.empty())
	{
		auto mime = new QMimeData;

		QStringList list;

		for (auto dir : dirs)
			list.push_back(dir->getFilePath());

		mime->setText(list.join('\n'));

		QApplication::clipboard()->setMimeData(mime);
	}
}

void SearchPathsDialog::on_actionCopyDirName_triggered()
{
	auto dirs = getSelectedDirs();
	if (!dirs.empty())
	{
		auto mime = new QMimeData;

		QStringList list;

		for (auto dir : dirs)
			list.push_back(dir->getFileName());

		mime->setText(list.join('\n'));

		QApplication::clipboard()->setMimeData(mime);
	}
}

std::vector<Directory *> SearchPathsDialog::getSelectedDirs() const
{
	std::vector<Directory *> result;
	auto sel_model = ui->tableView->selectionModel();

	if (nullptr != sel_model)
	{
		for (auto &index : sel_model->selectedIndexes())
			result.push_back(paths_model->getDirectoryAtIndex(index));
	}

	return result;
}

void SearchPathsDialog::on_actionUnregisterSelected_triggered()
{
	auto sel_model = ui->tableView->selectionModel();
	if (sel_model != nullptr)
	{
		auto indexes = sel_model->selectedIndexes();
		if (indexes.count() > 0)
		{
			if (QMessageBox::question(this, QCoreApplication::applicationName(),
									  tr("Are you sure you want to unregister selected paths?"),
									  QMessageBox::Yes | QMessageBox::No,
									  QMessageBox::Yes) == QMessageBox::Yes)
			{
				paths_model->unregisterPaths(indexes);
			}
		}
	}
}
}
