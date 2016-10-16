#pragma once

#include <QString>
#include <QStringList>

class QObject;

namespace Banana
{
	class AbstractDirectory;
	class AbstractFileSystemObject
	{
	public:
		AbstractFileSystemObject(QObject *thiz);
		virtual ~AbstractFileSystemObject();

		QObject *getObject() const { return thiz; }

		virtual bool isSymLink() const;
		virtual QString getSymLinkTarget() const;

		AbstractDirectory *getTopDirectory() const;
		AbstractDirectory *getParentDirectory() const;
		virtual QString getFilePath(const AbstractDirectory *relative_to = nullptr) const;
		virtual QString getCanonicalFilePath() const;
		virtual QString getFileName() const;
		QString getBaseName() const;

		virtual bool rename(const QString &new_name);
		virtual bool setFileName(const QString &value);

		virtual const QString &getFileExtension() const = 0;

		virtual QStringList getSupportedExtensions() const;

		virtual bool isWritableFormat(const QString &extension) const;

	protected:
		virtual bool updateFileExtension(const QString &filename, QString *out_ext);
		virtual void updateFileNameError(const QString &failed_name);
		virtual void executeUpdateFilePathError(const QString &path, const QString &failed_path) = 0;

		AbstractFileSystemObject *internalFind(const QString &path, bool canonical) const;

		QObject *thiz;
	};

}
