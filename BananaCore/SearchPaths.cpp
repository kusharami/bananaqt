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
	auto dir =
		dynamic_cast<Directory *>(project_dir->findFileSystemObject(
									  path,
									  false));
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
	{
		result[i] = static_cast<Directory *>(obj_list.at(i));
	}

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
