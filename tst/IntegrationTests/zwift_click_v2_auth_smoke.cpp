// Live, credential-backed smoke test for QZ's existing AuthToken class.
// This deliberately does not log credentials, bearer/refresh tokens or payloads.
// Run only from the narrowly scoped GitHub Actions workflow on a trusted branch.
#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QtGlobal>

#include "zwift-api/zwift_client_auth.h"

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const QString username = QString::fromUtf8(qgetenv("ZWIFT_USER"));
    const QString password = QString::fromUtf8(qgetenv("ZWIFT_PASSWORD"));
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        qCritical() << "ZWIFT_USER and ZWIFT_PASSWORD are required in GitHub Actions secrets.";
        return 2;
    }

    // The same AuthToken implementation used by Click V2, with the Game
    // Client grant; existing Zwift Mobile Link authentication is unaffected.
    AuthToken auth(username, password, &app, true);
    bool refreshRequested = false;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &app, [&app]() {
        qCritical() << "Zwift Game Client authentication timed out.";
        app.exit(1);
    });

    QObject::connect(&auth, &AuthToken::tokenReceived, &app,
                     [&app, &auth, &refreshRequested, &timeout](bool success, const QString &) {
        if (!success || !auth.haveValidAccessToken()) {
            qCritical() << "Zwift Game Client authentication failed (token redacted).";
            timeout.stop();
            app.exit(1);
            return;
        }
        if (!refreshRequested) {
            if (!auth.haveValidRefreshToken()) {
                qCritical() << "Zwift did not return a usable refresh token.";
                timeout.stop();
                app.exit(1);
                return;
            }
            refreshRequested = true;
            // Expire only the access token. The next real AuthToken request
            // must follow the refresh-token branch, not the password grant.
            auth.access_token.clear();
            auth.access_token_expiration = 0;
            QTimer::singleShot(0, &auth, &AuthToken::updateTokenData);
            return;
        }
        timeout.stop();
        qInfo() << "PASS: Zwift Game Client login and refresh grants (no tokens printed).";
        app.quit();
    });

    timeout.start(25000);
    auth.updateTokenData();
    return app.exec();
}
