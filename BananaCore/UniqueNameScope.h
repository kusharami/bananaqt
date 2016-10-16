#pragma once

#include <QObject>

namespace Core
{

	class UniqueNameScope : public QObject
	{
		Q_OBJECT

	public:
		explicit UniqueNameScope(const QMetaObject *meta_object,
								 Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive,
								 QObject *parent = nullptr);

		const QMetaObject *getObjectType() const;

		Qt::CaseSensitivity getCaseSensitivity() const;
		void setCaseSensitivity(Qt::CaseSensitivity value);

		void connectObject(QObject *object);
		void disconnectObject(QObject *object);

	protected:
		virtual QObject *findSibling(QObject *object, const QString &find_name) const;
		virtual bool checkSiblingNameForObject(QObject *sibling, const QString &name, QObject *object) const;

		const QMetaObject *meta_object;
		Qt::CaseSensitivity sensitivity;

	private slots:
		void onObjectNameChanged(const QString &name);

	private:
		static QString getFixedName(const QString &name, QObject *object);
		static QString getNumberSeparator(QObject *object);
		static QString getNumberedFormat(QObject *object);

		void setObjectName(QObject *object, const QString &name);
		void internalConnectObject(QObject *object);
		void internalDisconnectObject(QObject *object);				
	};

}
