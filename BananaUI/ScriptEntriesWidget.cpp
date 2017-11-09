/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "ScriptEntriesWidget.h"

#include "QtnProperty/Core/PropertyQString.h"
#include "QtnProperty/Core/PropertyEnum.h"
#include "QtnProperty/PropertyDelegateAttrs.h"

#include "BananaCore/ScriptRunner.h"
#include "BananaCore/Directory.h"

#include <QFileDialog>
#include <QCoreApplication>
#include <QMimeData>
#include <QDataStream>

static const auto kScriptEntryMimeType = QStringLiteral("Banana::ScriptEntry");

using namespace Banana;

ScriptEntriesWidget::ScriptEntriesWidget(QWidget *parent)
	: QtnPropertyWidgetEx(parent)
	, mObjectTypeEnumInfo(QStringLiteral("ObjectType"), objectTypeEnumItems())
{
	mFileDelegateInfo.name = qtnSelectEditDelegate();
	mFileDelegateInfo.attributes[qtnShowRelativePathAttr()] = true;
	mFileDelegateInfo.attributes[qtnFileModeAttr()] = QFileDialog::ExistingFile;
	mFileDelegateInfo.attributes[qtnInvalidColorAttr()] = QColor(Qt::red);
	mFileDelegateInfo.attributes[qtnFileNameFilterAttr()] =
		Directory::getFilterForExtension(Scripting::szScriptExtension);

	mCaptionDelegateInfo.name = qtnLineEditDelegate();
	mCaptionDelegateInfo.attributes[qtnMultiLineEditAttr()] = false;
}

void ScriptEntriesWidget::setRootDirectory(const QString &rootDirectory)
{
	mFileDelegateInfo.attributes[qtnDefaultDirAttr()] = rootDirectory;
}

ScriptEntriesWidget::InternalEntry ScriptEntriesWidget::internalEntryFrom(
	QtnPropertySet *set) const
{
	InternalEntry entry;
	for (auto p : set->childProperties())
	{
		switch (p->id())
		{
			case TYPE_PROPERTY_ID:
			{
				auto aEnum = dynamic_cast<QtnPropertyEnum *>(p);
				Q_ASSERT(nullptr != aEnum);

				entry.objectType = aEnum->value();
				break;
			}

			case FILE_PATH_PROPERTY_ID:
			{
				auto str = dynamic_cast<QtnPropertyQString *>(p);
				Q_ASSERT(nullptr != str);

				entry.filePath = str->value();
				break;
			}

			case CAPTION_PROPERTY_ID:
			{
				auto str = dynamic_cast<QtnPropertyQString *>(p);
				Q_ASSERT(nullptr != str);

				entry.caption = str->value();
				break;
			}

			default:
				break;
		}
	}

	return entry;
}

ScriptEntriesWidget::Entry ScriptEntriesWidget::entryFrom(
	QtnPropertySet *set) const
{
	return internalEntryFrom(set).toEntry();
}

ScriptEntriesWidget::Entries ScriptEntriesWidget::entries() const
{
	auto set = propertySet();

	Entries entries;
	if (set)
	{
		for (auto property : set->childProperties())
		{
			auto childSet = dynamic_cast<QtnPropertySet *>(property);
			Q_ASSERT(nullptr != childSet);

			entries.push_back(entryFrom(childSet));
		}
	}
	return entries;
}

void ScriptEntriesWidget::setEntries(const Entries &entries)
{
	auto set = propertySet();

	setPropertySet(nullptr);
	delete set;

	set = new QtnPropertySet(this);

	for (auto &entry : entries)
	{
		set->addChildProperty(newPropertySetForEntry(entry));
	}

	setPropertySet(set);
}

void ScriptEntriesWidget::addEntry(const ScriptEntriesWidget::Entry &entry)
{
	auto set = propertySet();

	bool wasNull = false;
	if (nullptr == set)
	{
		set = new QtnPropertySet(this);
		wasNull = true;
	}

	auto newProperty = newPropertySetForEntry(entry);
	set->addChildProperty(newProperty);

	if (wasNull)
	{
		setPropertySet(set);
	}

	propertyView()->setActiveProperty(newProperty, true);
}

void ScriptEntriesWidget::clear()
{
	auto set = propertySet();
	setPropertySet(nullptr);
	delete set;
}

QtnPropertySet *ScriptEntriesWidget::getActiveEntryProperty() const
{
	auto active = getActiveProperty();

	if (nullptr == active)
		return nullptr;

	if (active->id() == ENTRY_PROPERTY_ID)
	{
		return dynamic_cast<QtnPropertySet *>(active);
	}

	auto result = dynamic_cast<QtnPropertySet *>(active->parent());
	Q_ASSERT(result != nullptr);
	Q_ASSERT(result->id() == ENTRY_PROPERTY_ID);

	return result;
}

bool ScriptEntriesWidget::canDeleteProperty(QtnPropertyBase *property)
{
	if (property && property->id() == ENTRY_PROPERTY_ID)
	{
		Q_ASSERT(dynamic_cast<QtnPropertySet *>(property) != nullptr);
		return true;
	}

	return false;
}

bool ScriptEntriesWidget::canCutToClipboard()
{
	return false;
}

void ScriptEntriesWidget::deleteProperty(QtnPropertyBase *property)
{
	if (canDeleteProperty(property))
	{
		auto set = propertySet();
		Q_ASSERT(nullptr != set);
		auto index = set->childProperties().indexOf(property);
		set->removeChildProperty(property);
		delete property;

		propertyView()->setActiveProperty(index, true);
	}
}

