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

#include "ScriptQFont.h"

#include "ScriptUtils.h"

#include "BananaCore/Const.h"

namespace Banana
{
namespace Script
{
struct Font : public QFont
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<Style, qint32>();
			QMetaType::registerConverter<StyleStrategy, qint32>();
			QMetaType::registerConverter<StyleHint, qint32>();
			QMetaType::registerConverter<Capitalization, qint32>();
			QMetaType::registerConverter<SpacingType, qint32>();
			QMetaType::registerConverter<HintingPreference, qint32>();
		}

		qScriptRegisterMetaType(
			engine, EnumToScriptValue<Style>, EnumFromScriptValue<Style>);
		qScriptRegisterMetaType(engine, EnumToScriptValue<StyleStrategy>,
			EnumFromScriptValue<StyleStrategy>);
		qScriptRegisterMetaType(engine, EnumToScriptValue<StyleHint>,
			EnumFromScriptValue<StyleHint>);
		qScriptRegisterMetaType(engine, EnumToScriptValue<Capitalization>,
			EnumFromScriptValue<Capitalization>);
		qScriptRegisterMetaType(engine, EnumToScriptValue<SpacingType>,
			EnumFromScriptValue<SpacingType>);
		qScriptRegisterMetaType(engine, EnumToScriptValue<HintingPreference>,
			EnumFromScriptValue<HintingPreference>);

		SCRIPT_REG_ENUM(to, Helvetica);
		SCRIPT_REG_ENUM(to, SansSerif);
		SCRIPT_REG_ENUM(to, Times);
		SCRIPT_REG_ENUM(to, Serif);
		SCRIPT_REG_ENUM(to, Courier);
		SCRIPT_REG_ENUM(to, TypeWriter);
		SCRIPT_REG_ENUM(to, OldEnglish);
		SCRIPT_REG_ENUM(to, Decorative);
		SCRIPT_REG_ENUM(to, System);
		SCRIPT_REG_ENUM(to, AnyStyle);
		SCRIPT_REG_ENUM(to, Cursive);
		SCRIPT_REG_ENUM(to, Monospace);
		SCRIPT_REG_ENUM(to, Fantasy);
		SCRIPT_REG_ENUM(to, StyleNormal);
		SCRIPT_REG_ENUM(to, StyleItalic);
		SCRIPT_REG_ENUM(to, StyleOblique);
		SCRIPT_REG_ENUM(to, PreferDefault);
		SCRIPT_REG_ENUM(to, PreferBitmap);
		SCRIPT_REG_ENUM(to, PreferDevice);
		SCRIPT_REG_ENUM(to, PreferOutline);
		SCRIPT_REG_ENUM(to, ForceOutline);
		SCRIPT_REG_ENUM(to, PreferMatch);
		SCRIPT_REG_ENUM(to, PreferQuality);
		SCRIPT_REG_ENUM(to, PreferAntialias);
		SCRIPT_REG_ENUM(to, PreferDefaultHinting);
		SCRIPT_REG_ENUM(to, PreferNoHinting);
		SCRIPT_REG_ENUM(to, PreferVerticalHinting);
		SCRIPT_REG_ENUM(to, PreferFullHinting);
		SCRIPT_REG_ENUM(to, NoAntialias);
		SCRIPT_REG_ENUM(to, OpenGLCompatible);
		SCRIPT_REG_ENUM(to, ForceIntegerMetrics);
		SCRIPT_REG_ENUM(to, NoSubpixelAntialias);
		SCRIPT_REG_ENUM(to, NoFontMerging);
		SCRIPT_REG_ENUM(to, Thin);
		SCRIPT_REG_ENUM(to, ExtraLight);
		SCRIPT_REG_ENUM(to, Light);
		SCRIPT_REG_ENUM(to, Normal);
		SCRIPT_REG_ENUM(to, Medium);
		SCRIPT_REG_ENUM(to, DemiBold);
		SCRIPT_REG_ENUM(to, Bold);
		SCRIPT_REG_ENUM(to, ExtraBold);
		SCRIPT_REG_ENUM(to, Black);
		SCRIPT_REG_ENUM(to, UltraCondensed);
		SCRIPT_REG_ENUM(to, ExtraCondensed);
		SCRIPT_REG_ENUM(to, Condensed);
		SCRIPT_REG_ENUM(to, SemiCondensed);
		SCRIPT_REG_ENUM(to, Unstretched);
		SCRIPT_REG_ENUM(to, SemiExpanded);
		SCRIPT_REG_ENUM(to, Expanded);
		SCRIPT_REG_ENUM(to, ExtraExpanded);
		SCRIPT_REG_ENUM(to, UltraExpanded);
		SCRIPT_REG_ENUM(to, MixedCase);
		SCRIPT_REG_ENUM(to, AllUppercase);
		SCRIPT_REG_ENUM(to, AllLowercase);
		SCRIPT_REG_ENUM(to, SmallCaps);
		SCRIPT_REG_ENUM(to, Capitalize);
		SCRIPT_REG_ENUM(to, PercentageSpacing);
		SCRIPT_REG_ENUM(to, AbsoluteSpacing);
	}
};
}

