#include "SearchPaths.h"

#include "AbstractProjectDirectory.h"

namespace Banana
{

	SearchPaths::SearchPaths(AbstractProjectDirectory *project_dir, QObject *parent)
		: QObject(parent)
		, project_dir(project_dir)
	{

	}

	Directory *SearchPaths::registerPath(const QString &path, int order) const
	{
		return project_dir->addSearchPath(path, order, false);
	}

	void SearchPaths::unregisterPath(const QString &path) const
	{
		auto dir = dynamic_cast<Directory *>(project_dir->findFileSystemObject(path, false));
		unregisterDirectory(dir);
	}

	bool SearchPaths::unregisterDirectory(Directory *dir) const
	{
		if (nullptr != dir && dir != project_dir)
		{
			dir->setSearched(false);
			if (dir->children().empty())
			{
				if (nullptr != dynamic_cast<RootDirectory *>(dir))
					delete dir;
			}
			return true;
		}

		return false;
	}

	void SearchPaths::clear()
	{
		unregisterDirsIn(project_dir);
	}

	SearchPaths::DirectoryList SearchPaths::getDirectoryList() const
	{
		auto obj_list = project_dir->getSearchDirectoryList();

		DirectoryList result;
		int count = obj_list.count();
		result.resize(count);
		for (int i = 0; i < count; i++)
			result[i] = static_cast<Directory *>(obj_list.at(i));

		return result;
	}

	void SearchPaths::unregisterDirsIn(Directory *dir) const
	{
		Q_ASSERT(nullptr != dir);

		for (auto obj : dir->children())
		{
			auto dir = dynamic_cast<Directory *>(obj);
			if (nullptr != dir)
			{
				unregisterDirsIn(dir);
				unregisterDirectory(dir);
			}
		}
	}

}
