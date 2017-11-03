#pragma once

#include <QObject>

#include <vector>

namespace Banana
{
class ScriptManager : public QObject
{
	Q_OBJECT

public:
	struct Entry
	{
		const QMetaObject *metaObject;
		QString filePath;
		QString caption;
	};

	using Entries = std::vector<Entry>;

	explicit ScriptManager(QObject *parent = nullptr);

	Q_INVOKABLE void registerScriptFor(const QMetaObject *metaObject,
		const QString &filePath, const QString &caption);
	Q_INVOKABLE void clear();

	inline const Entries &scriptEntries() const;

private:
	Entries mRegisteredScripts;
};

const ScriptManager::Entries &ScriptManager::scriptEntries() const
{
	return mRegisteredScripts;
}
}

Q_DECLARE_METATYPE(Banana::ScriptManager *)
