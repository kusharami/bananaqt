/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "AbstractScriptRunner.h"
#include "IAbortDelegate.h"

#include <QString>
#include <QVariant>
#include <QObject>
#include <QScriptValue>

#include <functional>

class QWidget;

namespace Scripting
{
extern const char szScriptExtension[];
}

class QScriptEngine;

namespace Banana
{
class ScriptRunner;
inline void internalEvalScript(
	ScriptRunner *runner, const QString &filePath, QScriptValue &out);
class ProjectGroup;

class ScriptRunner
	: public QObject
	, public AbstractScriptRunner
	, public IAbortDelegate
{
	Q_OBJECT

public:
	ScriptRunner(Banana::ProjectGroup *projectGroup, QObject *parent = nullptr);

	bool execute(
		const QString &filePath, const QString &prepareScript = QString());
	bool executeForTargets(const QString &filePath, const QObjectList &targets);

	virtual bool abort() override;

	bool isRunning() const;
	const QString &getErrorMessage() const;

	inline int processEventsInterval() const;
	void setProcessEventsInterval(int value);

	inline QScriptEngine *getActiveEngine() const;

protected:
	virtual void initializeEngine(QScriptEngine *engine) override;

private:
	bool loadScriptFile(const QString &filePath, QString &dest);

	bool executeScript(const QString &script, const QString &filePath);

	using Evaluate = std::function<QScriptValue()>;

	bool executeCustom(const Evaluate &evaluate);

	Banana::ProjectGroup *projectGroup;
	QScriptEngine *activeEngine;
	int mProcessEventsInterval;
	QString savedErrorMessage;

	friend void internalEvalScript(
		ScriptRunner *runner, const QString &filePath, QScriptValue &out);
};

int ScriptRunner::processEventsInterval() const
{
	return mProcessEventsInterval;
}

QScriptEngine *ScriptRunner::getActiveEngine() const
{
	return activeEngine;
}
}
