#pragma once

#include "AbstractObjectTreeModel.h"

#include <QIcon>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QFileIconProvider>

#include <map>
#include <functional>

namespace Banana
{

	class AbstractProjectDirectory;
	class Directory;
	class AbstractFile;
	class AbstractFileSystemObject;
	class BaseFileManager;

	class ProjectDirectoryModel
		: public QFileSystemModel
			, public QFileIconProvider
	{
		Q_OBJECT

	public:
		struct FileTypeInfo
		{
			QIcon icon;
		};

		explicit ProjectDirectoryModel(QObject *parent = nullptr);

		inline AbstractProjectDirectory *getProjectDirectory() const;
		QModelIndex setProjectDirectory(AbstractProjectDirectory *dir);

		void setFileManager(Banana::BaseFileManager *manager);
		BaseFileManager *getFileManager() const;

		virtual int columnCount(const QModelIndex &parent) const override;

		virtual QIcon icon(const QFileInfo &info) const override;

		virtual QVariant data(const QModelIndex &index, int role) const override;

		virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

		virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
									 int row, int column, const QModelIndex &parent) override;

		virtual Qt::DropActions supportedDragActions() const override;
		virtual Qt::DropActions supportedDropActions() const override;
		virtual Qt::DropAction convertDropAction(const QMimeData *mimeData,
												 Qt::DropAction action) const;

		void setFileTypeInfo(const QString &extension, const FileTypeInfo &info);
		void setPictureMetaObject(const QMetaObject *metaObject);

		bool canOpenFiles(const QStringList &filePaths) const;
		bool canOpenFile(AbstractFile *file) const;
		bool canOpenFile(const QMetaObject *fileMetaObject) const;
		bool canOpenFile(const QString &filePath) const;
		void openFiles(const QStringList &filePaths) const;

	signals:
		void changeFilePathError(const QString &path, const QString &failed_path);
		void changeDirPathError(const QString &path, const QString &failed_path);
		void projectDirectoryChanged();
		void openFile(const QString &filePath) const;
		void locateFile(const QString &filePath) const;

	private slots:
		void onProjectDirectoryDestroyed();

	private:
		static QString getToolTipForFile(const QFileInfo &info);

		QModelIndex updateActiveProjectDirectory();
		void connectProjectDirectory();
		void disconnectProjectDirectory();

		std::map<QString, FileTypeInfo> file_types_info;

		AbstractProjectDirectory *projectDir;
		BaseFileManager *fileManager;
		const QMetaObject *pictureMetaObject;
	};

	AbstractProjectDirectory *ProjectDirectoryModel::getProjectDirectory() const
	{
		return projectDir;
	}

}
