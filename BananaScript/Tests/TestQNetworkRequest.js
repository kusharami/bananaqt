var CONTENT_TYPE = "Content-Type";
var CONTENT_LENGTH = "Content-Length";

function testQNetworkRequest()
{
	assert(QNetworkRequest.prototype.priority === 0);
	assert(QNetworkRequest.prototype.maximumRedirectsAllowed === -1);
	assert(QNetworkRequest.prototype.url instanceof QUrl);
	assert(QNetworkRequest.prototype.header(
			   QNetworkRequest.ContentTypeHeader) === undefined);
	assert(QNetworkRequest.prototype.rawHeaderList() === undefined);
	assert(QNetworkRequest.prototype.rawHeader("header") instanceof QByteArray);
	assert(QNetworkRequest.prototype.attribute(
			   QNetworkRequest.User) === undefined);
	assert(QNetworkRequest.prototype.equals("") === false);
	assert(QNetworkRequest.prototype.toString() === "QNetworkRequest");

	assert(isFinite(QNetworkRequest.ContentTypeHeader));
	assert(isFinite(QNetworkRequest.ContentLengthHeader));
	assert(isFinite(QNetworkRequest.LocationHeader));
	assert(isFinite(QNetworkRequest.LastModifiedHeader));
	assert(isFinite(QNetworkRequest.CookieHeader));
	assert(isFinite(QNetworkRequest.SetCookieHeader));
	assert(isFinite(QNetworkRequest.ContentDispositionHeader));
	assert(isFinite(QNetworkRequest.UserAgentHeader));
	assert(isFinite(QNetworkRequest.ServerHeader));
	assert(isFinite(QNetworkRequest.HttpStatusCodeAttribute));
	assert(isFinite(QNetworkRequest.HttpReasonPhraseAttribute));
	assert(isFinite(QNetworkRequest.RedirectionTargetAttribute));
	assert(isFinite(QNetworkRequest.ConnectionEncryptedAttribute));
	assert(isFinite(QNetworkRequest.CacheLoadControlAttribute));
	assert(isFinite(QNetworkRequest.CacheSaveControlAttribute));
	assert(isFinite(QNetworkRequest.SourceIsFromCacheAttribute));
	assert(isFinite(QNetworkRequest.DoNotBufferUploadDataAttribute));
	assert(isFinite(QNetworkRequest.HttpPipeliningAllowedAttribute));
	assert(isFinite(QNetworkRequest.HttpPipeliningWasUsedAttribute));
	assert(isFinite(QNetworkRequest.CustomVerbAttribute));
	assert(isFinite(QNetworkRequest.CookieLoadControlAttribute));
	assert(isFinite(QNetworkRequest.AuthenticationReuseAttribute));
	assert(isFinite(QNetworkRequest.CookieSaveControlAttribute));
	assert(isFinite(QNetworkRequest.MaximumDownloadBufferSizeAttribute));
	assert(isFinite(QNetworkRequest.DownloadBufferAttribute));
	assert(isFinite(QNetworkRequest.SynchronousRequestAttribute));
	assert(isFinite(QNetworkRequest.BackgroundRequestAttribute));
	assert(isFinite(QNetworkRequest.SpdyAllowedAttribute));
	assert(isFinite(QNetworkRequest.SpdyWasUsedAttribute));
	assert(isFinite(QNetworkRequest.EmitAllUploadProgressSignalsAttribute));
	assert(isFinite(QNetworkRequest.FollowRedirectsAttribute));
	assert(isFinite(QNetworkRequest.User));
	assert(isFinite(QNetworkRequest.UserMax));
	assert(isFinite(QNetworkRequest.HighPriority));
	assert(isFinite(QNetworkRequest.NormalPriority));
	assert(isFinite(QNetworkRequest.LowPriority));
	assert(isFinite(QNetworkRequest.AlwaysNetwork));
	assert(isFinite(QNetworkRequest.PreferNetwork));
	assert(isFinite(QNetworkRequest.PreferCache));
	assert(isFinite(QNetworkRequest.AlwaysCache));
	assert(isFinite(QNetworkRequest.Automatic));
	assert(isFinite(QNetworkRequest.Manual));

	var r = new QNetworkRequest();
	assert(r instanceof QNetworkRequest);
	assert(r.toString() === "");
	r.url = "http://ach.com";
	assert(r.url instanceof QUrl);
	assert(r.equals(r.url));
	assert(r.toString() === r.url.toString());
	assert(r.priority > 0);
	assert(r.maximumRedirectsAllowed > 0);

	assert(!r.hasRawHeader(CONTENT_TYPE));
	r.setHeader(QNetworkRequest.ContentTypeHeader, "json");
	assert(r.header(QNetworkRequest.ContentTypeHeader) === "json");
	var rawHeaderList = r.rawHeaderList();
	assert(r.hasRawHeader(CONTENT_TYPE));
	assert(rawHeaderList.length === 1);
	assert(rawHeaderList[0].toString("latin1") === CONTENT_TYPE);
	assert(r.rawHeader(CONTENT_TYPE).toString("latin1") === "json");
	var TEST_HEADER = "TestHeader";
	r.setRawHeader(TEST_HEADER, [1,2,3,4,5]);
	assert(r.rawHeader(TEST_HEADER)[0] === 1);
	r.setRawHeader(CONTENT_LENGTH, 5);
	assert(r.header(QNetworkRequest.ContentLengthHeader) === 5);
	assert(r.attribute(QNetworkRequest.CacheSaveControlAttribute, true) === true);
	r.setAttribute(QNetworkRequest.User, [5,6,7,8]);
	assert(r.attribute(QNetworkRequest.User).toString() === "5,6,7,8");
	assert(!r.equals(r.url));

	print("QNetworkRequest OK");
}
