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

// Experiment GUI test script routines

function testFailed()
{
	throw new Error("Test Failed!");
}

var project_dir = projects.activeProjectDirectory;
print(project_dir.name);
print(project_dir.path);
print(project_dir.projectFile.fileName);
print(project_dir.projectFile.filePath);

var test_file = project_dir.newFile(project_dir.path + "/abraxas.frame");
print(test_file.fileName);

var frame = test_file.getData(true);
print(frame.objectName)

frame.mAlignment = GUI_Alignment_None;

print("Test QRectF constructor")
var rect = new QRectF(44, 33, 22, 99);
if (rect.x !== 44 || rect.y !== 33 || rect.width !== 22 || rect.height !== 99)
	testFailed();

print("Test QRectF equals")
var new_rect1 = new QRectF(44, 33, 22, 99)
var new_rect2 = new QRectF(22, 33, 22, 88)
if (!rect.equals(new_rect1) || rect.equals(new_rect2))
	testFailed();

print("Test QRectF property assignment")
frame.mRect = rect;
if (!rect.equals(frame.mRect))
	testFailed();

print("Test QRectF.containsPoint")
if (!frame.mRect.containsPoint({ x: 48, y: 39 })
||	frame.mRect.containsPoint({ x: 1, y: 1 }))
	testFailed();

print("Test QSizeF constructor")
var size = new QSizeF(10, 20);
if (size.width !== 10 || size.height !== 20)
	testFailed();

print("Test QSizeF equals")
var new_size1 = new QSizeF(10, 20)
var new_size2 = new QSizeF(20, 10)
if (!size.equals(new_size1) || size.equals(new_size2))
	testFailed();

print("Test QSizeF property assign")
frame.mMinSize = size;
if (!frame.mMinSize.equals(size))
	testFailed();

print("Test QPointF constructor")
var margins = new QPointF(10, 10);
if (margins.x !== 10 || margins.y !== 10)
	testFailed();

print("Test QPointF equals")
var new_point1 = new QPointF(10, 10)
var new_point2 = new QPointF(20, 25)
if (!margins.equals(new_point1) || margins.equals(new_point2))
	testFailed();

print("Test Margin sproperty assign")
frame.mStretch = true;
frame.mMargins = margins;
if (!frame.mMargins.equals(margins))
	testFailed();

print("Test QFont constructor")
var font = new QFont();
if (!font)
	testFailed();
font.pixelSize = 22;
print(QFont.StyleNormal)
print(QFont.StyleItalic)
print(QFont.StyleOblique)
font.style = QFont.StyleOblique;
print(font.pointSize);
print(font.pixelSize);
print(font.style);

print("Test QFont copy constructor")
var font2 = new QFont(font);
if (!font2 || font2.pixelSize !== 22)
	testFailed();

print("Test GUI::Label constructor")
var label = new GUI_Label();
var label_name = "TestLabel";
label.objectName = label_name;
print(label.objectName);
if (label_name !== label.objectName)
	testFailed();

label.mAlignment = GUI_Alignment_Center;
label.mFont = font;
print(label.mFont.pointSize)
print(label.mFont.pixelSize)
label.mText = "Test Text";

print("Test GUI::WidgetContainer::addWidget");
frame.addWidget(label);
if (label.objectName !== frame.findChild(label_name).objectName)
	testFailed();

label.mCustomData = {
	a: [
		{
			x: 15,
			y: 26
		},
		{
			x: 26,
			y: 15
		}
	],
	andalusia: "Andalusia",
	numbers: 99123,
	flags: {
		a: true,
		b: false,
		c: true
	}
};

print("Test system.requestNewFilePath");
var filepath = system.requestNewFilePath("Save to JSON", "test", "*.json");

if (filepath)
{
	obj = {};
	obj.x = 12;
	obj.y = [ "a", "b", "c"];

	print("Test system.saveToJson");
	system.saveToJson(obj, filepath);

	print("Test system.loadFromJson");
	var new_obj = system.loadFromJson(filepath);

	print(new_obj.x);
	print(new_obj.y[0]);
}
