#pragma once

#include <gtest/gtest.h>

#include "devices/xcxbike/xcxbike.h"

class XcxBikeParserTest
    : public testing::TestWithParam<std::tuple<QByteArray, XcxTelemetry>> {};
