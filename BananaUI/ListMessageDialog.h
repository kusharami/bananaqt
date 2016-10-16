#pragma once

#include <QDialog>
#include <QMessageBox>

namespace Ui
{
	class ListMessageDialog;
}

namespace Banana
{

class ListMessageDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ListMessageDialog(QWidget *parent = nullptr);
	virtual ~ListMessageDialog();

	enum Type
	{
		None = QMessageBox::NoIcon,
		Information = QMessageBox::Information,
		Warning = QMessageBox::Warning,
		Error = QMessageBox::Critical,
		Question = QMessageBox::Question
	};

	void setText(const QString &text, Qt::TextFormat format = Qt::PlainText);

	inline Type type() const;
	void setType(Type type);

	void addListItem(const QString &text, const QString &hint);

private:
	Ui::ListMessageDialog *ui;

	Type mType;
};

ListMessageDialog::Type ListMessageDialog::type() const
{
	return mType;
}
}
