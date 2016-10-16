#pragma once

#include "ObjectPropertyWidget.h"

namespace Banana
{
	class AbstractProjectFile;

class ProjectPropertyWidget : public ObjectPropertyWidget
{
	Q_OBJECT

public:
	explicit ProjectPropertyWidget(QWidget *parent = nullptr);

	void selectProjectFile(Banana::AbstractProjectFile *file);
	Banana::AbstractProjectFile *getProjectFile() const;

signals:
	void projectFileChanged();

private:
	void connectFile();
	void disconnectFile();
};
}