using namespace Script;

void ScriptQFont::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, ToScriptValue, FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQFont>(engine);
	engine->setDefaultPrototype(qMetaTypeId<QFont *>(), proto);
	auto qFont = engine->newFunction(Constructor, proto);
	qFont.setData(proto);

	Font::RegisterEnums(engine, qFont);

	engine->globalObject().setProperty(className(), qFont, STATIC_SCRIPT_VALUE);
}

ScriptQFont::ScriptQFont(QObject *parent)
	: QObject(parent)
{
}

QString ScriptQFont::family() const
{
	auto font = thisFont();
	if (font)
		return font->family();

	return QString();
}

void ScriptQFont::setFamily(const QString &f)
{
	auto font = thisFont();
	if (font)
		font->setFamily(f);
}

QString ScriptQFont::styleName() const
{
	auto font = thisFont();
	if (font)
		return font->styleName();

	return QString();
}

void ScriptQFont::setStyleName(const QString &s)
{
	auto font = thisFont();
	if (font)
		font->setStyleName(s);
}

qreal ScriptQFont::pointSize() const
{
	auto font = thisFont();
	if (font)
		return font->pointSizeF();

	return qQNaN();
}

void ScriptQFont::setPointSize(qreal s)
{
	auto font = thisFont();
	if (font)
		font->setPointSizeF(s);
}

int ScriptQFont::pixelSize() const
{
	auto font = thisFont();
	if (font)
		return font->pixelSize();

	return -1;
}

void ScriptQFont::setPixelSize(int s)
{
	auto font = thisFont();
	if (font)
		font->setPixelSize(s);
}

int ScriptQFont::weight() const
{
	auto font = thisFont();
	if (font)
		return font->weight();

	return -1;
}

void ScriptQFont::setWeight(int w)
{
	auto font = thisFont();
	if (font)
		font->setWeight(w);
}

bool ScriptQFont::bold() const
{
	auto font = thisFont();
	if (font)
		return font->bold();

	return false;
}

void ScriptQFont::setBold(bool b)
{
	auto font = thisFont();
	if (font)
		font->setBold(b);
}

QFont::Style ScriptQFont::style() const
{
	auto font = thisFont();
	if (font)
		return font->style();

	return QFont::Style(-1);
}

void ScriptQFont::setStyle(QFont::Style style)
{
	auto font = thisFont();
	if (font)
		font->setStyle(style);
}

bool ScriptQFont::italic() const
{
	auto font = thisFont();
	if (font)
		return font->italic();

	return false;
}

void ScriptQFont::setItalic(bool b)
{
	auto font = thisFont();
	if (font)
		font->setItalic(b);
}

bool ScriptQFont::underline() const
{
	auto font = thisFont();
	if (font)
		return font->underline();

	return false;
}

void ScriptQFont::setUnderline(bool b)
{
	auto font = thisFont();
	if (font)
		font->setUnderline(b);
}

bool ScriptQFont::overline() const
{
	auto font = thisFont();
	if (font)
		return font->overline();

	return false;
}

void ScriptQFont::setOverline(bool b)
{
	auto font = thisFont();
	if (font)
		font->setOverline(b);
}

bool ScriptQFont::strikeOut() const
{
	auto font = thisFont();
	if (font)
		return font->strikeOut();

	return false;
}

void ScriptQFont::setStrikeOut(bool b)
{
	auto font = thisFont();
	if (font)
		font->setStrikeOut(b);
}

bool ScriptQFont::fixedPitch() const
{
	auto font = thisFont();
	if (font)
		return font->fixedPitch();

	return false;
}

void ScriptQFont::setFixedPitch(bool b)
{
	auto font = thisFont();
	if (font)
		font->setFixedPitch(b);
}

bool ScriptQFont::kerning() const
{
	auto font = thisFont();
	if (font)
		return font->kerning();

	return false;
}

void ScriptQFont::setKerning(bool b)
{
	auto font = thisFont();
	if (font)
		font->setKerning(b);
}

QFont::StyleHint ScriptQFont::styleHint() const
{
	auto font = thisFont();
	if (font)
		return font->styleHint();

	return QFont::StyleHint(-1);
}

