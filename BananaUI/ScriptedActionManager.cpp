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

#include "ScriptedActionManager.h"

#include "BananaCore/ScriptManager.h"

#include <QAction>
#include <QMenu>

namespace Banana
{
ScriptedActionManager::ScriptedActionManager(
	ScriptManager *scriptManager, QObject *parent)
	: QObject(parent)
	, mScriptManager(scriptManager)
{
	Q_ASSERT(nullptr != mScriptManager);
	updateActions();

	QObject::connect(mScriptManager, &ScriptManager::changed, this,
		&ScriptedActionManager::updateActions);
}

ScriptedActionManager::~ScriptedActionManager()
{
	clearActions();
}

QList<QAction *> ScriptedActionManager::actionsForTargets(
	ISelectionDelegate *selection, bool all) const
{
	if (selection)
	{
		return actionsForTargets(selection->getSelectedItems(), all);
	}

	return actionsForTargets(QObjectList(), all);
}

QList<QAction *> ScriptedActionManager::actionsForTargets(
	const QObjectList &targets, bool all) const
{
	QList<QAction *> result;

	auto &entries = mScriptManager->scriptEntries();
	for (auto action : mActions)
	{
		if (action == nullptr)
			continue;

		auto &entry = entries.at(entryIndexForAction(action));

		if ((all && entry.isValid() && entry.metaObjects.empty()) ||
			entry.supportsTargets(targets))
		{
			result.append(action);
		}
	}

	return separatedActions(result);
}

bool ScriptedActionManager::buildSubMenu(ISelectionDelegate *selection,
	QMenu *menu, int subMenuLimit, bool all) const
{
	return buildSubMenu(
		selection ? selection->getSelectedItems() : QObjectList(), menu,
		subMenuLimit, all);
}

bool ScriptedActionManager::buildSubMenu(
	const QObjectList &targets, QMenu *menu, int subMenuLimit, bool all) const
{
	auto actions = actionsForTargets(targets, all);
	if (actions.isEmpty())
		return false;

	if (actions.count() > subMenuLimit)
	{
		menu = new QMenu(ScriptManager::scriptedActionsCaption(), menu);
	}

	for (auto action : actions)
	{
		if (action)
			menu->addAction(action);
		else
			menu->addSeparator();
	}
	return true;
}

void ScriptedActionManager::enableActionsForTargets(
	ISelectionDelegate *selection)
{
	if (selection)
	{
		enableActionsForTargets(selection->getSelectedItems());
	} else
	{
		enableActionsForTargets(QObjectList());
	}
}

void ScriptedActionManager::enableActionsForTargets(const QObjectList &targets)
{
	mTargets = targets;
	auto &entries = mScriptManager->scriptEntries();
	for (auto action : mActions)
	{
		if (action == nullptr)
			continue;

		auto &entry = entries.at(entryIndexForAction(action));

		if (not entry.metaObjects.empty())
			action->setEnabled(entry.supportsTargets(targets));
	}
}

void ScriptedActionManager::onActionTriggered()
{
	auto action = static_cast<QAction *>(sender());

	auto &entries = mScriptManager->scriptEntries();
	auto &entry = entries.at(entryIndexForAction(action));

	emit actionTriggered(entry);
}

void ScriptedActionManager::updateActions()
{
	clearActions();

	QList<QAction *> actions;

	auto &entries = mScriptManager->scriptEntries();
	for (int i = 0, count = int(entries.size()); i < count; i++)
	{
		auto &entry = entries.at(i);
		if (not entry.isValid())
			continue;

		auto action = new QAction(entry.caption, this);
		action->setShortcut(entry.keySeq);
		action->setEnabled(entry.metaObjects.empty());
		action->setData(i);
		QObject::connect(action, &QAction::triggered, this,
			&ScriptedActionManager::onActionTriggered);
		actions.append(action);
	}

	mActions = separatedActions(actions);
	enableActionsForTargets(mTargets);

	emit actionsUpdated();
}

void ScriptedActionManager::clearActions()
{
	if (not mActions.isEmpty())
	{
		for (auto action : mActions)
			delete action;

		mActions.clear();
		emit actionsClean();
	}
}

QList<QAction *> ScriptedActionManager::separatedActions(
	const QList<QAction *> &actions) const
{
	QList<QAction *> result;

	auto &entries = mScriptManager->scriptEntries();
	const ScriptCommand *prevEntry = nullptr;
	for (auto action : actions)
	{
		auto &newEntry = entries.at(entryIndexForAction(action));

		if (prevEntry != nullptr)
		{
			if (prevEntry->metaObjects != newEntry.metaObjects)
			{
				result.append(nullptr);
			}
		}

		prevEntry = &newEntry;
		result.append(action);
	}

	return result;
}

int ScriptedActionManager::entryIndexForAction(QAction *action)
{
	Q_ASSERT(nullptr != action);
	Q_ASSERT(action->data().type() == QVariant::Int);

	return action->data().toInt();
}
}
