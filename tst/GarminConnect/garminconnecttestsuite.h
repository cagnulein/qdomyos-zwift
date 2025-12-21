#ifndef GARMINCONNECTTESTSUITE_H
#define GARMINCONNECTTESTSUITE_H

#include "gtest/gtest.h"

/**
 * @brief Test suite for GarminConnect OAuth1 URL encoding
 *
 * This suite tests the critical URL encoding behavior needed for OAuth1 authentication.
 * The problem: Qt's QUrl doesn't encode ':' and '/' in parameter values by default,
 * but OAuth1 and Garmin Connect require them to be percent-encoded.
 *
 * These tests verify the workaround: manually encoding with QUrl::toPercentEncoding()
 * and using QUrl::fromEncoded() with StrictMode to preserve the encoding.
 */
class GarminConnectTestSuite: public testing::Test {

public:
    GarminConnectTestSuite();

    /**
     * @brief Test that QUrl::toPercentEncoding() correctly encodes ':' and '/'
     *
     * Verifies that the Qt function encodes all special characters including
     * ':' and '/' which are normally considered "unreserved" by QUrl.
     */
    void test_toPercentEncoding_encodesColonAndSlash();

    /**
     * @brief Test that QUrl::fromEncoded() with StrictMode preserves encoding
     *
     * Verifies that when we build a URL string with correct encoding and parse it
     * with fromEncoded(StrictMode), Qt doesn't decode/re-encode it.
     */
    void test_fromEncodedStrictMode_preservesEncoding();

    /**
     * @brief Test that QUrlQuery.addQueryItem() FAILS to encode ':' and '/'
     *
     * This test documents the PROBLEM we're working around: Qt's natural
     * URL construction doesn't encode these characters in parameter values.
     */
    void test_QUrlQuery_doesNotEncodeColonSlash();

    /**
     * @brief Test the complete URL construction pattern used in GarminConnect
     *
     * Verifies the end-to-end pattern:
     * 1. Build query string with toPercentEncoding()
     * 2. Concatenate with base URL
     * 3. Parse with fromEncoded(StrictMode)
     * 4. Verify the URL is correctly encoded
     */
    void test_completePattern_correctEncoding();

    /**
     * @brief Test manual query parameter parsing for OAuth1 signature
     *
     * Verifies that we can correctly:
     * 1. Extract query string from URL
     * 2. Split by '&' and '='
     * 3. Decode values with fromPercentEncoding()
     * 4. Re-encode values for signature
     */
    void test_manualQueryParsing_decodesCorrectly();
};

// Register individual tests with Google Test
TEST_F(GarminConnectTestSuite, ToPercentEncodingEncodesColonAndSlash) {
    this->test_toPercentEncoding_encodesColonAndSlash();
}

TEST_F(GarminConnectTestSuite, FromEncodedStrictModePreservesEncoding) {
    this->test_fromEncodedStrictMode_preservesEncoding();
}

TEST_F(GarminConnectTestSuite, QUrlQueryDoesNotEncodeColonSlash) {
    this->test_QUrlQuery_doesNotEncodeColonSlash();
}

TEST_F(GarminConnectTestSuite, CompletePatternCorrectEncoding) {
    this->test_completePattern_correctEncoding();
}

TEST_F(GarminConnectTestSuite, ManualQueryParsingDecodesCorrectly) {
    this->test_manualQueryParsing_decodesCorrectly();
}

#endif // GARMINCONNECTTESTSUITE_H
