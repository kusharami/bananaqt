
function testQCryptographicHash()
{
	assert(isFinite(QCryptographicHash.Md4));
	assert(isFinite(QCryptographicHash.Md5));
	assert(isFinite(QCryptographicHash.Sha1));
	assert(isFinite(QCryptographicHash.Sha224));
	assert(isFinite(QCryptographicHash.Sha256));
	assert(isFinite(QCryptographicHash.Sha384));
	assert(isFinite(QCryptographicHash.Sha512));
	assert(isFinite(QCryptographicHash.Keccak_224));
	assert(isFinite(QCryptographicHash.Keccak_256));
	assert(isFinite(QCryptographicHash.Keccak_384));
	assert(isFinite(QCryptographicHash.Keccak_512));
	assert(isFinite(QCryptographicHash.RealSha3_224));
	assert(isFinite(QCryptographicHash.RealSha3_256));
	assert(isFinite(QCryptographicHash.RealSha3_384));
	assert(isFinite(QCryptographicHash.RealSha3_512));

	assert(QCryptographicHash.prototype.toString() === "QCryptographicHash");

	var hash = new QCryptographicHash(QCryptographicHash.Md5);
	assert(hash instanceof QCryptographicHash);
	assert(hash.algorithm === QCryptographicHash.Md5);

	var Roger = "Roger";
	hash.addData(Roger);
	var md5 = hash.result();
	assert(md5.equals(QCryptographicHash.hash(Roger, QCryptographicHash.Md5)));
	hash.reset();
	hash.addData(Roger);
	assert(md5.equals(hash.result()));
	hash.addData(Roger);
	assert(!md5.equals(hash.result()));
	print(hash.toString());

	hash.algorithm = QCryptographicHash.Md4;
	var md4 = hash.result();
	assert(!md5.equals(md4));
	print(hash.toString());
	assert(md4.equals(QCryptographicHash.hash("", QCryptographicHash.Md4)));
	hash.addData(Roger);
	assert(!md5.equals(hash.result()));
	print(hash.toString());

	print("QCryptographicHash OK");
}
