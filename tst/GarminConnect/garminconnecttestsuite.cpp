#include "garminconnecttestsuite.h"
#include "garminconnect.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QString>

GarminConnectTestSuite::GarminConnectTestSuite()
{
}

void GarminConnectTestSuite::test_toPercentEncoding_encodesColonAndSlash()
{
    // Test URL that contains ':' and '/' characters
    QString testUrl = "https://sso.garmin.com/sso/embed";

    // Encode using QUrl::toPercentEncoding()
    QByteArray encoded = QUrl::toPercentEncoding(testUrl);
    QString encodedStr = QString::fromUtf8(encoded);

    // Verify that ':' is encoded as %3A
    EXPECT_TRUE(encodedStr.contains("%3A")) << "Expected ':' to be encoded as %3A";
    EXPECT_FALSE(encodedStr.contains(":")) << "':' should not appear unencoded";

    // Verify that '/' is encoded as %2F
    EXPECT_TRUE(encodedStr.contains("%2F")) << "Expected '/' to be encoded as %2F";
    EXPECT_FALSE(encodedStr.contains("/")) << "'/' should not appear unencoded";

    // Verify complete encoding
    QString expected = "https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed";
    EXPECT_EQ(encodedStr.toStdString(), expected.toStdString())
        << "Complete URL encoding mismatch";
}

void GarminConnectTestSuite::test_fromEncodedStrictMode_preservesEncoding()
{
    // Build a properly encoded URL string
    QString baseUrl = "https://connectapi.garmin.com/oauth-service/oauth/preauthorized";
    QString ticket = "ST-12345-abcdefg";
    QString loginUrl = "https://sso.garmin.com/sso/embed";

    // Encode the login-url parameter
    QString encodedLoginUrl = QString::fromUtf8(QUrl::toPercentEncoding(loginUrl));

    // Build complete URL with encoded parameter
    QString fullUrl = baseUrl + "?ticket=" + ticket + "&login-url=" + encodedLoginUrl;

    // Parse with fromEncoded and StrictMode
    QUrl url = QUrl::fromEncoded(fullUrl.toUtf8(), QUrl::StrictMode);

    // Verify the URL is valid
    EXPECT_TRUE(url.isValid()) << "URL should be valid";

    // Get the query string back
    QString query = url.query(QUrl::FullyEncoded);

    // Verify login-url parameter is still encoded (contains %3A and %2F)
    EXPECT_TRUE(query.contains("login-url=https%3A%2F%2F"))
        << "login-url parameter should preserve encoding with %3A and %2F";

    // Verify there's no double encoding (%253A or %252F)
    EXPECT_FALSE(query.contains("%253A")) << "Should not have triple encoding %253A";
    EXPECT_FALSE(query.contains("%252F")) << "Should not have triple encoding %252F";
}

void GarminConnectTestSuite::test_QUrlQuery_doesNotEncodeColonSlash()
{
    // This test DOCUMENTS THE PROBLEM we're working around
    QUrl url("https://connectapi.garmin.com/oauth-service/oauth/preauthorized");
    QUrlQuery query;

    QString loginUrl = "https://sso.garmin.com/sso/embed";
    query.addQueryItem("login-url", loginUrl);
    url.setQuery(query);

    // Get the encoded URL
    QString fullUrl = url.toString(QUrl::FullyEncoded);

    // EXPECT FAILURE: Qt doesn't encode ':' and '/'
    EXPECT_TRUE(fullUrl.contains("login-url=https://"))
        << "This test documents that QUrlQuery does NOT encode ':' and '/' - this is the PROBLEM";

    // These assertions SHOULD fail but won't because of Qt's behavior
    EXPECT_FALSE(fullUrl.contains("login-url=https%3A%2F%2F"))
        << "QUrlQuery fails to encode - this is why we need the workaround";
}

