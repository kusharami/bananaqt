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

#include "AbstractFileSystemObject.h"
#include "NamingPolicy.h"
#include "Object.h"

#include <QMetaMethod>

class QIODevice;

namespace Banana
{
class Directory;
class AbstractDirectory;
class AbstractFileRegistrator;
class OpenedFiles;
class AbstractFile
	: public Object
	, public AbstractFileSystemObject
	, public FileNamingPolicy
{
	Q_OBJECT

	Q_PROPERTY(QString fileName READ getFileName WRITE setFileName
			SCRIPTABLE true STORED false DESIGNABLE false)

	Q_PROPERTY(QString filePath READ getFilePath
			SCRIPTABLE true STORED false DESIGNABLE false)

	Q_PROPERTY(QString shortFilePath READ getFilePathShort
			SCRIPTABLE true STORED false DESIGNABLE false)

	Q_PROPERTY(QString canonicalFilePath READ getCanonicalFilePath
			SCRIPTABLE true STORED false DESIGNABLE false)

public:
	explicit AbstractFile(const QString &extension);
	virtual ~AbstractFile();

	virtual const QString &getFileExtension() const override;

	Directory *getTopDirectory() const;
	Directory *getParentDirectory() const;

	Q_INVOKABLE QObject *getData(bool open = true);

	template <typename CLASS>
	inline CLASS *getDataAs(bool open = true);

	Q_INVOKABLE bool isOpen() const;
	Q_INVOKABLE bool canClose();
	Q_INVOKABLE void bind();
	Q_INVOKABLE void unbind(bool stayOpen);
	Q_INVOKABLE bool isBound() const;
	Q_INVOKABLE bool save();
	Q_INVOKABLE bool create(bool open = false);
	Q_INVOKABLE bool open();
	Q_INVOKABLE bool reload();
	Q_INVOKABLE void close();
	Q_INVOKABLE virtual bool rename(const QString &newName) override;

	Q_INVOKABLE bool isWritable() const;

	bool saveTo(QIODevice *device);
	bool loadFrom(QIODevice *device);

	virtual QString getCanonicalFilePath() const override;

	inline unsigned getBindCount() const;

	virtual bool isSymLink() const override;
	virtual QString getSymLinkTarget() const override;

	virtual bool isWatched() const = 0;

	virtual void watchFile() = 0;
	virtual void unwatchFile() = 0;

	void updateFilePath(bool checkOldPath = true);

	void setLoadError(bool value);

	QString getFilePathShort(Directory *topDirectory = nullptr) const;
	Directory *getSearchedDirectory(Directory *topDirectory = nullptr) const;

signals:
	void flagsChanged();
	void dataChanged();
	void fileOpened();
	void fileClosed();
	void fileReloaded();
	void pathChanged();
	void updateFilePathError(const QString &path, const QString &failedPath);

private slots:
	void onNameChanged();
	void onDataDestroyed();

protected:
	friend class AbstractFileRegistrator;
	friend class AbstractDirectory;
	friend class Directory;

	void connectData();
	void disconnectData();

	virtual void changeFilePath(const QString &newPath);
	virtual bool tryChangeFilePath(const QString &newPath);
	virtual void executeUpdateFilePathError(
		const QString &path, const QString &failedPath) override;
	virtual bool updateFileExtension(
		const QString &fileName, QString *outExtension) override;
	virtual void doUpdateFilePath(bool checkOldPath);
	virtual void doParentChange() override;
	virtual QObject *doGetData();
	virtual void createData(bool *reused = nullptr);
	virtual void destroyData();
	virtual bool doSave(QIODevice *device) = 0;
	virtual bool doLoad(QIODevice *device) = 0;
	virtual void doFlagsChanged() override;
	virtual void onOpen();

	QString extension;
	QString canonicalPath;
	QString symLinkTarget;
	unsigned bindCount;
	bool loadError;
	bool symLink;

private:
	void recreateSymLinkIfNeeded(bool unwatched);
	bool saveInternal();
	void disconnectData(QObject *data);
	void internalClose();
	bool opened;
	bool signalsConnected;

	QObject *oldParent;
	QString savedPath;
	QObject *connectedData;
};

template <typename CLASS>
CLASS *AbstractFile::getDataAs(bool open)
{
	auto result = getData(open);

	if (result != nullptr)
		Q_ASSERT(nullptr != dynamic_cast<CLASS *>(result));

	return static_cast<CLASS *>(result);
}

unsigned AbstractFile::getBindCount() const
{
	return bindCount;
}
}

Q_DECLARE_METATYPE(Banana::AbstractFile *)
