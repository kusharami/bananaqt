#include "ProjectPropertyWidget.h"

#include "BananaCore/AbstractProjectFile.h"

namespace Banana
{

ProjectPropertyWidget::ProjectPropertyWidget(QWidget *parent)
	: ObjectPropertyWidget(parent)
{

}

void ProjectPropertyWidget::selectProjectFile(AbstractProjectFile *file)
{
	if (getProjectFile() != file)
	{
		disconnectFile();

		if (nullptr == file)
			deselectAllObjects();
		else
			selectObject(file);

		connectFile();
	}
}

AbstractProjectFile *ProjectPropertyWidget::getProjectFile() const
{
	if (!selectedObjects.empty())
		return dynamic_cast<AbstractProjectFile *>(*selectedObjects.begin());

	return nullptr;
}

void ProjectPropertyWidget::connectFile()
{
	auto projectFile = getProjectFile();
	if (nullptr != projectFile)
	{
		QObject::connect(projectFile, &Object::modifiedFlagChanged,
						 this, &ProjectPropertyWidget::projectFileChanged);
	}
}

void ProjectPropertyWidget::disconnectFile()
{
	auto projectFile = getProjectFile();
	if (nullptr != projectFile)
	{
		QObject::disconnect(projectFile, &Object::modifiedFlagChanged,
						 this, &ProjectPropertyWidget::projectFileChanged);
	}
}
}
