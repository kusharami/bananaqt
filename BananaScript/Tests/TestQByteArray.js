var UTF8 = "UTF-8";
var UTF8_CODEC = new QByteArray(UTF8);

function badCompare(a, b)
{
	throw new Error("Bad Compare OK");
}

function descendCompare(a, b)
{
	return a > b;
}

function testEveryFalse(value, index, arr)
{
	assert(arr[index] === value);

	return false;
}

function testEveryTrue(value, index, arr)
{
	assert(arr[index] === value);

	return true;
}

function testSome20(value, index, arr)
{
	assert(arr[index] === value);

	return value === 20;
}

function testSome7(value, index, arr)
{
	assert(arr[index] === value);

	return value === 7;
}

function testForEachPlus1(value, index, arr)
{
	assert(arr[index] === value);

	arr[index] = value + 1;
}

function testMapPlus1(value, index, arr)
{
	assert(arr[index] === value);

	return value + 1;
}

function testFilterBelow20(value, index, arr)
{
	assert(arr[index] === value);

	return value < 20;
}

function testReduceSum(accumulator, value, index, arr)
{
	assert(arr[index] === value);

	return accumulator + value;
}

function testReduceRight(accumulator, value, index, arr)
{
	assert(arr[index] === value);

	return accumulator - value;
}

