importScript("TestQPoint.js")

function testQSize()
{
	var size = new QSize();
	assert(size.equals(-1, -1));
	assert(isFinite(QSize.IgnoreAspectRatio));
	assert(isFinite(QSize.KeepAspectRatio));
	assert(isFinite(QSize.KeepAspectRatioByExpanding));

	assert(QSizeF.prototype === QSize.prototype);
	assert(QSize.prototype.isNull);
	assert(QSize.prototype.empty);
	assert(QSize.prototype.valid === false);
	assert(isNaN(QSize.prototype.width));
	assert(isNaN(QSize.prototype.height));
	assert(!QSize.prototype.equals(size));
	assert(QSize.prototype.sub(1, 1).equals(size));
	assert(QSize.prototype.add(1, 1).equals(size));
	assert(QSize.prototype.mul(1).equals(size));
	assert(QSize.prototype.div(1).equals(size));
	assert(QSize.prototype.toSize().equals(size));
	assert(QSize.prototype.clone().equals(size));
	assert(QSize.prototype.transposed().equals(size));
	assert(QSize.prototype.scaled(1, 1, QSize.IgnoreAspectRatio).equals(size));
	assert(QSize.prototype.expandedTo(1, 1).equals(size));
	assert(QSize.prototype.boundedTo(1, 1).equals(size));
	assert(QSize.prototype.toString() === "QSize");
	QSize.prototype.transpose();
	QSize.prototype.scale(1, 1, QSize.IgnoreAspectRatio);
	QSize.prototype.assign({width:0, height:0});

	size = new QSizeF(23.3, 24.7);
	assert(size instanceof QSize);
	assert(size instanceof QSizeF);
	assert(size.toString() === "QSize(23.3 x 24.7)");
	var size2 = size.toSize();
	assert(size2 instanceof QSize);
	assert(size2 instanceof QSizeF);
	testVec2(size, size2, "width", "height");
	assert(!size.empty);
	assert(size.valid);
	size2.width = 0;
	size2.height = 0;
	assert(size2.empty);
	assert(size2.valid);
	size2.width = -1;
	size2.height = -2;
	assert(size2.empty);
	assert(!size2.valid);
	size2 = size.transposed();
	assert(size2.width === size.height);
	assert(size2.height === size.width);
	size2.transpose();
	assert(size2.equals(size));
	size2.width = 10;
	size2.height = 12;
	assert(size2.scaled(60, 60, QSizeF.IgnoreAspectRatio).equals(60, 60));
	assert(size2.scaled(60, 60, QSizeF.KeepAspectRatio).equals(50, 60));
	size2.scale(60, 60, QSizeF.KeepAspectRatioByExpanding);
	assert(size2.equals(60, 72));
	assert(size.expandedTo(size2).equals(Math.max(size.width, size2.width),
										 Math.max(size.height, size2.height)));
	assert(size.boundedTo(size2).equals(Math.min(size.width, size2.width),
										Math.min(size.height, size2.height)));
	print("QSize OK");
}
