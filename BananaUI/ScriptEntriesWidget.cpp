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
#include "QtnProperty/Core/PropertyEnumFlags.h"
#include "QtnProperty/PropertyDelegateAttrs.h"
#include "QtnProperty/PropertyQKeySequence.h"

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
	, mEnumItems(objectTypeEnumItems())
{
	QVector<QtnEnumValueInfo> enumItems;
	for (const auto &item : mEnumItems)
	{
		enumItems.append(item.valueInfo);
	}
	mObjectTypeEnumInfo = QtnEnumInfo(QStringLiteral("ObjectType"), enumItems);
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

ScriptEntriesWidget::Entry ScriptEntriesWidget::entryFrom(
	QtnPropertySet *set) const
{
	Entry entry;
	for (auto p : set->childProperties())
	{
		switch (p->id())
		{
			case TYPES_PROPERTY_ID:
			{
				auto typesProp = dynamic_cast<QtnPropertyEnumFlags *>(p);
				Q_ASSERT(nullptr != typesProp);

				entry.metaObjects = metaObjectsFromFlags(typesProp->value());
				break;
			}

			case FILE_PATH_PROPERTY_ID:
			{
				auto filePathProp = dynamic_cast<QtnPropertyQString *>(p);
				Q_ASSERT(nullptr != filePathProp);

				entry.filePath = filePathProp->value();
				break;
			}

			case CAPTION_PROPERTY_ID:
			{
				auto captionProp = dynamic_cast<QtnPropertyQString *>(p);
				Q_ASSERT(nullptr != captionProp);

				entry.caption = captionProp->value();
				break;
			}

			case KEYSEQ_PROPERTY_ID:
			{
				auto keySeqProp = dynamic_cast<QtnPropertyQKeySequence *>(p);
				Q_ASSERT(nullptr != keySeqProp);

				entry.keySeq = keySeqProp->value();
				break;
			}

			default:
				break;
		}
	}

	return entry;
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

void ScriptEntriesWidget::addEntry(const Entry &entry)
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
	if (nullptr == result)
	{
		result = dynamic_cast<QtnPropertySet *>(
			active->getMasterProperty()->parent());
	}
	Q_ASSERT(nullptr != result);
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

				auto entry = entryFrom(set);

				QByteArray bytes;
				{
					QDataStream stream(&bytes, QIODevice::WriteOnly);
					stream.setByteOrder(QDataStream::LittleEndian);

					stream << metaObjectsToFlags(entry.metaObjects);
					stream << entry.caption;
					stream << entry.filePath;
					stream << entry.keySeq;
					stream << property->isExpanded();

					auto types = set->findChildProperty(TYPES_PROPERTY_ID);
					Q_ASSERT(nullptr != types);
					stream << types->isExpanded();
				}

				mime->setText(entry.caption);
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
		bool expanded;
		bool typesExpanded;
		Entry entry;
		{
			QDataStream stream(data->data(kScriptEntryMimeType));
			stream.setByteOrder(QDataStream::LittleEndian);

			qint32 flags;
			stream >> flags;
			entry.metaObjects = metaObjectsFromFlags(flags);
			stream >> entry.caption;
			stream >> entry.filePath;
			stream >> entry.keySeq;
			stream >> expanded;
			stream >> typesExpanded;

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
				case QtnApplyPosition::None:
				case QtnApplyPosition::Before:
				case QtnApplyPosition::Over:
					break;

				case QtnApplyPosition::After:
					index++;
					break;
			}
		}

		auto newSet = newPropertySetForEntry(entry);
		newSet->setExpanded(expanded);

		auto types = newSet->findChildProperty(TYPES_PROPERTY_ID);
		Q_ASSERT(nullptr != types);
		types->setExpanded(typesExpanded);
		return set->addChildProperty(newSet, true, index);
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

		items.push_back({ QtnEnumValueInfo(0, displayTypeName), metaObject });
	}

	std::sort(
		items.begin(), items.end(), &ScriptEntriesWidget::enumItemLessThan);

	for (size_t i = 0, count = items.size(); i < count; i++)
	{
		Q_ASSERT(i <= 31);
		items.at(i).valueInfo.setValue(qint32(1 << i));
	}

	return items;
}

bool ScriptEntriesWidget::enumItemLessThan(const EnumItem &a, const EnumItem &b)
{
	return (QString::localeAwareCompare(
				a.valueInfo.name(), b.valueInfo.name()) < 0);
}

qint32 ScriptEntriesWidget::metaObjectsToFlags(
	const ScriptCommand::MetaObjects &metaObjects) const
{
	qint32 result = 0;
	for (auto &item : mEnumItems)
	{
		if (metaObjects.count(item.metaObject))
			result |= item.valueInfo.value();
	}

	return result;
}

ScriptCommand::MetaObjects ScriptEntriesWidget::metaObjectsFromFlags(
	qint32 flags) const
{
	ScriptManager::MetaObjects result;

	for (auto &item : mEnumItems)
	{
		auto value = item.valueInfo.value();
		if ((flags & value) == value)
			result.insert(item.metaObject);
	}

	return result;
}

QtnPropertySet *ScriptEntriesWidget::newPropertySetForEntry(
	const Entry &entry) const
{
	auto newSet = new QtnPropertySet(nullptr);
	newSet->setId(ENTRY_PROPERTY_ID);
	newSet->setName(entryDisplayCaption(entry.caption));

	auto types = new QtnPropertyEnumFlags;
	types->collapse();
	types->setId(TYPES_PROPERTY_ID);
	types->setName(tr("Object Types"));
	types->setEnumInfo(&mObjectTypeEnumInfo);
	types->setValue(metaObjectsToFlags(entry.metaObjects));

	newSet->addChildProperty(types);

	auto filePath = new QtnPropertyQString;
	filePath->setId(FILE_PATH_PROPERTY_ID);
	filePath->setName(tr("Script File Path"));
	filePath->setValue(entry.filePath);
	filePath->setDelegate(mFileDelegateInfo);

	newSet->addChildProperty(filePath);

	auto caption = new QtnPropertyQString;
	caption->setId(CAPTION_PROPERTY_ID);
	caption->setName(tr("Caption"));
	caption->setValue(entry.caption);
	caption->setDelegate(mCaptionDelegateInfo);

	newSet->addChildProperty(caption);

	auto keySeq = new QtnPropertyQKeySequence;
	keySeq->setId(KEYSEQ_PROPERTY_ID);
	keySeq->setName(tr("Shortcut"));
	keySeq->setValue(entry.keySeq);

	newSet->addChildProperty(keySeq);

	QObject::connect(caption, &QtnProperty::propertyDidChange,
		[newSet, caption](QtnPropertyChangeReason reason) //
		{
			if (0 != (reason & QtnPropertyChangeReasonValue))
			{
				newSet->setName(entryDisplayCaption(caption->value()));
			}
		});

	return newSet;
}
