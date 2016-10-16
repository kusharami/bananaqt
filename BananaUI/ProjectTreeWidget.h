#pragma once

#include <QWidget>

namespace Ui
{
	class ProjectTreeWidget;
}

class QMenu;

namespace Banana
{

class ProjectTreeView;
class ProjectTreeWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ProjectTreeWidget(QWidget *parent = nullptr);
	virtual ~ProjectTreeWidget();

	ProjectTreeView *getTreeView() const;

	void showFilter(bool show);
	QMenu *getButtonOptionsMenu() const;

private slots:
	void onFilterTextChanged();

	void on_filterEdit_editingFinished();

	void on_actionOptionsRegExp_toggled(bool checked);

	void on_actionOptionsWildcard_toggled(bool checked);

	void on_actionOptionsContainingString_toggled(bool checked);

private:
	void updateFilter(bool force = false);
	void setPatternSyntax(QRegExp::PatternSyntax syntax);

	Ui::ProjectTreeWidget *ui;
	QString currentFilter;
	QRegExp::PatternSyntax pattern_syntax;
};
}
