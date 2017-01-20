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

#include <QObject>

namespace Banana
{
class AbstractProjectDirectory;
class Directory;
class SearchPaths : public QObject
{
	Q_OBJECT

public:
	explicit SearchPaths(Banana::AbstractProjectDirectory *project_dir,
						 QObject *parent = nullptr);

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