function testQByteArray()
{
	assert(!QByteArray.prototype.equals(""));
	assert(QByteArray.prototype.left(1).equals(""));
	assert(QByteArray.prototype.mid(1, 1).equals(""));
	assert(QByteArray.prototype.right(1).equals(""));
	assert(QByteArray.prototype.remove(1, 1) === QByteArray.prototype);
	assert(QByteArray.prototype.simplified().equals(""));
	assert(QByteArray.prototype.toLower().equals(""));
	assert(QByteArray.prototype.toUpper().equals(""));
	assert(QByteArray.prototype.trimmed().equals(""));
	assert(QByteArray.prototype.repeated(1).equals(""));
	assert(QByteArray.prototype.split(',') === undefined);
	assert(QByteArray.prototype.replace(',', '.').equals(""));
	assert(QByteArray.prototype.replace(1, 2, '.').equals(""));
	assert(QByteArray.prototype.valueOf() === "");
	assert(QByteArray.prototype.join() === "");
	assert(QByteArray.prototype.join(',') === "");
	assert(QByteArray.prototype.pop() === undefined);
	assert(QByteArray.prototype.reverse() === QByteArray.prototype);
	assert(QByteArray.prototype.shift() === undefined);
	assert(QByteArray.prototype.slice(1, 1).equals(""));
	assert(QByteArray.prototype.sort(badCompare) === QByteArray.prototype);
	assert(QByteArray.prototype.indexOf(',', 1) < 0);
	assert(QByteArray.prototype.lastIndexOf(',', 1) < 0);
	assert(!QByteArray.prototype.every(testEveryFalse));
	assert(!QByteArray.prototype.some(testEveryFalse));
	assert(QByteArray.prototype.map(testMapPlus1) === undefined);
	assert(QByteArray.prototype.mapBytes(testMapPlus1).equals(""));
	assert(QByteArray.prototype.filter(testFilterBelow20).equals(""));
	assert(QByteArray.prototype.reduce(testReduceSum) === undefined);
	assert(QByteArray.prototype.reduceRight(testReduceRight) === undefined);
	assert(QByteArray.prototype.toString() === "QByteArray");
	QByteArray.prototype.clear();
	QByteArray.prototype.truncate(1);
	QByteArray.prototype.chop(1);
	QByteArray.prototype.forEach(testForEachPlus1);

	print("Test QByteArray case conversion");
	var ba = new QByteArray(3, "A");
	assert(ba instanceof QByteArray);
	assert(ba.length === 3);
	assert(ba.toString() === "QByteArray(length=3)");
	assert(ba.toString(UTF8) === "AAA");
	ba = ba.toLower();
	assert(ba.toString(UTF8) === "aaa");
	ba = ba.toUpper();
	assert(ba.toString(UTF8) === "AAA");
	ba.clear();
	assert(ba.length === 0);

	print("Test QByteArray base functional");
	ba = new QByteArray([0xAA, 0xBB, 0xCC]);
	assert(ba.push(0x02, 0x01) === 5);
	assert(ba[ba.length - 2] === 0x02);
	assert(ba[ba.length - 1] === 0x01);
	assert(ba.unshift(0x02, 0x01) === 7);
	assert(ba[0] === 0x01);
	assert(ba[1] === 0x02);
	assert(ba.toHex().equals("0102aabbcc0201"));
	assert(ba.equals(QByteArray.fromHex("0102aabbcc0201")));
	assert(ba.slice(2, 4).toHex().equals("aabb"));
	assert(ba.pop() === 0x01);
	assert(ba.pop() === 0x02);
	assert(ba.shift() === 0x01);
	assert(ba.shift() === 0x02);
	assert(ba.toHex().equals("aabbcc"));
	print(ba.reverse().toHex());
	assert(ba.equals(new QByteArray([0xCC, 0xBB, 0xAA])));
	var baJoin = ba.join();
	print(baJoin);
	assert(baJoin === strf("%1,%2,%3", ba[0], ba[1], ba[2]));
	baJoin = new QByteArray(baJoin, UTF8_CODEC);
	baJoin = baJoin.split(",");
	assert(baJoin instanceof Array);
	assert(baJoin.length === 3);
	assert(Number(baJoin[0].toStringLatin()) === ba[0]);
	assert(Number(baJoin[1].toStringLatin()) === ba[1]);
	assert(Number(baJoin[2].toStringLatin()) === ba[2]);
	baJoin = ba.join("<->");
	assert(baJoin === strf("%1<->%2<->%3", ba[0], ba[1], ba[2]));
	ba.chop(2);
	assert(ba.length === 1);
	assert((ba.pop() & 0xFF) === 0xCC);
	assert(ba.length === 0);

	print("Test QByteArray string conversions");
	var ZHIZN = "Жизнь";
	ba = new QByteArray(ZHIZN, UTF8);
	var ZHIZN_B64 = "0JbQuNC30L3RjA==";
	assert(ba.toBase64().equals(ZHIZN_B64));
	assert(ba.equals(QByteArray.fromBase64(ZHIZN_B64)));
	var ba2 = new QByteArray(ZHIZN);
	assert(ba.toString(UTF8) === ba2.toStringLocal());
	assert(ba.toString(UTF8) === ZHIZN);
	assert(ba.toString(UTF8_CODEC) === ZHIZN);
	print(ba.toString());
	print(ba.valueOf());
	print(ba.toHex().toStringLatin());

	assert(ba.repeated(2).toString(UTF8_CODEC) === ZHIZN + ZHIZN);

	print("Test QByteArray.indexOf");
	assert(ba.indexOf(new QByteArray("Ж", UTF8_CODEC)) === 0);
	assert(ba.indexOf(new QByteArray("Жи", UTF8_CODEC)) === 0);
	var N = new QByteArray("нь", UTF8);
	var N_INDEX = ba.length - N.length;
	assert(ba.indexOf(0) === -1);
	assert(ba.indexOf(N, -1) === -1);
	assert(ba.indexOf(N) === N_INDEX);
	assert(ba.indexOf(N, N_INDEX) === N_INDEX);
	assert(ba.indexOf(N, -N.length) === N_INDEX);

	print("Test QByteArray.lastIndexOf");
	assert(ba.lastIndexOf(N) === N_INDEX);
	assert(ba.lastIndexOf(N, ba.length) === N_INDEX);
	assert(ba.lastIndexOf(N, -ba.length) === -1);
	assert(ba.lastIndexOf(N, -1) === N_INDEX);
	assert(ba.lastIndexOf(new QByteArray("ЖЖ", UTF8_CODEC)) === -1);

	print("Test QByteArray.truncate");
	ba.truncate(ba.indexOf(N));
	assert(ba.toString(UTF8) === "Жиз");
	var I = new QByteArray("и", UTF8_CODEC);
	print(ba.remove(ba.indexOf(I), I.length).valueOf());
	assert(ba.toString(UTF8) === "Жз");

	print("Test QByteArray.sort");
	ba = new QByteArray([255, 3, 6, 8, 20]);
	print(ba.sort().join());
	assert(ba.equals(new QByteArray([-1, 3, 6, 8, 20])));
	var error = false;
	try
	{
		ba.sort("bad func");
	} catch (e)
	{
		print(e);
		error = true;
	}
	assert(error);
	error = false;
	try
	{
		ba.sort(badCompare);
	} catch (e)
	{
		print(e);
		error = true;
	}
	print(ba.sort(descendCompare).join());
	assert(ba.equals(new QByteArray([20, 8, 6, 3, -1])));

	print("Test QByteArray.every");
	assert(ba.every(testEveryFalse) === false);
	assert(ba.every(testEveryTrue) === true);

	print("Test QByteArray.some");
	assert(ba.some(testSome20) === true);
	assert(ba.some(testSome7) === false);

	print("Test QByteArray.forEach");
	ba.forEach(testForEachPlus1);
	assert(ba.equals(new QByteArray([21, 9, 7, 4, 0])));
	assert(ba.some(testSome7) === true);

	print("Test QByteArray.map");
	ba2 = ba.map(testMapPlus1);
	assert(ba2 instanceof Array);
	assert(String(ba2.valueOf()) === String([22, 10, 8, 5, 1].valueOf()));

	print("Test QByteArray.mapBytes");
	ba2 = ba.mapBytes(testMapPlus1);
	assert(!(ba2 instanceof Array));
	assert(!ba.equals(ba2));
	assert(ba2.equals(new QByteArray([22, 10, 8, 5, 1])));

	print("Test QByteArray.filter");
	ba2 = ba2.filter(testFilterBelow20);
	assert(ba2.equals(new QByteArray([10, 8, 5, 1])));

	print("Test QByteArray.reduce");
	assert(ba.reduce(testReduceSum) === 0 + 21 + 9 + 7 + 4);
	assert(ba.reduce(testReduceSum, 2) === 2 + 0 + 21 + 9 + 7 + 4);

	print("Test QByteArray.reduceRight");
	assert(ba.reduceRight(testReduceRight) === 0 - 4 - 7 - 9 - 21);
	assert(ba.reduceRight(testReduceRight, 25) === 25 - 4 - 7 - 9 - 21 - 0);

	print("Test QByteArray.concat");
	ba2 = ba.concat(ba2);
	assert(!ba.equals(ba2));
	ba = ba2;
	assert(ba.equals(new QByteArray([21, 9, 7, 4, 0, 10, 8, 5, 1])));

	print("Test QByteArray.splice");
	ba.splice(1, 3, 7, 9, 21);
	ba.splice(-2, 1);
	ba.splice(-3, 1, 2);
	assert(ba.equals(new QByteArray([21, 7, 9, 21, 0, 2, 8, 1])));

	print("QByteArray OK");
}
