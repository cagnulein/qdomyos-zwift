#include "garminconnecttestsuite.h"
#include "garminconnect.h"
#include <QDate>
#include <QJsonArray>
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

void GarminConnectTestSuite::test_calendarFallbackJson_selectsNearestUpcomingWorkout()
{
    static const char *kCalendarJson = R"json({
        "startDayOfMonth": 0,
        "numOfDaysInMonth": 28,
        "month": 1,
        "year": 2026,
        "calendarItems": [
            {"id": 1771393014302, "itemType": "weight", "date": "2026-02-18", "workoutUuid": null},
            {"id": 1771993942000, "itemType": "fbtAdaptiveWorkout", "title": "Base", "date": "2026-02-25",
             "sportTypeKey": "running", "workoutUuid": "09483634-794c-4034-b971-9cd80455018a"},
            {"id": 1772166742000, "itemType": "fbtAdaptiveWorkout", "title": "Anaerobico", "date": "2026-02-27",
             "sportTypeKey": "running", "workoutUuid": "708e2be1-e43d-4576-b7d3-c9bf8dc8fdea"},
            {"id": 1772253142000, "itemType": "fbtAdaptiveWorkout", "title": "Lunga corsa", "date": "2026-02-28",
             "sportTypeKey": "running", "workoutUuid": "c80a56c4-f2a1-4046-a3b4-e6dd4730c971"}
        ]
    })json";

    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(kCalendarJson));
    ASSERT_TRUE(doc.isObject()) << "Calendar JSON fixture must be valid";

    const QDate anchorDate = QDate::fromString("2026-02-25", Qt::ISODate);
    ASSERT_TRUE(anchorDate.isValid());

    const QJsonArray items = doc.object()["calendarItems"].toArray();
    int bestDaysAhead = 4;
    QJsonObject bestItem;
    for (const QJsonValue &itemVal : items) {
        const QJsonObject item = itemVal.toObject();
        const QString itemType = item["itemType"].toString();
        if (itemType != "workout" && itemType != "fbtAdaptiveWorkout") {
            continue;
        }
        if (itemType == "workout") {
            const qint64 scheduleId = item["id"].toVariant().toLongLong();
            if (scheduleId <= 0) {
                continue;
            }
        } else {
            const QString workoutUuid = item["workoutUuid"].toString();
            if (workoutUuid.trimmed().isEmpty()) {
                continue;
            }
        }

        const QDate itemDate = QDate::fromString(item["date"].toString(), Qt::ISODate);
        if (!itemDate.isValid()) {
            continue;
        }

        const int daysAhead = anchorDate.daysTo(itemDate);
        if (daysAhead < 0 || daysAhead > 3) {
            continue;
        }
        if (daysAhead >= bestDaysAhead) {
            continue;
        }
        bestDaysAhead = daysAhead;
        bestItem = item;
    }

    ASSERT_FALSE(bestItem.isEmpty()) << "Expected at least one planned workout in fallback window";
    EXPECT_EQ(bestItem["itemType"].toString().toStdString(), "fbtAdaptiveWorkout");
    EXPECT_EQ(bestItem["date"].toString().toStdString(), "2026-02-25");
    EXPECT_EQ(bestItem["title"].toString().toStdString(), "Base");
    EXPECT_EQ(bestItem["workoutUuid"].toString().toStdString(), "09483634-794c-4034-b971-9cd80455018a");
}

