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

#include <QObject>

#include "ISelectionDelegate.h"

class QAction;
class QMenu;

namespace Banana
{
class ScriptManager;
struct ScriptCommand;
class ScriptedActionManager : public QObject
{
	Q_OBJECT

	QList<QAction *> mActions;

	ScriptManager *mScriptManager;
	QObjectList mTargets;

public:
	explicit ScriptedActionManager(
		ScriptManager *scriptManager, QObject *parent = nullptr);
	virtual ~ScriptedActionManager() override;

	inline ScriptManager *scriptManager() const;

	inline const QList<QAction *> &actions() const;
	QList<QAction *> actionsForTargets(
		ISelectionDelegate *selection, bool all = false) const;
	QList<QAction *> actionsForTargets(
		const QObjectList &targets, bool all = false) const;

	inline const QObjectList &targets() const;

	bool buildSubMenu(ISelectionDelegate *selection, QMenu *menu,
		int subMenuLimit = 0, bool all = false) const;
	bool buildSubMenu(const QObjectList &targets, QMenu *menu,
		int subMenuLimit = 0, bool all = false) const;

	void enableActionsForTargets(ISelectionDelegate *selection);
	void enableActionsForTargets(const QObjectList &targets);

signals:
	void actionsClean();
	void actionsUpdated();
	void actionTriggered(const ScriptCommand &cmd);

private:
	void onActionTriggered();
	void updateActions();
	void clearActions();
	QList<QAction *> separatedActions(const QList<QAction *> &actions) const;

	static int entryIndexForAction(QAction *action);
};

ScriptManager *ScriptedActionManager::scriptManager() const
{
	return mScriptManager;
}

const QList<QAction *> &ScriptedActionManager::actions() const
{
	return mActions;
}

const QObjectList &ScriptedActionManager::targets() const
{
	return mTargets;
}
}