bool ScriptEntriesWidget::dataHasSupportedFormats(const QMimeData *data)
{
	if (nullptr != data)
	{
		return QtnPropertyWidgetEx::dataHasSupportedFormats(data) ||
			data->hasFormat(kScriptEntryMimeType);
	}

	return false;
}

QMimeData *ScriptEntriesWidget::getPropertyDataForAction(
	QtnPropertyBase *property, Qt::DropAction dropAction)
{
	if (property->id() == ENTRY_PROPERTY_ID)
	{
		auto set = dynamic_cast<QtnPropertySet *>(property);
		Q_ASSERT(nullptr != set);

		switch (dropAction)
		{
			case Qt::MoveAction:
			case Qt::CopyAction:
			case Qt::IgnoreAction:
			{
				auto mime = new QMimeData;

				auto internal = internalEntryFrom(set);

				QByteArray bytes;
				{
					QDataStream stream(&bytes, QIODevice::WriteOnly);
					stream.setByteOrder(QDataStream::LittleEndian);

					stream << internal.objectType;
					stream << internal.caption;
					stream << internal.filePath;
					stream << property->isExpanded();
				}

				mime->setText(internal.caption);
				mime->setData(kScriptEntryMimeType, bytes);
				return mime;
			}

			default:
				break;
		}
	}

	return QtnPropertyWidgetEx::getPropertyDataForAction(property, dropAction);
}

bool ScriptEntriesWidget::applyPropertyData(const QMimeData *data,
	QtnPropertyBase *destination, QtnApplyPosition position)
{
	if (data->hasFormat(kScriptEntryMimeType))
	{
		bool expanded = true;
		InternalEntry internal;
		{
			QDataStream stream(data->data(kScriptEntryMimeType));
			stream.setByteOrder(QDataStream::LittleEndian);

			stream >> internal.objectType;
			stream >> internal.caption;
			stream >> internal.filePath;
			stream >> expanded;

			if (stream.status() != QDataStream::Ok)
				return false;
		}

		auto entryDest = dynamic_cast<QtnPropertySet *>(destination);

		if (entryDest == nullptr)
		{
			position = QtnApplyPosition::After;
			if (destination)
			{
				entryDest =
					dynamic_cast<QtnPropertySet *>(destination->parent());
			}
		}

		auto set = propertySet();
		auto &siblings = set->childProperties();

		int index = -1;
		if (entryDest != nullptr)
		{
			Q_ASSERT(ENTRY_PROPERTY_ID == entryDest->id());
			index = siblings.indexOf(entryDest);
			switch (position)
			{
				case QtnApplyPosition::Before:
				case QtnApplyPosition::Over:
					break;

				case QtnApplyPosition::After:
					index++;
					break;
			}
		}

		auto newProperty = newPropertySetForEntry(internal.toEntry());
		newProperty->setExpanded(expanded);
		return set->addChildProperty(newProperty, true, index);
	}

	return QtnPropertyWidgetEx::applyPropertyData(data, destination, position);
}

QString ScriptEntriesWidget::entryDisplayCaption(const QString &caption)
{
	if (caption.isEmpty())
		return tr("(Untitled Action)");

	return caption;
}

ScriptEntriesWidget::EnumItems ScriptEntriesWidget::objectTypeEnumItems()
{
	EnumItems items;

	for (auto metaObject : ScriptManager::metaObjects())
	{
		auto displayTypeName =
			QCoreApplication::translate("ClassName", metaObject->className());

		items.append(QtnEnumValueInfo(
			QMetaType::type(QByteArray(metaObject->className()) + "*"),
			displayTypeName));
	}

	std::sort(
		items.begin(), items.end(), &ScriptEntriesWidget::objectTypeLessThan);

	return items;
}

bool ScriptEntriesWidget::objectTypeLessThan(
	const QtnEnumValueInfo &a, const QtnEnumValueInfo &b)
{
	return QString::localeAwareCompare(a.name(), b.name()) < 0;
}

QtnPropertySet *ScriptEntriesWidget::newPropertySetForEntry(
	const Entry &entry) const
{
	auto result = new QtnPropertySet(nullptr);
	result->setId(ENTRY_PROPERTY_ID);
	result->setName(entryDisplayCaption(entry.caption));

	auto type = new QtnPropertyEnum;
	type->setId(TYPE_PROPERTY_ID);
	type->setName(tr("Object Type"));
	type->setEnumInfo(&mObjectTypeEnumInfo);
	type->setValue(
		QMetaType::type(QByteArray(entry.metaObject->className()) + "*"));

	result->addChildProperty(type);

	auto filePath = new QtnPropertyQString;
	filePath->setId(FILE_PATH_PROPERTY_ID);
	filePath->setName(tr("Script File Path"));
	filePath->setValue(entry.filePath);
	filePath->setDelegate(mFileDelegateInfo);

	result->addChildProperty(filePath);

	auto caption = new QtnPropertyQString;
	caption->setId(CAPTION_PROPERTY_ID);
	caption->setName(tr("Caption"));
	caption->setValue(entry.caption);
	caption->setDelegate(mCaptionDelegateInfo);

	result->addChildProperty(caption);

	QObject::connect(caption, &QtnProperty::propertyDidChange,
		[result, caption](QtnPropertyChangeReason reason) //
		{
			if (0 != (reason & QtnPropertyChangeReasonValue))
			{
				result->setName(entryDisplayCaption(caption->value()));
			}
		});

	return result;
}

ScriptEntriesWidget::Entry ScriptEntriesWidget::InternalEntry::toEntry() const
{
	Entry entry;

	entry.filePath = filePath;
	entry.caption = caption;
	entry.metaObject = QMetaType::metaObjectForType(objectType);

	return entry;
}