void GarminConnectTestSuite::test_scheduleJson_nestedWorkoutPayloadParses()
{
    static const char *kScheduleJson = R"json({
        "workoutScheduleId": 1567468629,
        "workout": {
            "workoutId": 1485541572,
            "workoutName": "Long run",
            "sportType": {"sportTypeId": 1, "sportTypeKey": "running", "displayOrder": 1},
            "estimatedDurationInSecs": 6420,
            "workoutSegments": [{
                "segmentOrder": 1,
                "sportType": {"sportTypeId": 1, "sportTypeKey": "running", "displayOrder": 1},
                "workoutSteps": [{
                    "type": "ExecutableStepDTO",
                    "stepOrder": 1,
                    "stepType": {"stepTypeId": 3, "stepTypeKey": "interval", "displayOrder": 3},
                    "endCondition": {"conditionTypeId": 3, "conditionTypeKey": "distance", "displayOrder": 3, "displayable": true},
                    "endConditionValue": 17702.784,
                    "targetType": {"workoutTargetTypeId": 6, "workoutTargetTypeKey": "pace.zone", "displayOrder": 6},
                    "targetValueOne": 2.9802667,
                    "targetValueTwo": 2.5545143
                }]
            }]
        },
        "calendarDate": "2026-02-28"
    })json";

    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(kScheduleJson));
    ASSERT_TRUE(doc.isObject()) << "Schedule JSON fixture must be valid";

    const QJsonObject root = doc.object();
    ASSERT_TRUE(root["workout"].isObject()) << "Schedule fixture must contain nested workout object";

    const QJsonObject workoutPayload = root["workout"].toObject();
    const QString xml = garminConnectGenerateWorkoutXml(workoutPayload);

    EXPECT_TRUE(xml.contains("<rows>")) << "Expected workout XML root. XML was:\n" << xml.toStdString();
    EXPECT_TRUE(xml.contains("speed=\"")) << "Expected running speed target from nested workout payload. XML was:\n"
                                           << xml.toStdString();
    EXPECT_TRUE(xml.contains("forcespeed=\"1\"")) << "Expected forced speed for pace/speed targets. XML was:\n"
                                                   << xml.toStdString();
    EXPECT_FALSE(xml.contains("power=\"3854\""))
        << "Nested schedule payload must not produce invalid power conversion. XML was:\n"
        << xml.toStdString();
}

