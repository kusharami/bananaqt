#include "AbstractObjectSelector.h"

namespace Core
{

	AbstractObjectSelector::~AbstractObjectSelector()
	{

	}

	bool AbstractObjectSelector::isObjectSelected(QObject *object)
	{
		return selected.end() != selected.find(object);
	}

	void AbstractObjectSelector::setObjectSelected(QObject *object, bool value)
	{
		if (isObjectSelected(object) != value)
		{
			if (value)
			{
				selected.insert(object);
				doConnectSelected(object);
			}
			else
			{
				doDisconnectSelected(object);
				selected.erase(object);
			}

			doChangeObjectSelection(object, false);
		}
	}

	bool AbstractObjectSelector::selectionIsEmpty() const
	{
		return selected.empty();
	}

	void AbstractObjectSelector::clearSelection()
	{
		for (auto object : selected)
			doDisconnectSelected(object);

		selected.clear();
		doChangeObjectSelection(nullptr, false);
	}

	const std::set<QObject *> &AbstractObjectSelector::getSelectedObjects() const
	{
		return selected;
	}

	void AbstractObjectSelector::onSelectedObjectDestroyed(QObject *object)
	{
		selected.erase(object);
		doChangeObjectSelection(object, true);
	}

}
