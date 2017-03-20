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

#include "ObjectGroup.h"

#include "AbstractProjectDirectory.h"

class CocosGLWidget;
class QUndoGroup;

namespace Banana
{
class OpenedFiles;

struct IProjectGroupDelegate;
class ProjectGroup : public Banana::ObjectGroup
{
	Q_OBJECT

	Q_PROPERTY(
		AbstractProjectDirectory * activeProjectDirectory
		READ getActiveProjectDirectory
		WRITE setActiveProjectDirectory
		NOTIFY activeProjectDirectoryChanged
		DESIGNABLE false
		SCRIPTABLE true)

	Q_PROPERTY(
		bool silent
		READ isSilent
		WRITE getSilent
		SCRIPTABLE true
		DESIGNABLE false)

public:
	explicit ProjectGroup(CocosGLWidget *cocos,
						  const QMetaObject *projectDirType);
	virtual ~ProjectGroup();

	inline QUndoGroup *getUndoGroup() const;
	void setUndoGroup(QUndoGroup *undoGroup);

	AbstractProjectDirectory *getActiveProjectDirectory() const;
	void setActiveProjectDirectory(AbstractProjectDirectory *value);

	AbstractProjectDirectory *findProject(const QString &path) const;

	IProjectGroupDelegate *getDelegate() const;
	void setDelegate(IProjectGroupDelegate *delegate);

	Banana::OpenedFiles *getOpenedFiles() const;
	CocosGLWidget *getCocosWidget() const;

	bool isSilent() const;
	void getSilent(bool value);

	void saveAllFiles();

signals:
	void activeProjectDirectoryChanged();

private slots:
	void onActiveProjectDirectoryDestroyed();
	void onUndoGroupDestroyed();

protected:
	virtual void sortChildren(QObjectList &) override;
	virtual void deleteChild(QObject *child) override;

private:
	void setActiveProjectDirectoryInternal(AbstractProjectDirectory *value);

	void connectActiveProjectDirectory();
	void disconnectActiveProjectDirectory();
	void connectUndoGroup();
	void disconnectUndoGroup();

	static void closeUnboundFiles(Directory *dir);

	CocosGLWidget *cocos;
	Banana::OpenedFiles *openedFiles;
	AbstractProjectDirectory *activeProjectDir;
	IProjectGroupDelegate *delegate;
	QUndoGroup *undoGroup;
	bool silent;
};

QUndoGroup *ProjectGroup::getUndoGroup() const
{
	return undoGroup;
}

}
