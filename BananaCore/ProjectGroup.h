#pragma once

#include "ObjectGroup.h"

#include "AbstractProjectDirectory.h"

class CocosGLWidget;
class QUndoGroup;

namespace Core
{
	class OpenedFiles;

	struct IProjectGroupDelegate;
	class ProjectGroup : public Core::ObjectGroup
	{
		Q_OBJECT

		Q_PROPERTY(AbstractProjectDirectory *activeProjectDirectory
				   READ getActiveProjectDirectory
				   WRITE setActiveProjectDirectory
				   NOTIFY activeProjectDirectoryChanged
				   DESIGNABLE false
				   SCRIPTABLE true)

		Q_PROPERTY(bool silent
				   READ isSilent
				   WRITE getSilent
				   SCRIPTABLE true
				   DESIGNABLE false)

	public:
		explicit ProjectGroup(CocosGLWidget *cocos, const QMetaObject *projectDirType);
		virtual ~ProjectGroup();

		inline QUndoGroup *getUndoGroup() const;
		void setUndoGroup(QUndoGroup *undoGroup);

		AbstractProjectDirectory *getActiveProjectDirectory() const;
		void setActiveProjectDirectory(AbstractProjectDirectory *value);

		AbstractProjectDirectory *findProject(const QString &path) const;

		IProjectGroupDelegate *getDelegate() const;
		void setDelegate(IProjectGroupDelegate *delegate);

		Core::OpenedFiles *getOpenedFiles() const;
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
		Core::OpenedFiles *openedFiles;
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
