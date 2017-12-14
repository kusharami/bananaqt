/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#pragma once

#include <QObject>
#include <QFont>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QFont *)
Q_DECLARE_METATYPE(QFont::Style)
Q_DECLARE_METATYPE(QFont::StyleHint)
Q_DECLARE_METATYPE(QFont::SpacingType)
Q_DECLARE_METATYPE(QFont::Capitalization)
Q_DECLARE_METATYPE(QFont::HintingPreference)

namespace Banana
{
class ScriptQFont
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(QString key READ key)
	Q_PROPERTY(QString defaultFamily READ defaultFamily)
	Q_PROPERTY(QString lastResortFamily READ lastResortFamily)
	Q_PROPERTY(QString lastResortFont READ lastResortFont)
	Q_PROPERTY(QString family READ family WRITE setFamily)
	Q_PROPERTY(QString styleName READ styleName WRITE setStyleName)
	Q_PROPERTY(qreal pointSize READ pointSize WRITE setPointSize)
	Q_PROPERTY(int pixelSize READ pixelSize WRITE setPixelSize)
	Q_PROPERTY(int weight READ weight WRITE setWeight)
	Q_PROPERTY(int stretch READ stretch WRITE setStretch)
	Q_PROPERTY(bool bold READ bold WRITE setBold)
	Q_PROPERTY(bool italic READ italic WRITE setItalic)
	Q_PROPERTY(bool underline READ underline WRITE setUnderline)
	Q_PROPERTY(bool overline READ overline WRITE setOverline)
	Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut)
	Q_PROPERTY(bool fixedPitch READ fixedPitch WRITE setFixedPitch)
	Q_PROPERTY(bool kerning READ kerning WRITE setKerning)
	Q_PROPERTY(QFont::Style style READ style WRITE setStyle)
	Q_PROPERTY(QFont::StyleHint styleHint READ styleHint WRITE setStyleHint)
	Q_PROPERTY(QFont::StyleStrategy styleStrategy READ styleStrategy WRITE
			setStyleStrategy)
	Q_PROPERTY(qreal letterSpacing READ letterSpacing)
	Q_PROPERTY(QFont::SpacingType letterSpacingType READ letterSpacingType)
	Q_PROPERTY(qreal wordSpacing READ wordSpacing WRITE setWordSpacing)
	Q_PROPERTY(QFont::Capitalization capitalization READ capitalization WRITE
			setCapitalization)
	Q_PROPERTY(QFont::HintingPreference hintingPreference READ hintingPreference
			WRITE setHintingPreference)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQFont(QObject *parent);

	QString family() const;
	void setFamily(const QString &f);

	QString styleName() const;
	void setStyleName(const QString &s);

	qreal pointSize() const;
	void setPointSize(qreal s);

	int pixelSize() const;
	void setPixelSize(int s);

	int weight() const;
	void setWeight(int w);

	bool bold() const;
	void setBold(bool b);

	QFont::Style style() const;
	void setStyle(QFont::Style style);

	bool italic() const;
	void setItalic(bool b);

	bool underline() const;
	void setUnderline(bool b);

	bool overline() const;
	void setOverline(bool b);

	bool strikeOut() const;
	void setStrikeOut(bool b);

	bool fixedPitch() const;
	void setFixedPitch(bool b);

	bool kerning() const;
	void setKerning(bool b);

	QFont::StyleHint styleHint() const;

	QFont::StyleStrategy styleStrategy() const;
	void setStyleStrategy(QFont::StyleStrategy s);

	int stretch() const;
	void setStretch(int i);

	qreal letterSpacing() const;
	QFont::SpacingType letterSpacingType() const;

	qreal wordSpacing() const;
	void setWordSpacing(qreal spacing);

	QFont::Capitalization capitalization() const;
	void setCapitalization(QFont::Capitalization c);

	QFont::HintingPreference hintingPreference() const;
	void setHintingPreference(QFont::HintingPreference p);

	QString key() const;

	QString defaultFamily() const;
	QString lastResortFamily() const;
	QString lastResortFont() const;

public slots:
	void setLetterSpacing(QFont::SpacingType type, qreal spacing);
	void setStyleHint(
		QFont::StyleHint h, QFont::StyleStrategy s = QFont::PreferDefault);
	bool exactMatch() const;
	virtual bool equals(const QScriptValue &other) const;

	QString toString() const;
	bool fromString(const QString &);

protected:
	virtual QFont *thisFont() const;

private:
	static QString className();

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QFont &in);
	static void FromScriptValue(const QScriptValue &object, QFont &out);
};
}
