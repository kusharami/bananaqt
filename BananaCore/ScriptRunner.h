/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <QString>
#include <QVariant>
#include <QObject>

class QWidget;
class QScriptEngine;

namespace Scripting
{
	extern const char szScriptExtension[];
}

namespace Banana
{
	class ProjectGroup;

	class ScriptRunner : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunner(Banana::ProjectGroup *projectGroup);

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

		Banana::ProjectGroup *projectGroup;
		QString savedErrorMessage;
		QWidget *parentWidget;
	};

}
