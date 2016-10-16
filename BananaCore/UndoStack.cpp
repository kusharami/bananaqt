#include "UndoStack.h"

namespace Core
{

	UndoStack::UndoStack(QObject *parent)
		: QUndoStack(parent)
		, macroCounter(0)
		, updateCounter(0)
	{

	}

	void UndoStack::beginUpdate()
	{
		updateCounter++;
	}

	void UndoStack::endUpdate()
	{
		Q_ASSERT(updateCounter > 0);
		updateCounter--;
	}

	void UndoStack::beginMacro(const QString &text)
	{
		if (0 == macroCounter++)
			QUndoStack::beginMacro(text);
	}

	void UndoStack::endMacro()
	{
		Q_ASSERT(macroCounter > 0);

		if (0 == --macroCounter)
			QUndoStack::endMacro();
	}

	void UndoStack::clear(bool force)
	{
		if (force || (updateCounter == 0 && macroCounter == 0))
		{
			updateCounter = 0;
			macroCounter = 0;
			QUndoStack::clear();
		}
	}

	QString UndoStack::getDragAndDropCommandText(Qt::DropAction action)
	{
		switch (action)
		{
			case Qt::CopyAction:
				return tr("Drag && Drop: Copy");

			case Qt::MoveAction:
				return tr("Drag && Drop: Move");

			case Qt::LinkAction:
				return tr("Drag && Drop: Link");

			default:
				qFatal("Unsupported drop action");
				break;
		}

		return QString();
	}

}
