#pragma once

#include <QObject>
#include <set>

namespace Core
{

	class AbstractObjectSelector
	{
	public:
		virtual ~AbstractObjectSelector();

		bool isObjectSelected(QObject *object);
		void setObjectSelected(QObject *object, bool value);

		bool selectionIsEmpty() const;

		virtual void clearSelection();
		const std::set<QObject *> &getSelectedObjects() const;

	protected:
		virtual void doConnectSelected(QObject *object) = 0;
		virtual void doDisconnectSelected(QObject *object) = 0;
		virtual void doChangeObjectSelection(QObject *object, bool destroyed) = 0;
		void onSelectedObjectDestroyed(QObject *object);

		std::set<QObject *> selected;
	};

}
