#pragma once

#include "QtnProperty/QObjectPropertyWidget.h"

class QtnPropertyConnector;

namespace Banana
{
	class Object;

class ObjectPropertyWidget : public QObjectPropertyWidget
{
	Q_OBJECT

public:
	explicit ObjectPropertyWidget(QWidget *parent = nullptr);

private slots:
	void onBeforePropertyEdited(QtnProperty *property, QtnPropertyValuePtr newValue);
	void onPropertyEdited(QtnProperty *property);

private:
	static Banana::Object *getObjectForProperty(QtnPropertyBase *property);
	static QtnPropertyConnector *getConnectorForProperty(QtnPropertyBase *property);
};
}
