#include "VariantMapFile.h"

#include "Utils.h"
#include "Object.h"

namespace Banana
{

	VariantMapFile::VariantMapFile(const QString &extension)
		: AbstractFile(extension)
	{

	}

	bool VariantMapFile::doSave(QIODevice *device)
	{
		QVariantMap data;
		saveData(data);

		return Utils::SaveVariantMapToIODevice(data, device);
	}

	bool VariantMapFile::doLoad(QIODevice *device)
	{
		QVariantMap data;
		if (Utils::LoadVariantMapFromIODevice(data, device))
			return loadData(data);

		return false;
	}

	void VariantMapFile::saveData(QVariantMap &output)
	{
		auto data = doGetData();
		if (nullptr != data)
		{
			auto obj = dynamic_cast<Object *>(data);
			if (nullptr != obj)
				obj->saveContents(output);
			else
				Object::saveContents(data, output);
		}
	}

	bool VariantMapFile::loadData(const QVariantMap &input)
	{
		loadCounter++;
		bool ok = false;
		auto data = doGetData();
		if (nullptr != data)
		{
			auto obj = dynamic_cast<Object *>(data);
			if (nullptr != obj)
				ok = obj->loadContents(input, true);
			else
				ok = Object::loadContents(input, data, true);
		}

		loadCounter--;
		return ok;
	}

}
