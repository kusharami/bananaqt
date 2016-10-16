#pragma once

#include <QString>
#include <QVariant>
#include <QObject>

class QWidget;
class QScriptEngine;

namespace Scripting
{
	extern const char *pScriptExtension;
}

namespace Core
{
	class ProjectGroup;

	class ScriptRunner : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunner(Core::ProjectGroup *projectGroup);

		void setParentWidget(QWidget *parentWidget);
		bool execute(const QString &filePath, const QString &prepareScript);
		const QString &getErrorMessage() const;

	protected:
		virtual void initializeEngine(QScriptEngine *engine);

	signals:
		void logPrint(const QString &text);

	public slots:
		QString requestExistingFilePath(const QString &title, const QString &dir = QString(),
													const QString &filters = QString()) const;
		QString requestNewFilePath(const QString &title, const QString &dir = QString(),
											   const QString &filters = QString()) const;
		QString requestDirectoryPath(const QString &title, const QString &dir = QString()) const;

		void log(const QString &text);

	private:
		bool executeScript(QScriptEngine *engine, const QString &script, const QString &filePath);

		Core::ProjectGroup *projectGroup;
		QString savedErrorMessage;
		QWidget *parentWidget;
	};

}