void GarminConnectTestSuite::test_scheduleJson_realLogDistanceWorkoutUsesDistanceOnly()
{
    static const char *kScheduleJsonFromLog = R"json({"associatedActivityDateTime":null,"associatedActivityId":null,"atpPlanTypeId":null,"calendarDate":"2026-02-27","consumer":null,"createdDate":"2026-02-27","itp":false,"nameChanged":false,"newName":null,"ownerId":116920806,"priority":null,"protected":false,"race":false,"tpType":null,"workout":{"atpPlanId":null,"author":null,"avgTrainingSpeed":2.8351564809265977,"consumer":null,"consumerImageURL":null,"consumerName":null,"consumerWebsiteURL":null,"createdDate":"2026-02-26T16:53:34.0","description":null,"descriptionI18nKey":null,"estimateType":"TIME_ESTIMATED","estimatedDistanceInMeters":12874.752,"estimatedDistanceUnit":{"factor":160934.4,"unitId":4,"unitKey":"mile"},"estimatedDurationInSecs":3660,"isSessionTransitionEnabled":null,"locale":null,"ownerId":116920806,"poolLength":null,"poolLengthUnit":null,"shared":false,"sharedWithUsers":null,"sportType":{"displayOrder":1,"sportTypeId":1,"sportTypeKey":"running"},"subSportType":"GENERIC","trainingPlanId":null,"updatedDate":"2026-02-26T23:13:46.0","uploadTimestamp":null,"workoutId":1486746866,"workoutName":"6 @ HM pace 630/mi","workoutNameI18nKey":null,"workoutProvider":"null","workoutSegments":[{"avgTrainingSpeed":null,"description":null,"estimateType":null,"estimatedDistanceInMeters":null,"estimatedDistanceUnit":null,"estimatedDurationInSecs":null,"poolLength":null,"poolLengthUnit":null,"segmentOrder":1,"sportType":{"displayOrder":1,"sportTypeId":1,"sportTypeKey":"running"},"workoutSteps":[{"category":null,"childStepId":null,"description":null,"endCondition":{"conditionTypeId":3,"conditionTypeKey":"distance","displayOrder":3,"displayable":true},"endConditionCompare":null,"endConditionValue":1609.34400001,"endConditionZone":null,"equipmentType":{"displayOrder":0,"equipmentTypeId":0,"equipmentTypeKey":null},"exerciseName":null,"preferredEndConditionUnit":{"factor":160934.4,"unitId":4,"unitKey":"mile"},"providerExerciseSourceId":null,"secondaryTargetType":null,"secondaryTargetValueOne":null,"secondaryTargetValueTwo":null,"secondaryTargetValueUnit":null,"secondaryZoneNumber":null,"stepId":12570918089,"stepOrder":1,"stepType":{"displayOrder":1,"stepTypeId":1,"stepTypeKey":"warmup"},"strokeType":{"displayOrder":0,"strokeTypeId":0,"strokeTypeKey":null},"targetType":{"displayOrder":6,"workoutTargetTypeId":6,"workoutTargetTypeKey":"pace.zone"},"targetValueOne":2.5957161,"targetValueTwo":2.5545143,"targetValueUnit":null,"type":"ExecutableStepDTO","weightUnit":{"factor":453.59237,"unitId":9,"unitKey":"pound"},"weightValue":null,"workoutProvider":null,"zoneNumber":null},{"category":null,"childStepId":null,"description":null,"endCondition":{"conditionTypeId":3,"conditionTypeKey":"distance","displayOrder":3,"displayable":true},"endConditionCompare":null,"endConditionValue":9656.06400002,"endConditionZone":null,"equipmentType":{"displayOrder":0,"equipmentTypeId":0,"equipmentTypeKey":null},"exerciseName":null,"preferredEndConditionUnit":{"factor":160934.4,"unitId":4,"unitKey":"mile"},"providerExerciseSourceId":null,"secondaryTargetType":null,"secondaryTargetValueOne":null,"secondaryTargetValueTwo":null,"secondaryTargetValueUnit":null,"secondaryZoneNumber":null,"stepId":12570918090,"stepOrder":2,"stepType":{"displayOrder":3,"stepTypeId":3,"stepTypeKey":"interval"},"strokeType":{"displayOrder":0,"strokeTypeId":0,"strokeTypeKey":null},"targetType":{"displayOrder":6,"workoutTargetTypeId":6,"workoutTargetTypeKey":"pace.zone"},"targetValueOne":4.1265231,"targetValueTwo":3.9736889,"targetValueUnit":null,"type":"ExecutableStepDTO","weightUnit":{"factor":453.59237,"unitId":9,"unitKey":"pound"},"weightValue":null,"workoutProvider":null,"zoneNumber":null},{"category":null,"childStepId":null,"description":null,"endCondition":{"conditionTypeId":3,"conditionTypeKey":"distance","displayOrder":3,"displayable":true},"endConditionCompare":null,"endConditionValue":1609.34400001,"endConditionZone":null,"equipmentType":{"displayOrder":0,"equipmentTypeId":0,"equipmentTypeKey":null},"exerciseName":null,"preferredEndConditionUnit":{"factor":160934.4,"unitId":4,"unitKey":"mile"},"providerExerciseSourceId":null,"secondaryTargetType":null,"secondaryTargetValueOne":null,"secondaryTargetValueTwo":null,"secondaryTargetValueUnit":null,"secondaryZoneNumber":null,"stepId":12570918091,"stepOrder":3,"stepType":{"displayOrder":2,"stepTypeId":2,"stepTypeKey":"cooldown"},"strokeType":{"displayOrder":0,"strokeTypeId":0,"strokeTypeKey":null},"targetType":{"displayOrder":6,"workoutTargetTypeId":6,"workoutTargetTypeKey":"pace.zone"},"targetValueOne":2.5957161,"targetValueTwo":2.5545143,"targetValueUnit":null,"type":"ExecutableStepDTO","weightUnit":{"factor":453.59237,"unitId":9,"unitKey":"pound"},"weightValue":null,"workoutProvider":null,"zoneNumber":null}]}],"workoutSourceId":"null","workoutThumbnailUrl":null},"workoutScheduleId":1569279024})json";

    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(kScheduleJsonFromLog));
    ASSERT_TRUE(doc.isObject()) << "Schedule JSON fixture from log must be valid";

    const QJsonObject root = doc.object();
    ASSERT_TRUE(root["workout"].isObject()) << "Expected nested workout object in schedule payload";

    const QString xml = garminConnectGenerateWorkoutXml(root["workout"].toObject());

    EXPECT_EQ(xml.count("<row"), 3) << "Expected exactly 3 rows from 3 workout steps. XML was:\n"
                                     << xml.toStdString();
    EXPECT_FALSE(xml.contains("duration=\""))
        << "Distance-based workout should not emit row duration attributes. XML was:\n"
        << xml.toStdString();

    EXPECT_TRUE(xml.contains("distance=\"1.609344\""))
        << "Expected 1-mile steps converted to 1.609344 km. XML was:\n"
        << xml.toStdString();
    EXPECT_TRUE(xml.contains("distance=\"9.656064\""))
        << "Expected 6-mile interval converted to 9.656064 km. XML was:\n"
        << xml.toStdString();
}

