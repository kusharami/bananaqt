#include "Utils.h"

#include <QWidget>
#include <QShortcut>
#include <QAction>
#include <QKeySequence>

namespace Banana
{
namespace Utils
{
	void addShortcutForAction(QWidget *parent,
							  const QKeySequence &sequence,
							  QAction *action)
	{
		auto shortcut = new QShortcut(sequence, parent, nullptr, nullptr,
									  Qt::ApplicationShortcut);
		QObject::connect(shortcut, &QShortcut::activated,
						 action, &QAction::trigger);
		if (QKeySequence::ExactMatch != action->shortcut().matches(sequence))
		{
			shortcut = new QShortcut(action->shortcut(), parent);
			QObject::connect(shortcut, &QShortcut::activated,
							 action, &QAction::trigger);
		}
	}
}
}
