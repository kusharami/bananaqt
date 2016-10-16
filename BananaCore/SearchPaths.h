#pragma once

#include <QObject>

namespace Core
{
	class AbstractProjectDirectory;
	class Directory;
	class SearchPaths : public QObject
	{
		Q_OBJECT

	public:
		explicit SearchPaths(Core::AbstractProjectDirectory *project_dir, QObject *parent = nullptr);

		Directory *registerPath(const QString &path, int order) const;
		void unregisterPath(const QString &path) const;
		bool unregisterDirectory(Directory *dir) const;
		void clear();

		typedef std::vector<Directory *> DirectoryList;

		DirectoryList getDirectoryList() const;

		inline Core::AbstractProjectDirectory *getProjectDirectory() const;

	private:
		void unregisterDirsIn(Core::Directory *dir) const;

		Core::AbstractProjectDirectory *project_dir;
	};

	Core::AbstractProjectDirectory *SearchPaths::getProjectDirectory() const
	{
		return project_dir;
	}
}

Q_DECLARE_METATYPE(Core::SearchPaths *)
