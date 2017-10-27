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

#pragma once

#include "BananaCore/ContainerTypes.h"

#include <QUndoCommand>
#include <QStringList>
#include <vector>

namespace Banana
{
class BaseTreeView;

class SelectTreeItemsCommand : public QUndoCommand
{
public:
	SelectTreeItemsCommand(BaseTreeView *tree);
	SelectTreeItemsCommand(BaseTreeView *tree, const QObjectSet &oldSelected,
		const QObjectSet &newSelected);

	void setOldSelected(const QObjectSet &set);
	void setNewSelected(const QObjectSet &set);

	virtual void undo() override;
	virtual void redo() override;

	virtual int id() const override;
	virtual bool mergeWith(const QUndoCommand *other) override;

private:
	struct Path
	{
		QObject *topAncestor;
		QStringList path;
	};

	typedef std::vector<Path> Paths;

	static void toPaths(const QObjectSet &source, Paths &output);

	void select(const Paths &toSelect);

	BaseTreeView *tree;

	Paths newSelected;
	Paths oldSelected;

	bool skipRedoOnPush;
};
}
