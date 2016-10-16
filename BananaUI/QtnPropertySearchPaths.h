#pragma once

#include "QtnProperty/Property.h"
#include "QtnProperty/Delegates/PropertyDelegate.h"

#include <QMetaProperty>

namespace Banana
{
	class SearchPaths;

	class QtnPropertySearchPaths : public QtnProperty
	{
		Q_OBJECT

	public:
		explicit QtnPropertySearchPaths(QObject *object,
										const QMetaProperty &metaProperty);

		SearchPaths *getSearchPaths() const;

		static QString getPlaceholderStr();
		static void Register();

	private:
		QObject *object;
		QMetaProperty metaProperty;
	};

	class QtnPropertyDelegateSearchPaths
		: public QtnPropertyDelegateTyped<QtnPropertySearchPaths>
	{
		Q_DISABLE_COPY(QtnPropertyDelegateSearchPaths)

	public:
		QtnPropertyDelegateSearchPaths(QtnPropertySearchPaths &owner);

	protected:
		virtual void drawValueImpl(QStylePainter &painter,
								   const QRect &rect,
								   const QStyle::State &state,
								   bool *needTooltip = nullptr) const override;

		virtual QWidget* createValueEditorImpl(QWidget *parent,
											   const QRect &rect,
											   QtnInplaceInfo *inplaceInfo = nullptr) override;

		virtual bool propertyValueToStr(QString &strValue) const override;
	};
}
