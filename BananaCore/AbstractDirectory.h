#pragma once

#include "AbstractFileSystemObject.h"

#include <vector>

namespace Core
{
	enum class DescendantState
	{
		Added,
		Removed,
		Renamed,
		SearchPathsChanged
	};

	class AbstractDirectory : public AbstractFileSystemObject
	{
	public:
		AbstractDirectory(QObject *thiz);

		QString getAbsoluteFilePathFor(const QString &path) const;
		QString getRelativeFilePathFor(const QString &path) const;

		virtual const QString &getFileExtension() const override;
		virtual QString getFileName() const override;

		AbstractFileSystemObject *findFileSystemObject(const QString &path, bool linked);
		template <typename T>
		T *findFileT(const QString &path, bool linked = false)
		{
			return dynamic_cast<T *>(findFileSystemObject(path, linked));
		}

	protected:
		friend class AbstractFileSystemObject;

		virtual void descendantChanged(QObject *descendant, DescendantState state);

	private:
		AbstractFileSystemObject *internalFind(const QString &path, bool canonical);
	};
}
