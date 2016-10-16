#pragma once

#include "AbstractObjectGroup.h"
#include "Object.h"

#include <QFileSystemWatcher>

#include <set>
#include <map>

namespace Core
{
	class AbstractFile;
	class ProjectGroup;
	class OpenedFiles : public Object, public AbstractObjectGroup
	{
		Q_OBJECT
	public:
		explicit OpenedFiles(ProjectGroup *owner);
		virtual ~OpenedFiles();

		inline ProjectGroup *getOwner() const;

		bool fileIsOpened(const QString &filePath);
		QObject *getRegisteredFileData(const QString &filePath);
		void registerFile(const QString &filePath, QObject *data);
		QObject *unregisterFile(const QString &filePath, unsigned *ref_count_ptr = nullptr);
		QObject *deleteFileData(const QString &filePath);

		QObject *updateFilePath(const QString &oldFilePath, const QString &newFilePath);
		bool canChangeFilePath(const QString &oldFilePath, const QString &newFilePath);

		virtual const QObjectList &getChildren() override;
		virtual void resetChildren() override;

		bool isFileWatched(const Core::AbstractFile *file) const;
		bool isFileWatched(const QString &filePath) const;

		void watchFile(AbstractFile *file, bool yes);
		void watch(const QString &path, bool yes);

		template <typename CLASS>
		inline void connectFilesChanged(CLASS *object, void (CLASS::*onFilesChanged)(const QString &))
		{
			QObject::connect(this, &OpenedFiles::filesChanged,
							 object, onFilesChanged);
		}

		template <typename CLASS>
		inline void disconnectFilesChanged(CLASS *object, void (CLASS::*onFilesChanged)(const QString &))
		{
			QObject::disconnect(this, &OpenedFiles::filesChanged,
								object, onFilesChanged);
		}

		void clearWatcher();

	signals:
		void filesChanged(const QString &path);

	private slots:
		void onFileDataDestroyed();

	protected:
		virtual void childEvent(QChildEvent *event) override;

	private:
		struct Info
		{
			QObject *data;
			unsigned ref_count;
		};
		typedef std::map<QString, Info> FileMap;

		void resetWatcher(bool copy);
		void addPathInternal(const QString &path);
		void removePathInternal(const QString &path);

		FileMap::iterator findFileData(const QString &filePath);
		FileMap file_map;
		QObjectList m_children;
		QFileSystemWatcher *watcher;
		ProjectGroup *owner;
	};

	ProjectGroup *OpenedFiles::getOwner() const
	{
		return owner;
	}
}
