/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

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

#include "UndoStack.h"

#include <QApplication>
#include <QEvent>

namespace Banana
{
UndoStack::UndoStack(QObject *parent)
	: QUndoStack(parent)
	, blockCounter(0)
	, macroCounter(0)
	, updateCounter(0)
	, firstClean(true)
{
	QObject::connect(
		this, &QUndoStack::cleanChanged, this, &UndoStack::onCleanChanged);
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
	{
		QUndoStack::beginMacro(text);
		emit macroStarted();
	}
}

void UndoStack::endMacro()
{
	Q_ASSERT(macroCounter > 0);

	if (0 == --macroCounter)
	{
		emit macroFinished();
		QUndoStack::endMacro();
	}
}

void UndoStack::blockMacro()
{
	blockCounter++;
}

void UndoStack::unblockMacro()
{
	Q_ASSERT(blockCounter > 0);
	blockCounter--;
}

void UndoStack::clear(bool force)
{
	if (force || (updateCounter == 0 && macroCounter == 0))
	{
		updateCounter = 0;
		macroCounter = 0;
		firstClean = isClean();
		QUndoStack::clear();
	}
}

void UndoStack::setClean()
{
	if (!isClean())
	{
		firstClean = (index() == 0);
		QUndoStack::setClean();
	}
}

bool UndoStack::canPushForMacro() const
{
	return macroCounter > 0 && blockCounter == 0;
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
			break;
	}

	qFatal("Unsupported drop action");
	return QString();
}

void UndoStack::onCleanChanged(bool clean)
{
	if (clean && !firstClean && cleanIndex() == 0 && index() == 0)
	{
		emit cleanChanged(false);
	}
}
}