void GarminConnectTestSuite::test_completePattern_correctEncoding()
{
    // Simulate the exact pattern used in GarminConnect::exchangeForOAuth1Token
    QString baseUrl = "https://connectapi.garmin.com/oauth-service/oauth/preauthorized";
    QString ticket = "ST-0678458-M60zV6Chzz5dHYfoxEPs-cas";
    QString ssoUrl = "https://sso.garmin.com";
    QString ssoEmbedPath = "/sso/embed";

    // Build query string with toPercentEncoding()
    QString queryString = "ticket=" + QString::fromUtf8(QUrl::toPercentEncoding(ticket)) +
                          "&login-url=" + QString::fromUtf8(QUrl::toPercentEncoding(ssoUrl + ssoEmbedPath)) +
                          "&accepts-mfa-tokens=true";

    // Build complete URL
    QString fullUrl = baseUrl + "?" + queryString;

    // Parse with fromEncoded(StrictMode)
    QUrl url = QUrl::fromEncoded(fullUrl.toUtf8(), QUrl::StrictMode);

    // Verify URL is valid
    EXPECT_TRUE(url.isValid()) << "URL should be valid";

    // Verify the fullUrl string has correct single encoding
    EXPECT_TRUE(fullUrl.contains("login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed"))
        << "URL should have single encoding: %3A and %2F";

    // Verify NO triple encoding
    EXPECT_FALSE(fullUrl.contains("%253A")) << "Should not have triple encoding %253A";
    EXPECT_FALSE(fullUrl.contains("%252F")) << "Should not have triple encoding %252F";

    // Verify NO double encoding
    EXPECT_FALSE(fullUrl.contains("%%")) << "Should not have double % encoding";

    // Verify ticket is NOT encoded (it's alphanumeric with dashes)
    EXPECT_TRUE(fullUrl.contains("ticket=" + ticket)) << "Ticket should not be modified";
}

void GarminConnectTestSuite::test_manualQueryParsing_decodesCorrectly()
{
    // Simulate the query parameter parsing in generateOAuth1AuthorizationHeader
    QString fullUrl = "https://connectapi.garmin.com/oauth?ticket=ST-123&login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed&accepts-mfa-tokens=true";

    // Extract query string
    int queryStart = fullUrl.indexOf('?');
    ASSERT_GT(queryStart, 0) << "URL should have query string";
    QString queryString = fullUrl.mid(queryStart + 1);

    // Parse manually by splitting
    QStringList queryPairs = queryString.split('&', Qt::SkipEmptyParts);
    EXPECT_EQ(queryPairs.size(), 3) << "Should have 3 query parameters";

    // Find and parse the login-url parameter
    QString loginUrlEncoded;
    QString loginUrlDecoded;

    for (const QString &pair : queryPairs) {
        int eqPos = pair.indexOf('=');
        if (eqPos > 0) {
            QString key = pair.left(eqPos);
            if (key == "login-url") {
                loginUrlEncoded = pair.mid(eqPos + 1);
                loginUrlDecoded = QUrl::fromPercentEncoding(loginUrlEncoded.toUtf8());
                break;
            }
        }
    }

    // Verify we found the parameter
    EXPECT_FALSE(loginUrlEncoded.isEmpty()) << "Should find login-url parameter";

    // Verify encoded value
    EXPECT_EQ(loginUrlEncoded.toStdString(), "https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed")
        << "Encoded value should have %3A and %2F";

    // Verify decoded value
    EXPECT_EQ(loginUrlDecoded.toStdString(), "https://sso.garmin.com/sso/embed")
        << "Decoded value should have : and /";

    // Verify re-encoding produces the same result
    QString reEncoded = QString::fromUtf8(QUrl::toPercentEncoding(loginUrlDecoded));
    EXPECT_EQ(reEncoded.toStdString(), loginUrlEncoded.toStdString())
        << "Re-encoding should produce the same encoded value";
}

void GarminConnectTestSuite::test_fromEncodedToEncoded_roundTrip()
{
    // This is the CRITICAL test that verifies url.toEncoded() returns
    // the same string we used to create the QUrl with fromEncoded()

    // Build a URL string with proper encoding (like we do in the real code)
    QString baseUrl = "https://connectapi.garmin.com/oauth-service/oauth/preauthorized";
    QString ticket = "ST-0761225-ISyC0ElcBHwUkDEhFZGT-cas";
    QString loginUrl = "https://sso.garmin.com/sso/embed";

    // Encode login-url parameter manually
    QString encodedLoginUrl = QString::fromUtf8(QUrl::toPercentEncoding(loginUrl));
    QString fullUrl = baseUrl + "?ticket=" + ticket + "&login-url=" + encodedLoginUrl + "&accepts-mfa-tokens=true";

    // Create QUrl with fromEncoded(StrictMode) - this is what we do in exchangeForOAuth1Token()
    QUrl url = QUrl::fromEncoded(fullUrl.toUtf8(), QUrl::StrictMode);

    // Extract the URL Qt will actually send using toEncoded()
    QString urlFromQt = QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));

    // CRITICAL: These MUST match for OAuth1 signature to work
    EXPECT_EQ(urlFromQt.toStdString(), fullUrl.toStdString())
        << "Qt's toEncoded() should return the exact same URL we put in with fromEncoded()";

    // Verify the login-url parameter is still correctly encoded
    EXPECT_TRUE(urlFromQt.contains("login-url=https%3A%2F%2Fsso.garmin.com"))
        << "login-url parameter should preserve %3A and %2F encoding";

    // Verify NO double encoding
    EXPECT_FALSE(urlFromQt.contains("%253A")) << "Should not have double encoding %253A";
    EXPECT_FALSE(urlFromQt.contains("%252F")) << "Should not have double encoding %252F";

    // Verify NO unencoded characters
    EXPECT_FALSE(urlFromQt.contains("login-url=https://"))
        << "login-url should not have unencoded :// characters";
}

