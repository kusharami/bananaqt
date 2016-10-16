#pragma once

class QKeySequence;
class QAction;
class QWidget;

namespace Banana
{
	namespace Utils
	{
		void addShortcutForAction(QWidget *parent,
								  const QKeySequence &sequence,
								  QAction *action);
	}
}
