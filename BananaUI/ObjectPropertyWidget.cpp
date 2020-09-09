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

#include "ObjectPropertyWidget.h"

#include "ChangeValueCommand.h"

#include "BananaCore/Object.h"
#include "BananaCore/Utils.h"

#include "QtnProperty/PropertyConnector.h"
#include "QtnProperty/MultiProperty.h"
#include "QtnProperty/PropertyView.h"

namespace Banana
{
ObjectPropertyWidget::ObjectPropertyWidget(QWidget *parent)
	: QObjectPropertyWidget(parent)
{
	QObject::connect(propertyView(), &QtnPropertyView::beforePropertyEdited,
		this, &ObjectPropertyWidget::onBeforePropertyEdited);
	QObject::connect(propertyView(), &QtnPropertyView::propertyEdited, this,
		&ObjectPropertyWidget::onPropertyEdited);
}

void ObjectPropertyWidget::onBeforePropertyEdited(
	QtnPropertyBase *property, QtnPropertyValuePtr newValue)
{
	auto rootProperty = property->getRootProperty();
	Q_ASSERT(nullptr != rootProperty);

	auto object = getObjectForProperty(rootProperty);
	if (nullptr != object)
	{
		auto connector = getConnectorForProperty(rootProperty);
		Q_ASSERT(nullptr != connector);

		if (nullptr != newValue)
			object->beginMacro(ChangeValueCommand::getCommandTextFor(
				object, connector->getMetaProperty()));
		else
			object->beginMacro(ChangeValueCommand::getResetCommandTextFor(
				object, connector->getMetaProperty()));
	} else
	{
		auto multiProperty = dynamic_cast<QtnMultiProperty *>(rootProperty);
		Q_ASSERT(nullptr != multiProperty);

		auto metaProperty = multiProperty->getMetaProperty();
		auto metaObject = Utils::GetMetaObjectForProperty(metaProperty);

		QString commandText;

		if (nullptr != newValue)
		{
			commandText = ChangeValueCommand::getMultipleCommandTextFor(
				metaObject, metaProperty);
		} else
		{
			commandText = ChangeValueCommand::getMultipleResetCommandTextFor(
				metaObject, metaProperty);
		}

		for (auto property : multiProperty->getProperties())
		{
			auto object = getObjectForProperty(property);
			Q_ASSERT(nullptr != object);

			object->beginMacro(commandText);
		}
	}
}

void ObjectPropertyWidget::onPropertyEdited(QtnPropertyBase *property)
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

QtnPropertyConnector *ObjectPropertyWidget::getConnectorForProperty(
	QtnPropertyBase *property)
{
	auto rootProperty = property->getRootProperty();
	Q_ASSERT(nullptr != rootProperty);

	return rootProperty->getConnector();
}
}
