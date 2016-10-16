#pragma once

#include <QObject>

namespace Banana
{
	class AbstractProjectDirectory;
	class Directory;
	class SearchPaths : public QObject
	{
		Q_OBJECT

	public:
		explicit SearchPaths(Banana::AbstractProjectDirectory *project_dir, QObject *parent = nullptr);

		Directory *registerPath(const QString &path, int order) const;
		void unregisterPath(const QString &path) const;
		bool unregisterDirectory(Directory *dir) const;
		void clear();

		typedef std::vector<Directory *> DirectoryList;

		DirectoryList getDirectoryList() const;

		inline Banana::AbstractProjectDirectory *getProjectDirectory() const;

	private:
		void unregisterDirsIn(Banana::Directory *dir) const;

		Banana::AbstractProjectDirectory *project_dir;
	};

	Banana::AbstractProjectDirectory *SearchPaths::getProjectDirectory() const
	{
		return project_dir;
	}
}

Q_DECLARE_METATYPE(Banana::SearchPaths *)
