function testQRect()
{
	var r = new QRect();
	assert(r.equals(0, 0, 0, 0));
	assert(QRect.prototype === QRectF.prototype);
	assert(isNaN(QRect.prototype.x));
	assert(isNaN(QRect.prototype.y));
	assert(isNaN(QRect.prototype.width));
	assert(isNaN(QRect.prototype.height));
	assert(QRect.prototype.size.equals(-1, -1));
	assert(isNaN(QRect.prototype.left));
	assert(isNaN(QRect.prototype.top));
	assert(QRect.prototype.topLeft.equals(0, 0));
	assert(isNaN(QRect.prototype.right));
	assert(isNaN(QRect.prototype.bottom));
	assert(QRect.prototype.bottomRight.equals(0, 0));
	assert(QRect.prototype.center.equals(0, 0));
	assert(QRect.prototype.isNull);
	assert(QRect.prototype.empty);
	assert(QRect.prototype.valid === false);
	assert(!QRect.prototype.equals(r));
	assert(QRect.prototype.normalized().equals(r));
	assert(QRect.prototype.transposed().equals(r));
	assert(QRect.prototype.toRect().equals(r));
	assert(QRect.prototype.toAlignedRect().equals(r));
	assert(QRect.prototype.translated(0, 0).equals(r));
	assert(QRect.prototype.adjusted(0, 0, 0, 0).equals(r));
	assert(QRect.prototype.united(0, 0, 0, 0).equals(r));
	assert(QRect.prototype.intersected(0, 0, 0, 0).equals(r));
	assert(QRect.prototype.clone().equals(r));
	assert(!QRect.prototype.containsPoint(0, 0));
	assert(!QRect.prototype.containsRect(0, 0, 0, 0));
	assert(!QRect.prototype.intersects(0, 0, 0, 0));
	assert(QRect.prototype.toString() === "QRect");
	QRect.prototype.moveLeft(0.0);
	QRect.prototype.moveTop(0.0);
	QRect.prototype.moveRight(0.0);
	QRect.prototype.moveBottom(0.0);
	QRect.prototype.moveTopLeft({x:0, y:0});
	QRect.prototype.moveTopRight({x:0, y:0});
	QRect.prototype.moveBottomLeft({x:0, y:0});
	QRect.prototype.moveBottomRight({x:0, y:0});
	QRect.prototype.translate(0, 0);
	QRect.prototype.moveTo(0, 0);
	QRect.prototype.setRect(0, 0, 0, 0);
	QRect.prototype.setCoords(0, 0, 0, 0);
	QRect.prototype.adjust(0, 0, 0, 0);
	QRect.prototype.assign({x:0,y:0,width:0,height:0});

	r = new QRectF(22.3, 12.4, 33.3, 34.3);
	assert(r.toString() === "QRect(22.3, 12.4, 33.3 x 34.3)");
	assert(r instanceof QRect);
	assert(r instanceof QRectF);
	assert(!r.isNull);
	assert(r.valid);
	assert(!r.empty);
	var r2 = new QRect();
	assert(r2.isNull);
	assert(!r2.valid);
	assert(r2.empty);
	r2.assign(r);
	assert(r2.equals(r));
	r2 = r.toAlignedRect();
	assert(r2 instanceof QRect);
	assert(r2 instanceof QRectF);
	print(r2);
	assert(r2.equals(Math.floor(r.x), Math.floor(r.y),
					 Math.ceil(r.width), Math.ceil(r.height)));
	r2 = r.toRect();
	assert(r2.equals(Math.round(r.x), Math.round(r.y),
					 Math.round(r.width), Math.round(r.height)));
	r = r2.clone();
	print(r);
	assert(r.equals(r2));
	r2 = r.transposed();
	assert(r2.width === r.height);
	assert(r2.height === r.width);
	r2 = r2.transposed();
	assert(r2.width === r.width);
	assert(r2.height === r.height);
	r2.setRect(r.x - 2, r.y - 2, r.width - 4, r.height - 4);
	r2.adjust(2, 2, 6, 6);
	assert(r.equals(r2));
	assert(r.x === r2.x && r.x === r.left);
	assert(r.y === r2.y && r.y === r.top);
	assert(r.width === r2.width && r.right - r.left === r.width);
	assert(r.height === r2.height && r.bottom - r.top === r.height);
	assert(r.topLeft.equals(r.left, r.top));
	assert(r.topRight.equals(r.right, r.top));
	assert(r.bottomLeft.equals(r.left, r.bottom));
	assert(r.bottomRight.equals(r.right, r.bottom));
	assert(r.center.equals(r.x + r.width * 0.5, r.y + r.height * 0.5));
	assert(r.size.equals(new QSizeF(r2.width, r2.height)));
	assert(r.equals(r2.left, r2.top, r2.width, r2.height));
	assert(r.containsPoint(new QPointF(r.left + 2, r.top + 3)));
	assert(!r.containsPoint(r.left - 1, r.top + 3));
	assert(r.containsRect(r.left + 1, r.top + 10, 5, 5));
	assert(!r.containsRect(new QRect(r.left - 1, r.top - 2, 1, 1)));
	assert(r.intersects(r.left - 1, r.top - 2, 5, 5));
	assert(!r.intersects(r.left - 1, r.top - 2, 1, 1));
	var rAdjusted = r.adjusted(2, 2, -2, -2);
	print(rAdjusted);
	rAdjusted.left = rAdjusted.left;
	assert(rAdjusted.equals(r.left + 2, r.top + 2, r.width - 4, r.height - 4));
	assert(r.united(new QRect(r.left - 1, r.top - 2, 1, 1)).equals(
			   r.left - 1, r.top - 2,
			   r.right - (r.left - 1), r.bottom - (r.top - 2)));
	assert(r.intersected(new QRect(r.left - 1, r.top - 2, 20, 20)).equals(
			   r.left, r.top, 19, 18));
	r2.moveLeft(3);
	assert(r2.left === 3)
	r2.moveRight(3);
	assert(r2.right === 3)
	r2.moveTop(3);
	assert(r2.top === 3);
	r2.moveBottom(3);
	assert(r2.bottom === 3);
	assert(r2.size.equals(r.size));
	r2.center = new QPointF(r2.width * 0.5, r2.height * 0.5);
	print(r2);
	assert(r2.x === 0);
	assert(r2.y === 0);
	assert(r2.topLeft.equals({x: 0, y: 0}));

	r2.setCoords(r.left - 1, r.top - 1, r.right + 1, r.bottom + 1);
	assert(r2.topLeft.equals(r.left - 1, r.top - 1));
	assert(r2.bottomRight.equals(r.right + 1, r.bottom + 1));
	print("QRect OK");
}
