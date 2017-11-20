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

#include "AbstractScriptMenuBuilder.h"

#include "BananaCore/ScriptManager.h"
#include "BananaCore/ScriptRunner.h"

#include "IScriptRunnerDialogInitializer.h"
#include "ScriptRunnerDialog.h"

#include <QMenu>
#include <QAction>

namespace Banana
{
AbstractScriptMenuBuilder::AbstractScriptMenuBuilder()
{
	// do nothing
}

AbstractScriptMenuBuilder::~AbstractScriptMenuBuilder()
{
	// do nothing
}

bool AbstractScriptMenuBuilder::buildMenu(
	ScriptRunner *runner, QMenu *parentMenu, int subMenuLimit) const
{
	QObjectList targets;
	fetchScriptTargets(targets, parentMenu);

	if (not targets.isEmpty())
	{
		auto sm = scriptManager();
		Q_ASSERT(nullptr != sm);

		if (sm->hasActionsFor(targets))
		{
			auto dialog = new ScriptRunnerDialog(parentMenu);
			dialog->setAbortDelegate(runner);
			initRunnerDialog(dialog);
			runner->setDelegate(dialog);
			auto actions = createActionsFor(sm, targets, runner, parentMenu);
			Q_ASSERT(not actions.isEmpty());

			if (actions.count() > subMenuLimit)
			{
				auto menu = new QMenu(
					ScriptManager::scriptedActionsCaption(), parentMenu);
				menu->addActions(actions);
			} else
			{
				parentMenu->addActions(actions);
			}

			return true;
		}
	}

	return false;
}

QList<QAction *> AbstractScriptMenuBuilder::createActionsFor(ScriptManager *mgr,
	const QObjectList &targets, ScriptRunner *scriptRunner, QObject *parent)
{
	Q_ASSERT(nullptr != scriptRunner);

	QList<QAction *> result;

	for (const ScriptManager::Entry &entry : mgr->scriptEntries())
	{
		if (not entry.isValid())
			continue;

		QObjectList supportTargets;

		for (auto target : targets)
		{
			if (entry.metaObject->cast(target))
				supportTargets.append(target);
		}

		if (supportTargets.isEmpty())
			continue;

		auto action = new QAction(parent);

		action->setText(entry.caption);

		auto filePath = entry.filePath;
		QObject::connect(action, &QAction::triggered,
			[filePath, scriptRunner, supportTargets]() {
				scriptRunner->executeForTargets(filePath, supportTargets);
			});

		result.append(action);
	}

	return result;
}

void AbstractScriptMenuBuilder::initRunnerDialog(ScriptRunnerDialog *dlg) const
{
	if (mDelegate)
		mDelegate->initRunnerDialog(dlg);
}
}