QFont::StyleStrategy ScriptQFont::styleStrategy() const
{
	auto font = thisFont();
	if (font)
		return font->styleStrategy();

	return QFont::StyleStrategy(0);
}

void ScriptQFont::setStyleStrategy(QFont::StyleStrategy s)
{
	auto font = thisFont();
	if (font)
		font->setStyleStrategy(s);
}

int ScriptQFont::stretch() const
{
	auto font = thisFont();
	if (font)
		return font->stretch();

	return -1;
}

void ScriptQFont::setStretch(int i)
{
	auto font = thisFont();
	if (font)
		font->setStretch(i);
}

qreal ScriptQFont::letterSpacing() const
{
	auto font = thisFont();
	if (font)
		return font->letterSpacing();

	return qQNaN();
}

QFont::SpacingType ScriptQFont::letterSpacingType() const
{
	auto font = thisFont();
	if (font)
		return font->letterSpacingType();

	return QFont::SpacingType(-1);
}

qreal ScriptQFont::wordSpacing() const
{
	auto font = thisFont();
	if (font)
		return font->wordSpacing();

	return qQNaN();
}

void ScriptQFont::setWordSpacing(qreal spacing)
{
	auto font = thisFont();
	if (font)
		font->setWordSpacing(spacing);
}

QFont::Capitalization ScriptQFont::capitalization() const
{
	auto font = thisFont();
	if (font)
		return font->capitalization();

	return QFont::Capitalization(-1);
}

void ScriptQFont::setCapitalization(QFont::Capitalization c)
{
	auto font = thisFont();
	if (font)
		font->setCapitalization(c);
}

QFont::HintingPreference ScriptQFont::hintingPreference() const
{
	auto font = thisFont();
	if (font)
		return font->hintingPreference();

	return QFont::HintingPreference(-1);
}

void ScriptQFont::setHintingPreference(QFont::HintingPreference p)
{
	auto font = thisFont();
	if (font)
		font->setHintingPreference(p);
}

QString ScriptQFont::key() const
{
	auto font = thisFont();
	if (font)
		return font->key();

	return className();
}

QString ScriptQFont::defaultFamily() const
{
	auto font = thisFont();
	if (font)
		return font->defaultFamily();

	return QString();
}

QString ScriptQFont::lastResortFamily() const
{
	auto font = thisFont();
	if (font)
		return font->lastResortFamily();

	return QString();
}

QString ScriptQFont::lastResortFont() const
{
	auto font = thisFont();
	if (font)
		return font->lastResortFont();

	return QString();
}

void ScriptQFont::setLetterSpacing(QFont::SpacingType type, qreal spacing)
{
	auto font = thisFont();
	if (font)
		font->setLetterSpacing(type, spacing);
}

void ScriptQFont::setStyleHint(QFont::StyleHint h, QFont::StyleStrategy s)
{
	auto font = thisFont();
	if (font)
		font->setStyleHint(h, s);
}

bool ScriptQFont::exactMatch() const
{
	auto font = thisFont();
	if (font)
		return font->exactMatch();

	return false;
}

bool ScriptQFont::equals(const QScriptValue &other) const
{
	auto font = thisFont();
	if (font)
	{
		QFont fnt;
		FromScriptValue(other, fnt);
		return *font == fnt;
	}

	return false;
}

QString ScriptQFont::toString() const
{
	auto font = thisFont();
	if (font)
		return font->toString();

	return className();
}

bool ScriptQFont::fromString(const QString &str)
{
	bool result = false;
	auto font = thisFont();
	if (font)
	{
		result = font->fromString(str);

		if (result)
			font->setStyleStrategy(QFont::PreferAntialias);
	}

	return result;
}

QString ScriptQFont::className()
{
	return QSTRKEY(QFont);
}

QFont *ScriptQFont::thisFont() const
{
	return qscriptvalue_cast<QFont *>(thisObject().data());
}

QScriptValue ScriptQFont::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QFont font;
	if (context->argumentCount() == 1)
	{
		FromScriptValue(context->argument(0), font);
	} else if (context->argumentCount() != 0)
	{
		return ThrowBadNumberOfArguments(context);
	}

	return ToScriptValue(engine, font);
}

QScriptValue ScriptQFont::ToScriptValue(QScriptEngine *engine, const QFont &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QVariant::fromValue(in)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQFont::FromScriptValue(const QScriptValue &object, QFont &out)
{
	auto fi = qscriptvalue_cast<QFont *>(object.data());
	out = fi ? *fi : QFont(object.toString());
}
}
