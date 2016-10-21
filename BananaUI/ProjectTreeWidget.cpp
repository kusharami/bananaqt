/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ProjectTreeWidget.h"

#include "ProjectDirectoryFilterModel.h"

#include <QSettings>
#include <QMenu>

#include <map>

using namespace Banana;
#include "ui_ProjectTreeWidget.h"

namespace Banana
{

static const QString sPatternSyntaxKey = "PatternSyntax";

typedef std::map<QRegExp::PatternSyntax, QString> PatternSyntaxMap;
static const PatternSyntaxMap patternSyntaxMap =
{
	{ QRegExp::FixedString, "ContainingString" },
	{ QRegExp::RegExp, "RegExp" },
	{ QRegExp::WildcardUnix, "Wildcard" },
};

static QRegExp::PatternSyntax stringToPatternSyntax(const QString &str)
{
	for (auto &item : patternSyntaxMap)
	{
		if (item.second == str)
			return item.first;
	}

	return QRegExp::FixedString;
}

static QString patternSyntaxToString(QRegExp::PatternSyntax syntax)
{
	auto it = patternSyntaxMap.find(syntax);

	if (it != patternSyntaxMap.end())
		return it->second;

	return QString();
}

ProjectTreeWidget::ProjectTreeWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ProjectTreeWidget)
{
	ui->setupUi(this);

	QSettings settings;

	settings.beginGroup(metaObject()->className());
	pattern_syntax = stringToPatternSyntax(settings.value(sPatternSyntaxKey).toString());
	settings.endGroup();

	switch (pattern_syntax)
	{
		case QRegExp::RegExp:
			ui->actionOptionsRegExp->setChecked(true);
			break;

		case QRegExp::WildcardUnix:
			ui->actionOptionsWildcard->setChecked(true);
			break;

		case QRegExp::FixedString:
			ui->actionOptionsContainingString->setChecked(true);
			break;

		default:
			break;
	}

	QObject::connect(ui->filterEdit, &QLineEdit::textChanged, this, &ProjectTreeWidget::onFilterTextChanged);

	auto menu = new QMenu(this);

	menu->addAction(ui->actionOptionsContainingString);
	menu->addAction(ui->actionOptionsWildcard);
	menu->addAction(ui->actionOptionsRegExp);

	ui->buttonOptions->setMenu(menu);
}

ProjectTreeWidget::~ProjectTreeWidget()
{
	delete ui;
}

ProjectTreeView *ProjectTreeWidget::getTreeView() const
{
	return ui->projectTreeView;
}

QLineEdit *ProjectTreeWidget::getFilterEdit() const
{
	return ui->filterEdit;
}

void ProjectTreeWidget::showFilter(bool show)
{
	if (!show)
		ui->filterEdit->clear();
	ui->widget->setVisible(show);
}

QMenu *ProjectTreeWidget::getButtonOptionsMenu() const
{
	return ui->buttonOptions->menu();
}

void ProjectTreeWidget::onFilterTextChanged()
{
	if (ui->filterEdit->text().isEmpty())
		updateFilter();
}

void ProjectTreeWidget::on_filterEdit_editingFinished()
{
	updateFilter();
}

void ProjectTreeWidget::updateFilter(bool force)
{
	QString text(ui->filterEdit->text());

	if (force || currentFilter != text)
	{
		currentFilter = text;

		auto model = ui->projectTreeView->getFilterModel();
		Q_ASSERT(nullptr != model);

		auto pattern_syntax = this->pattern_syntax;

		if (pattern_syntax == QRegExp::FixedString)
		{
			if (!text.isEmpty())
				text = ".*" + QRegExp::escape(text) + ".*";
			pattern_syntax = QRegExp::RegExp;
		}

		model->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, pattern_syntax));
	}
}

void ProjectTreeWidget::setPatternSyntax(QRegExp::PatternSyntax syntax)
{
	if (pattern_syntax != syntax)
	{
		pattern_syntax = syntax;

		QSettings settings;

		settings.beginGroup(metaObject()->className());
		settings.setValue(sPatternSyntaxKey, patternSyntaxToString(syntax));
		settings.endGroup();

		updateFilter(true);
	}
}

void ProjectTreeWidget::on_actionOptionsRegExp_toggled(bool checked)
{
	if (checked)
	{
		setPatternSyntax(QRegExp::RegExp);

		ui->actionOptionsWildcard->setChecked(false);
		ui->actionOptionsContainingString->setChecked(false);
	}
}

void ProjectTreeWidget::on_actionOptionsWildcard_toggled(bool checked)
{
	if (checked)
	{
		setPatternSyntax(QRegExp::WildcardUnix);

		ui->actionOptionsRegExp->setChecked(false);
		ui->actionOptionsContainingString->setChecked(false);
	}
}

void ProjectTreeWidget::on_actionOptionsContainingString_toggled(bool checked)
{
	if (checked)
	{
		setPatternSyntax(QRegExp::FixedString);

		ui->actionOptionsRegExp->setChecked(false);
		ui->actionOptionsWildcard->setChecked(false);
	}
}
}