void GarminConnectTestSuite::test_scheduleJson_realLogEasyRunPaceZoneSetsSpeedAndForceSpeed()
{
    static const char *kScheduleJsonFromLog = R"json({"associatedActivityDateTime":null,"associatedActivityId":null,"atpPlanTypeId":null,"calendarDate":"2026-02-25","consumer":null,"createdDate":"2026-02-24","itp":false,"nameChanged":false,"newName":null,"ownerId":116920806,"priority":null,"protected":false,"race":false,"tpType":null,"workout":{"atpPlanId":null,"author":null,"avgTrainingSpeed":2.8597210718525474,"consumer":null,"consumerImageURL":null,"consumerName":null,"consumerWebsiteURL":null,"createdDate":"2026-02-25T04:48:59.0","description":null,"descriptionI18nKey":null,"estimateType":"DISTANCE_ESTIMATED","estimatedDistanceInMeters":9656.064,"estimatedDistanceUnit":{"factor":null,"unitId":null,"unitKey":null},"estimatedDurationInSecs":3600,"isSessionTransitionEnabled":null,"locale":null,"ownerId":116920806,"poolLength":null,"poolLengthUnit":null,"shared":false,"sharedWithUsers":null,"sportType":{"displayOrder":1,"sportTypeId":1,"sportTypeKey":"running"},"subSportType":"GENERIC","trainingPlanId":null,"updatedDate":"2026-02-25T16:09:21.0","uploadTimestamp":null,"workoutId":1485541079,"workoutName":"Easy run","workoutNameI18nKey":null,"workoutProvider":"null","workoutSegments":[{"avgTrainingSpeed":null,"description":null,"estimateType":null,"estimatedDistanceInMeters":null,"estimatedDistanceUnit":null,"estimatedDurationInSecs":null,"poolLength":null,"poolLengthUnit":null,"segmentOrder":1,"sportType":{"displayOrder":1,"sportTypeId":1,"sportTypeKey":"running"},"workoutSteps":[{"category":null,"childStepId":null,"description":null,"endCondition":{"conditionTypeId":2,"conditionTypeKey":"time","displayOrder":2,"displayable":true},"endConditionCompare":null,"endConditionValue":3600,"endConditionZone":null,"equipmentType":{"displayOrder":0,"equipmentTypeId":0,"equipmentTypeKey":null},"exerciseName":null,"preferredEndConditionUnit":null,"providerExerciseSourceId":null,"secondaryTargetType":null,"secondaryTargetValueOne":null,"secondaryTargetValueTwo":null,"secondaryTargetValueUnit":null,"secondaryZoneNumber":null,"stepId":12558913161,"stepOrder":1,"stepType":{"displayOrder":3,"stepTypeId":3,"stepTypeKey":"interval"},"strokeType":{"displayOrder":0,"strokeTypeId":0,"strokeTypeKey":null},"targetType":{"displayOrder":6,"workoutTargetTypeId":6,"workoutTargetTypeKey":"pace.zone"},"targetValueOne":2.6867179,"targetValueTwo":2.68224,"targetValueUnit":null,"type":"ExecutableStepDTO","weightUnit":{"factor":453.59237,"unitId":9,"unitKey":"pound"},"weightValue":null,"workoutProvider":null,"zoneNumber":null}]}],"workoutSourceId":"null","workoutThumbnailUrl":null},"workoutScheduleId":1567468124})json";

    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(kScheduleJsonFromLog));
    ASSERT_TRUE(doc.isObject()) << "Schedule JSON fixture from log must be valid";

    const QJsonObject root = doc.object();
    ASSERT_TRUE(root["workout"].isObject()) << "Expected nested workout object in schedule payload";

    const QString xml = garminConnectGenerateWorkoutXml(root["workout"].toObject());

    EXPECT_EQ(xml.count("<row"), 1) << "Expected exactly 1 row from 1 workout step. XML was:\n"
                                     << xml.toStdString();
    EXPECT_TRUE(xml.contains("duration=\"01:00:00\""))
        << "Expected 3600s step duration serialized as 01:00:00. XML was:\n"
        << xml.toStdString();
    EXPECT_TRUE(xml.contains("forcespeed=\"1\""))
        << "Expected forcespeed enabled for pace.zone targets. XML was:\n"
        << xml.toStdString();
    EXPECT_TRUE(xml.contains("lower_speed=\"9.656\""))
        << "Expected lower pace bound converted from 2.68224 m/s to 9.656 km/h. XML was:\n"
        << xml.toStdString();
    EXPECT_TRUE(xml.contains("upper_speed=\"9.672\""))
        << "Expected upper pace bound converted from 2.6867179 m/s to 9.672 km/h. XML was:\n"
        << xml.toStdString();
    EXPECT_TRUE(xml.contains("speed=\"9.664\""))
        << "Expected average pace speed converted to 9.664 km/h. XML was:\n"
        << xml.toStdString();
}
