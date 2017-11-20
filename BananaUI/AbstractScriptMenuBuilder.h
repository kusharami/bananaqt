/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#include <QObjectList>

class QMenu;
class QWidget;
class QAction;
namespace Banana
{
struct IScriptRunner;
struct IScriptRunnerDialogInitializer;
class ScriptRunner;
class ScriptRunnerDialog;
class ScriptManager;

class AbstractScriptMenuBuilder
{
	IScriptRunnerDialogInitializer *mDelegate;

public:
	AbstractScriptMenuBuilder();
	virtual ~AbstractScriptMenuBuilder();

	inline IScriptRunnerDialogInitializer *delegate() const;
	inline void setDelegate(IScriptRunnerDialogInitializer *delegate);

	bool buildMenu(
		ScriptRunner *runner, QMenu *parentMenu, int subMenuLimit = 0) const;
	static QList<QAction *> createActionsFor(ScriptManager *mgr,
		const QObjectList &targets, ScriptRunner *scriptRunner,
		QObject *parent = nullptr);

protected:
	virtual ScriptManager *scriptManager() const = 0;
	virtual void fetchScriptTargets(
		QObjectList &targets, QObject *owner) const = 0;

private:
	void initRunnerDialog(ScriptRunnerDialog *dlg) const;
};

IScriptRunnerDialogInitializer *AbstractScriptMenuBuilder::delegate() const
{
	return mDelegate;
}

void AbstractScriptMenuBuilder::setDelegate(
	IScriptRunnerDialogInitializer *delegate)
{
	mDelegate = delegate;
}
}
