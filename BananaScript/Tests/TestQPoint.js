function testVec2(p, p2, x, y)
{
	assert(!p.isNull);
	assert(!p2.isNull);
	print(p);
	print(p2);
	p[x] = Math.round(p[x]);
	p[y] = Math.round(p[y]);
	assert(p2[x] === p[x]);
	assert(p2[y] === p[y]);
	assert(p.equals(p2[x], p2[y]));
	assert(p.sub(p2).isNull);
	assert(p.sub(p2[x], p2[y]).isNull);
	assert(p.add(p2[x], p2[y]).equals(p2.add(p2)));
	assert(p.add(p2[x], p2[y]).equals(p[x] + p2[x], p[y] + p2[y]));
	assert(p.mul(0.5).equals(p2[x] * 0.5, p2[y] * 0.5));
	assert(p.div(0.5).equals(p2[x] / 0.5, p2[y] / 0.5));
	var p3 = p.clone();
	assert(p3.equals(p));
	p3[x] = 0;
	p3[y] = 0;
	assert(!p3.equals(p));
	p3.assign(p);
	assert(p3.equals(p));
}

function testQPoint()
{
	var p = new QPoint();
	assert(p.equals(0, 0));
	assert(QPointF.prototype === QPoint.prototype);
	assert(QPoint.prototype.isNull);
	assert(isNaN(QPoint.prototype.x));
	assert(isNaN(QPoint.prototype.y));
	assert(isNaN(QPoint.prototype.manhattanLength));
	assert(!QPoint.prototype.equals(p));
	assert(QPoint.prototype.sub(1, 1).equals(p));
	assert(QPoint.prototype.add(1, 1).equals(p));
	assert(QPoint.prototype.mul(1).equals(p));
	assert(QPoint.prototype.div(1).equals(p));
	assert(isNaN(QPoint.prototype.dotProduct(1, 1)));
	assert(QPoint.prototype.toPoint().equals(p));
	assert(QPoint.prototype.clone().equals(p));
	assert(QPoint.prototype.toString() === "QPoint");
	QPoint.prototype.assign({x:0, y:0});

	p = new QPointF(13.3, 14.4);
	assert(p instanceof QPointF);
	assert(p.toString() === "QPoint(13.3, 14.4)");
	var p2 = p.toPoint();
	assert(p2 instanceof QPoint);
	testVec2(p, p2, "x", "y");
	assert(p.manhattanLength === Math.abs(p.x) + Math.abs(p.y));
	assert(p.dotProduct(p2) === p.x * p2.x + p.y * p2.y);

	print("QPoint OK");
}
