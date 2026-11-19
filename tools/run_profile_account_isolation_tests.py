#!/usr/bin/env python3
"""Run the account regressions without building Bluetooth/UI dependencies.

Compile the production Garmin session methods and homeform::loadSettings verbatim
with real Qt QSettings, SimpleCrypt and the same Google Tests as the full suite.
The small homeform shell supplies only local-URL conversion and an absent UI
singleton. Garmin SSO/refresh calls fail immediately: these tests use fake tokens
and must never authenticate against a real service.
"""
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def method(source, signature):
    start = source.index(signature)
    # Production function closing braces are at column zero; nested ones are indented.
    end = source.index("\n}", start) + 2
    return source[start:end] + "\n"


def run():
    garmin = (ROOT / "src/garminconnect.cpp").read_text()
    home = (ROOT / "src/homeform.cpp").read_text()
    signatures = [
        "GarminConnect::GarminConnect(QObject *parent)",
        "GarminConnect::~GarminConnect()",
        "bool GarminConnect::isAuthenticated() const",
        "bool GarminConnect::matchesCurrentUser() const",
        "void GarminConnect::logout()",
        "void GarminConnect::loadTokensFromSettings()",
        "void GarminConnect::saveTokensToSettings()",
        "void GarminConnect::clearTokens()",
        "QString GarminConnect::currentUserIdFromSettings() const",
        "void GarminConnect::mirrorTokensToBaseSettings(QSettings &settings) const",
        "bool GarminConnect::tryRefreshToken()",
        "bool GarminConnect::uploadFitFile(const QString &fitFilePath)",
        "bool GarminConnect::uploadActivity(const QByteArray &fitData, const QString &fileName)",
        "bool GarminConnect::login(const QString &email, const QString &password, const QString &mfaCode)",
    ]
    with tempfile.TemporaryDirectory(prefix="qz-profile-tests-") as temp:
        build = Path(temp)
        (build / "homeform.h").write_text('''#pragma once
#include <QtCore>
#include "garminconnect.h"
#include "simplecrypt.h"
#include "qzsettings.h"
class QQmlFile {
public:
    static QString urlToLocalFileOrQrc(const QUrl &url) { return url.toLocalFile(); }
};
class homeform {
public:
    static homeform *singleton() { return nullptr; }
    static void loadSettings(const QUrl &url);
    static quint64 cryptoKeySettingsProfiles();
    void userProfileChanged() {}
    GarminConnect *garminConnect = nullptr;
    quint64 stravaProfileGeneration = 0;
};
''')
        production = '''#include <QtCore>
#include <QtNetwork>
#include "homeform.h"
#include "profiletokenstore.h"
'''
        production += "\n".join(method(garmin, signature) for signature in signatures)
        production += method(home, "quint64 homeform::cryptoKeySettingsProfiles()")
        production += method(home, "void homeform::loadSettings(const QUrl &filename)")
        production += '''
bool GarminConnect::fetchCookies() { qFatal("Unexpected Garmin SSO request"); return false; }
bool GarminConnect::fetchCsrfToken() { qFatal("Unexpected Garmin SSO request"); return false; }
bool GarminConnect::performLogin(const QString &, const QString &, bool) { qFatal("Unexpected Garmin login"); return false; }
void GarminConnect::performMfaVerification(const QString &) { qFatal("Unexpected Garmin MFA request"); }
bool GarminConnect::refreshOAuth2Token() { qFatal("Unexpected Garmin token refresh"); return false; }
'''
        (build / "production.cpp").write_text(production)
        (build / "main.cpp").write_text('''#include <gtest/gtest.h>
#include <QCoreApplication>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
''')
        subprocess.run(["moc", str(ROOT / "src/garminconnect.h"), "-o", str(build / "moc_garmin.cpp")], check=True)
        flags = shlex.split(subprocess.check_output(
            ["pkg-config", "--cflags", "--libs", "Qt5Core", "Qt5Network"], text=True))
        command = ["g++", "-std=c++14", "-fPIC", "-pthread", "-I" + str(build),
                   "-I" + str(ROOT / "src"), "-I" + str(ROOT / "tst/ToolTests"),
                   str(build / "main.cpp"), str(build / "production.cpp"), str(build / "moc_garmin.cpp"),
                   str(ROOT / "src/qzsettings.cpp"), str(ROOT / "src/simplecrypt.cpp"),
                   str(ROOT / "tst/ToolTests/profiletokenstoresuite.cpp"),
                   str(ROOT / "tst/ToolTests/profileaccountisolationsuite.cpp"),
                   "-o", str(build / "profile-tests"), "-lgtest"] + flags
        subprocess.run(command, check=True)
        subprocess.run([str(build / "profile-tests")], check=True)


if __name__ == "__main__":
    run()
