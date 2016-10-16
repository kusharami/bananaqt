#pragma once

#include "FileRegistrator.h"
#include "AbstractFile.h"

#include <QByteArray>

namespace Core
{
	class BinaryData : public Object, public QByteArray
	{
		Q_OBJECT
	};

	class BinaryFile : public Core::BaseFileRegistrator<Core::AbstractFile>
	{
		Q_OBJECT

	public:
		Q_INVOKABLE explicit BinaryFile();

		virtual bool isWritableFormat(const QString &extension) const override;

	protected:
		virtual QObject *doCreateFileData() const override;
		virtual bool doSave(QIODevice *device) override;
		virtual bool doLoad(QIODevice *device) override;
	};

}
