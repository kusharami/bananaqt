/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#include "ProjectTreeScriptMenuBuilder.h"

#include "ProjectTreeView.h"
#include "ProjectDirectoryModel.h"

#include "BananaCore/AbstractProjectDirectory.h"

namespace Banana
{
ProjectTreeScriptMenuBuilder::ProjectTreeScriptMenuBuilder(
	ProjectTreeView *view)
	: mView(view)
{
	Q_ASSERT(nullptr != mView);
}

ScriptManager *ProjectTreeScriptMenuBuilder::scriptManager() const
{
	auto projectDir = getProjectDirectory();
	if (nullptr == projectDir)
		return nullptr;

	auto projectFile = projectDir->getProjectFile();
	Q_ASSERT(nullptr != projectFile);

	return projectFile->getScriptManager();
}

void ProjectTreeScriptMenuBuilder::fetchScriptTargets(
	QObjectList &targets, QObject *owner) const
{
	auto projectDir = getProjectDirectory();
	if (nullptr == projectDir)
		return;

	auto selectedFiles = mView->getSelectedFilesList(false);

	auto projectFile = projectDir->getProjectFile();
	Q_ASSERT(nullptr != projectFile);

	if (selectedFiles.isEmpty())
	{
		targets.append(projectDir);
	} else
	{
		for (const auto &filePath : selectedFiles)
		{
			auto file = projectDir->addExistingFile(filePath, false);
			if (file)
			{
				if (not file->isOpen())
				{
					Q_ASSERT(file != projectFile);
					Q_ASSERT(nullptr != owner);

					QObject::connect(owner, &QObject::destroyed, file,
						&AbstractFile::tryCloseAndDelete);
				}
				targets.append(file);

			} else
			{
				auto dir = projectDir->findDirectory(filePath);
				if (nullptr == dir)
					dir = projectDir->addDirectory(filePath, true, false);
				if (dir)
					targets.append(dir);
			}
		}
	}
}

AbstractProjectDirectory *
ProjectTreeScriptMenuBuilder::getProjectDirectory() const
{
	return mView->getProjectTreeModel()->getProjectDirectory();
}
}
