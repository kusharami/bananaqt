#include "ObjectPropertyWidget.h"

#include "BananaCore/Object.h"
#include "BananaCore/Utils.h"
#include "BananaCore/ChangeValueCommand.h"

#include "QtnProperty/PropertyConnector.h"
#include "QtnProperty/MultiProperty.h"

namespace Banana
{

ObjectPropertyWidget::ObjectPropertyWidget(QWidget *parent)
	: QObjectPropertyWidget(parent)
{
	QObject::connect(propertyView(), &QtnPropertyView::beforePropertyEdited,
					 this, &ObjectPropertyWidget::onBeforePropertyEdited);
	QObject::connect(propertyView(), &QtnPropertyView::propertyEdited,
					 this, &ObjectPropertyWidget::onPropertyEdited);
}

void ObjectPropertyWidget::onBeforePropertyEdited(QtnProperty *property, QtnPropertyValuePtr newValue)
{
	auto rootProperty = property->getRootProperty();
	Q_ASSERT(nullptr != rootProperty);

	auto object = getObjectForProperty(rootProperty);
	if (nullptr != object)
	{
		auto connector = getConnectorForProperty(rootProperty);
		Q_ASSERT(nullptr != connector);

		if (nullptr != newValue)
			object->beginMacro(ChangeValueCommand::getCommandTextFor(object, connector->getMetaProperty()));
		else
			object->beginMacro(ChangeValueCommand::getResetCommandTextFor(object, connector->getMetaProperty()));
	} else
	{
		auto multiProperty = dynamic_cast<QtnMultiProperty *>(rootProperty);
		Q_ASSERT(nullptr != multiProperty);

		auto metaProperty = multiProperty->getMetaProperty();
		auto metaObject = Utils::GetMetaObjectForProperty(metaProperty);

		QString commandText;

		if (nullptr != newValue)
		{
			commandText = ChangeValueCommand::getMultipleCommandTextFor(metaObject, metaProperty);
		} else
		{
			commandText = ChangeValueCommand::getMultipleResetCommandTextFor(metaObject, metaProperty);
		}

		for (auto property : multiProperty->getProperties())
		{
			auto object = getObjectForProperty(property);
			Q_ASSERT(nullptr != object);

			object->beginMacro(commandText);
		}
	}
}

void ObjectPropertyWidget::onPropertyEdited(QtnProperty *property)
{
	auto rootProperty = property->getRootProperty();
	Q_ASSERT(nullptr != rootProperty);

	auto object = getObjectForProperty(rootProperty);
	if (nullptr != object)
	{
		object->endMacro();
	} else
	{
		auto multiProperty = dynamic_cast<QtnMultiProperty *>(rootProperty);
		Q_ASSERT(nullptr != multiProperty);

		for (auto property : multiProperty->getProperties())
		{
			auto object = getObjectForProperty(property);
			Q_ASSERT(nullptr != object);

			object->endMacro();
		}
	}
}

Object *ObjectPropertyWidget::getObjectForProperty(QtnPropertyBase *property)
{
	auto connector = getConnectorForProperty(property);
	if (nullptr != connector)
		return dynamic_cast<Object *>(connector->getObject());

	return nullptr;
}

QtnPropertyConnector *ObjectPropertyWidget::getConnectorForProperty(QtnPropertyBase *property)
{
	auto rootProperty = property->getRootProperty();
	Q_ASSERT(nullptr != rootProperty);

	return rootProperty->getConnector();
}

}
