#include "ProjectGroup.h"

#include "OpenedFiles.h"

#include <QDir>
#include <QUndoGroup>

namespace Banana
{
	ProjectGroup::ProjectGroup(CocosGLWidget *cocos, const QMetaObject *projectDirType)
		: cocos(cocos)
		, openedFiles(new OpenedFiles(this))
		, activeProjectDir(nullptr)
		, delegate(nullptr)
		, undoGroup(nullptr)
		, silent(false)
	{
		registerChildType(projectDirType);
	}

	ProjectGroup::~ProjectGroup()
	{
		delete openedFiles;
	}

	void ProjectGroup::setUndoGroup(QUndoGroup *undoGroup)
	{
		if (undoGroup != this->undoGroup)
		{
			disconnectUndoGroup();

			this->undoGroup = undoGroup;

			connectUndoGroup();
		}
	}

	AbstractProjectDirectory *ProjectGroup::getActiveProjectDirectory() const
	{
		return activeProjectDir;
	}

	void ProjectGroup::setActiveProjectDirectory(AbstractProjectDirectory *value)
	{
		if (activeProjectDir != value)
		{
			disconnectActiveProjectDirectory();

			setActiveProjectDirectoryInternal(value);
		}
	}

	AbstractProjectDirectory *ProjectGroup::findProject(const QString &path) const
	{
		QDir dir(path);
		auto find_path = QDir::cleanPath(path);
		QString canonical_path;

		if (dir.exists())
			canonical_path = dir.canonicalPath();

		for (auto child : children())
		{
			auto project_dir = dynamic_cast<AbstractProjectDirectory *>(child);

			if (nullptr != project_dir)
			{
				QDir dir(project_dir->getFilePath());
				if (0 == find_path.compare(dir.path(), Qt::CaseInsensitive))
				{
					return project_dir;
				}

				if (!canonical_path.isEmpty())
				{
					if (0 == canonical_path.compare(dir.path(), Qt::CaseInsensitive))
					{
						return project_dir;
					}

					if (0 == canonical_path.compare(dir.canonicalPath(), Qt::CaseInsensitive))
					{
						return project_dir;
					}
				}
			}
		}

		return nullptr;
	}

	IProjectGroupDelegate *ProjectGroup::getDelegate() const
	{
		return delegate;
	}

	void ProjectGroup::setDelegate(IProjectGroupDelegate *delegate)
	{
		this->delegate = delegate;
	}

	OpenedFiles *ProjectGroup::getOpenedFiles() const
	{
		return openedFiles;
	}

	CocosGLWidget *ProjectGroup::getCocosWidget() const
	{
		return cocos;
	}

	bool ProjectGroup::isSilent() const
	{
		return silent;
	}

	void ProjectGroup::getSilent(bool value)
	{
		silent = value;
	}

	void ProjectGroup::saveAllFiles()
	{
		for (auto child : getChildren())
		{
			auto project = dynamic_cast<AbstractProjectDirectory *>(child);
			if (nullptr != project)
				project->saveAllFiles();
		}
	}

	void ProjectGroup::onActiveProjectDirectoryDestroyed()
	{
		setActiveProjectDirectoryInternal(findChild<AbstractProjectDirectory *>(QString(), Qt::FindDirectChildrenOnly));
	}

	void ProjectGroup::onUndoGroupDestroyed()
	{
		undoGroup = nullptr;
	}

	void ProjectGroup::sortChildren(QObjectList &)
	{
		// do nothing
	}

	void ProjectGroup::deleteChild(QObject *child)
	{
		closeUnboundFiles(dynamic_cast<AbstractProjectDirectory *>(child));
		delete child;
	}

	void ProjectGroup::setActiveProjectDirectoryInternal(AbstractProjectDirectory *value)
	{
		activeProjectDir = value;

		connectActiveProjectDirectory();

		emit activeProjectDirectoryChanged();
	}

	void ProjectGroup::connectActiveProjectDirectory()
	{
		if (nullptr != activeProjectDir)
		{

		}
	}

	void ProjectGroup::disconnectActiveProjectDirectory()
	{
		if (nullptr != activeProjectDir)
		{
			QObject::disconnect(activeProjectDir, &QObject::destroyed,
							 this, &ProjectGroup::onActiveProjectDirectoryDestroyed);

			activeProjectDir = nullptr;
		}
	}

	void ProjectGroup::connectUndoGroup()
	{
		if (nullptr != undoGroup)
		{
			QObject::connect(undoGroup, &QObject::destroyed,
							 this, &ProjectGroup::onUndoGroupDestroyed);
		}
	}

	void ProjectGroup::disconnectUndoGroup()
	{
		if (nullptr != undoGroup)
		{
			QObject::disconnect(undoGroup, &QObject::destroyed,
								this, &ProjectGroup::onUndoGroupDestroyed);
			undoGroup = nullptr;
		}
	}

	void ProjectGroup::closeUnboundFiles(Directory *dir)
	{
		if (nullptr != dir)
		{
			for (auto obj : dir->children())
			{
				auto file = dynamic_cast<AbstractFile *>(obj);

				if (nullptr != file)
				{
					if (!file->isBound())
					{
						file->close();
					}
					continue;
				}

				closeUnboundFiles(dynamic_cast<Directory *>(obj));
			}
		}
	}
}
