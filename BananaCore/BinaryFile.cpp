#include "BinaryFile.h"

#include "Utils.h"

#include <QIODevice>

namespace Core
{

	BinaryFile::BinaryFile()
		: Inherited("")
	{
		(void) QT_TRANSLATE_NOOP("FileTypeTitlePlural", "Core::BinaryFile");
		(void) QT_TRANSLATE_NOOP("FileTypeTitle", "Core::BinaryFile");
		(void) QT_TRANSLATE_NOOP("FileFormatNamePlural", " ");
		(void) QT_TRANSLATE_NOOP("FileFormatName", " ");
	}

	bool BinaryFile::isWritableFormat(const QString &extension) const
	{
		return (extension.isEmpty());
	}

	QObject *BinaryFile::doCreateFileData() const
	{
		return new BinaryData;
	}

	bool BinaryFile::doSave(QIODevice *device)
	{
		if (device->isOpen())
		{
			auto binary = dynamic_cast<BinaryData *>(data);
			auto size = binary->size();
			return (device->write(binary->data(), size) == size);
		}

		return false;
	}

	bool BinaryFile::doLoad(QIODevice *device)
	{
		auto binary = dynamic_cast<BinaryData *>(data);
		return Utils::LoadBinaryFromIODevice(*binary, device);
	}
}
