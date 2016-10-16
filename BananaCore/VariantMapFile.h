#pragma once

#include "AbstractFile.h"

#include <QVariantMap>

namespace Core
{

	class VariantMapFile : public AbstractFile
	{
	public:
		VariantMapFile(const QString &extension);		

	protected:
		virtual bool doSave(QIODevice *device) override;
		virtual bool doLoad(QIODevice *device) override;
		virtual void saveData(QVariantMap &output);
		virtual bool loadData(const QVariantMap &input);
	};

}