void GarminConnectTestSuite::test_workoutDetailsJson_powerZoneUsesAverageWatts()
{
    static const char *kWorkoutJson = R"json({"atpPlanId":null,"author":null,"avgTrainingSpeed":null,"consumer":null,"consumerImageURL":null,"consumerName":null,"consumerWebsiteURL":null,"createdDate":"2026-02-28T12:40:43.0","description":"120W","descriptionI18nKey":null,"estimateType":null,"estimatedAnaerobicTrainingEffect":0,"estimatedDistanceInMeters":null,"estimatedDistanceUnit":null,"estimatedDurationInSecs":4800,"estimatedTrainingEffect":2.5,"isSessionTransitionEnabled":null,"locale":null,"ownerId":68966589,"poolLength":null,"poolLengthUnit":null,"priorityType":"REQUIRED","shared":false,"sharedWithUsers":null,"sportType":{"displayOrder":2,"sportTypeId":2,"sportTypeKey":"cycling"},"subSportType":"GENERIC","trainingEffectLabel":"AEROBIC_BASE","trainingPlanId":null,"updatedDate":null,"uploadTimestamp":null,"workoutId":null,"workoutIndex":0,"workoutName":"Base","workoutNameI18nKey":null,"workoutPhrase":"POOR_SLEEP_BASE","workoutProvider":null,"workoutSegments":[{"avgTrainingSpeed":null,"description":null,"estimateType":null,"estimatedDistanceInMeters":null,"estimatedDistanceUnit":null,"estimatedDurationInSecs":null,"poolLength":null,"poolLengthUnit":null,"segmentOrder":1,"sportType":{"displayOrder":2,"sportTypeId":2,"sportTypeKey":"cycling"},"workoutSteps":[{"category":null,"childStepId":null,"description":null,"endCondition":{"conditionTypeId":2,"conditionTypeKey":"time","displayOrder":2,"displayable":true},"endConditionCompare":null,"endConditionValue":4800,"endConditionZone":null,"equipmentType":null,"exerciseName":null,"preferredEndConditionUnit":null,"providerExerciseSourceId":null,"secondaryTargetType":null,"secondaryTargetValueOne":null,"secondaryTargetValueTwo":null,"secondaryTargetValueUnit":null,"secondaryZoneNumber":null,"stepId":null,"stepOrder":1,"stepType":{"displayOrder":3,"stepTypeId":3,"stepTypeKey":"interval"},"strokeType":null,"targetType":{"displayOrder":2,"workoutTargetTypeId":2,"workoutTargetTypeKey":"power.zone"},"targetValueOne":98,"targetValueTwo":142,"targetValueUnit":null,"type":"ExecutableStepDTO","weightUnit":null,"weightValue":null,"workoutProvider":null,"zoneNumber":null}]}],"workoutSourceId":null,"workoutThumbnailUrl":null,"workoutUuid":"3cb6ee63-7e3b-4ad8-8e07-a22d819e4985"})json";

    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(kWorkoutJson));
    ASSERT_TRUE(doc.isObject()) << "Workout JSON fixture must be valid";

    const QString xml = garminConnectGenerateWorkoutXml(doc.object());

    EXPECT_TRUE(xml.contains("<row duration=\"01:20:00\" power=\"120\"/>"))
        << "Expected average watt target 120 from bounds 98/142. XML was:\n"
        << xml.toStdString();
    EXPECT_FALSE(xml.contains("power=\"3854\""))
        << "Power must not be misinterpreted as FTP zone conversion. XML was:\n"
        << xml.toStdString();
}
