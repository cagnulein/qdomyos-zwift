#if __has_include("secret.h")
#include "secret.h"
#else
#if defined(WIN32)
#pragma message("PELOTON API WILL NOT WORK!!!")
#else
#warning "PELOTON API WILL NOT WORK!!!"
#endif
#endif
#include "homeform.h"
#include "peloton.h"
#include <chrono>
#include <QTimer>

using namespace std::chrono_literals;

const bool log_request = true;

#define RAWHEADER request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift/") + QCoreApplication::applicationVersion());request.setRawHeader(QByteArray("Authorization"), QByteArray("Bearer ") + (tempAccessToken.isEmpty() ?  getPelotonTokenForUser(QZSettings::peloton_accesstoken, userId, QZSettings::default_peloton_accesstoken).toString().toUtf8() : tempAccessToken.toUtf8()));

peloton::peloton(bluetooth *bl, QObject *parent) : QObject(parent) {

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    timer = new QTimer(this);

    //peloton_connect_clicked();
    peloton_refreshtoken();

    // only for test purpose
    /*
    current_image_downloaded =
        new fileDownloader(QUrl("https://s3.amazonaws.com/peloton-ride-images/fa50b87ea5c44ce078e28a3030b8865b5dbffb35/"
                                "img_1646099287_a620f71b3d6740718457b21769a7ed46.png"));
    */

    rower_pace_offset = 1;

    rower_pace[0].value = -1;
    rower_pace[0].display_name = QStringLiteral("Recovery");
    rower_pace[0].levels[0].fast_pace = 4.31;
    rower_pace[0].levels[0].slow_pace = 15;
    rower_pace[0].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[0].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[0].levels[1].fast_pace = 3.58;
    rower_pace[0].levels[1].slow_pace = 15;
    rower_pace[0].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[0].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[0].levels[2].fast_pace = 3.34;
    rower_pace[0].levels[2].slow_pace = 15;
    rower_pace[0].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[0].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[0].levels[3].fast_pace = 3.17;
    rower_pace[0].levels[3].slow_pace = 15;
    rower_pace[0].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[0].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[0].levels[4].fast_pace = 3.03;
    rower_pace[0].levels[4].slow_pace = 15;
    rower_pace[0].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[0].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[0].levels[5].fast_pace = 2.5;
    rower_pace[0].levels[5].slow_pace = 15;
    rower_pace[0].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[0].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[0].levels[6].fast_pace = 2.38;
    rower_pace[0].levels[6].slow_pace = 15;
    rower_pace[0].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[0].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[0].levels[7].fast_pace = 2.28;
    rower_pace[0].levels[7].slow_pace = 15;
    rower_pace[0].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[0].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[0].levels[8].fast_pace = 2.17;
    rower_pace[0].levels[8].slow_pace = 15;
    rower_pace[0].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[0].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[0].levels[9].fast_pace = 2.07;
    rower_pace[0].levels[9].slow_pace = 15;
    rower_pace[0].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[0].levels[9].slug = QStringLiteral("level_10");

    rower_pace[1].value = 0;
    rower_pace[1].display_name = QStringLiteral("Easy");
    rower_pace[1].levels[0].fast_pace = 3.51;
    rower_pace[1].levels[0].slow_pace = 4.31;
    rower_pace[1].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[1].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[1].levels[1].fast_pace = 3.22;
    rower_pace[1].levels[1].slow_pace = 3.58;
    rower_pace[1].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[1].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[1].levels[2].fast_pace = 3.02;
    rower_pace[1].levels[2].slow_pace = 3.34;
    rower_pace[1].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[1].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[1].levels[3].fast_pace = 2.47;
    rower_pace[1].levels[3].slow_pace = 3.17;
    rower_pace[1].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[1].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[1].levels[4].fast_pace = 2.36;
    rower_pace[1].levels[4].slow_pace = 3.03;
    rower_pace[1].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[1].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[1].levels[5].fast_pace = 2.24;
    rower_pace[1].levels[5].slow_pace = 2.5;
    rower_pace[1].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[1].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[1].levels[6].fast_pace = 2.14;
    rower_pace[1].levels[6].slow_pace = 2.38;
    rower_pace[1].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[1].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[1].levels[7].fast_pace = 2.06;
    rower_pace[1].levels[7].slow_pace = 2.28;
    rower_pace[1].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[1].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[1].levels[8].fast_pace = 1.56;
    rower_pace[1].levels[8].slow_pace = 2.17;
    rower_pace[1].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[1].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[1].levels[9].fast_pace = 1.48;
    rower_pace[1].levels[9].slow_pace = 2.07;
    rower_pace[1].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[1].levels[9].slug = QStringLiteral("level_10");

    rower_pace[2].value = 1;
    rower_pace[2].display_name = QStringLiteral("Moderate");
    rower_pace[2].levels[0].fast_pace = 3.35;
    rower_pace[2].levels[0].slow_pace = 3.51;
    rower_pace[2].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[2].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[2].levels[1].fast_pace = 3.09;
    rower_pace[2].levels[1].slow_pace = 3.22;
    rower_pace[2].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[2].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[2].levels[2].fast_pace = 2.5;
    rower_pace[2].levels[2].slow_pace = 3.02;
    rower_pace[2].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[2].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[2].levels[3].fast_pace = 2.36;
    rower_pace[2].levels[3].slow_pace = 2.47;
    rower_pace[2].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[2].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[2].levels[4].fast_pace = 2.25;
    rower_pace[2].levels[4].slow_pace = 2.36;
    rower_pace[2].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[2].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[2].levels[5].fast_pace = 2.15;
    rower_pace[2].levels[5].slow_pace = 2.24;
    rower_pace[2].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[2].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[2].levels[6].fast_pace = 2.05;
    rower_pace[2].levels[6].slow_pace = 2.14;
    rower_pace[2].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[2].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[2].levels[7].fast_pace = 1.57;
    rower_pace[2].levels[7].slow_pace = 2.06;
    rower_pace[2].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[2].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[2].levels[8].fast_pace = 1.49;
    rower_pace[2].levels[8].slow_pace = 1.57;
    rower_pace[2].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[2].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[2].levels[9].fast_pace = 1.41;
    rower_pace[2].levels[9].slow_pace = 1.48;
    rower_pace[2].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[2].levels[9].slug = QStringLiteral("level_10");

    rower_pace[3].value = 2;
    rower_pace[3].display_name = QStringLiteral("Challenging");
    rower_pace[3].levels[0].fast_pace = 3.17;
    rower_pace[3].levels[0].slow_pace = 3.35;
    rower_pace[3].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[3].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[3].levels[1].fast_pace = 2.52;
    rower_pace[3].levels[1].slow_pace = 3.09;
    rower_pace[3].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[3].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[3].levels[2].fast_pace = 2.35;
    rower_pace[3].levels[2].slow_pace = 2.5;
    rower_pace[3].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[3].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[3].levels[3].fast_pace = 2.23;
    rower_pace[3].levels[3].slow_pace = 2.36;
    rower_pace[3].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[3].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[3].levels[4].fast_pace = 2.13;
    rower_pace[3].levels[4].slow_pace = 2.25;
    rower_pace[3].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[3].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[3].levels[5].fast_pace = 2.03;
    rower_pace[3].levels[5].slow_pace = 2.15;
    rower_pace[3].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[3].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[3].levels[6].fast_pace = 1.54;
    rower_pace[3].levels[6].slow_pace = 2.05;
    rower_pace[3].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[3].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[3].levels[7].fast_pace = 1.47;
    rower_pace[3].levels[7].slow_pace = 1.57;
    rower_pace[3].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[3].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[3].levels[8].fast_pace = 1.4;
    rower_pace[3].levels[8].slow_pace = 1.49;
    rower_pace[3].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[3].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[3].levels[9].fast_pace = 1.32;
    rower_pace[3].levels[9].slow_pace = 1.41;
    rower_pace[3].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[3].levels[9].slug = QStringLiteral("level_10");

    rower_pace[4].value = 3;
    rower_pace[4].display_name = QStringLiteral("Max");
    rower_pace[4].levels[0].fast_pace = 3.06;
    rower_pace[4].levels[0].slow_pace = 3.17;
    rower_pace[4].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[4].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[4].levels[1].fast_pace = 2.42;
    rower_pace[4].levels[1].slow_pace = 2.52;
    rower_pace[4].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[4].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[4].levels[2].fast_pace = 2.26;
    rower_pace[4].levels[2].slow_pace = 2.35;
    rower_pace[4].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[4].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[4].levels[3].fast_pace = 2.15;
    rower_pace[4].levels[3].slow_pace = 2.23;
    rower_pace[4].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[4].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[4].levels[4].fast_pace = 2.05;
    rower_pace[4].levels[4].slow_pace = 2.13;
    rower_pace[4].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[4].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[4].levels[5].fast_pace = 1.56;
    rower_pace[4].levels[5].slow_pace = 2.03;
    rower_pace[4].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[4].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[4].levels[6].fast_pace = 1.48;
    rower_pace[4].levels[6].slow_pace = 1.54;
    rower_pace[4].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[4].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[4].levels[7].fast_pace = 1.41;
    rower_pace[4].levels[7].slow_pace = 1.47;
    rower_pace[4].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[4].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[4].levels[8].fast_pace = 1.34;
    rower_pace[4].levels[8].slow_pace = 1.4;
    rower_pace[4].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[4].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[4].levels[9].fast_pace = 1.27;
    rower_pace[4].levels[9].slow_pace = 1.32;
    rower_pace[4].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[4].levels[9].slug = QStringLiteral("level_10");

    treadmill_pace[0].value = 0;
    treadmill_pace[0].display_name = QStringLiteral("Recovery");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[0].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[0].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[0].levels[0].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[0].fast_pace = 4.82802;    // 3.0 mph
    treadmill_pace[0].levels[0].speed = (treadmill_pace[0].levels[0].slow_pace + treadmill_pace[0].levels[0].fast_pace) / 2.0;

    treadmill_pace[0].levels[1].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[1].fast_pace = 5.14989;    // 3.2 mph
    treadmill_pace[0].levels[1].speed = (treadmill_pace[0].levels[1].slow_pace + treadmill_pace[0].levels[1].fast_pace) / 2.0;

    treadmill_pace[0].levels[2].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[2].fast_pace = 5.63269;    // 3.5 mph
    treadmill_pace[0].levels[2].speed = (treadmill_pace[0].levels[2].slow_pace + treadmill_pace[0].levels[2].fast_pace) / 2.0;

    treadmill_pace[0].levels[3].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[3].fast_pace = 5.95456;    // 3.7 mph
    treadmill_pace[0].levels[3].speed = (treadmill_pace[0].levels[3].slow_pace + treadmill_pace[0].levels[3].fast_pace) / 2.0;

    treadmill_pace[0].levels[4].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[4].fast_pace = 6.59829;    // 4.1 mph
    treadmill_pace[0].levels[4].speed = (treadmill_pace[0].levels[4].slow_pace + treadmill_pace[0].levels[4].fast_pace) / 2.0;

    treadmill_pace[0].levels[5].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[5].fast_pace = 7.24203;    // 4.5 mph
    treadmill_pace[0].levels[5].speed = (treadmill_pace[0].levels[5].slow_pace + treadmill_pace[0].levels[5].fast_pace) / 2.0;

    treadmill_pace[0].levels[6].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[6].fast_pace = 8.0467;     // 5.0 mph
    treadmill_pace[0].levels[6].speed = (treadmill_pace[0].levels[6].slow_pace + treadmill_pace[0].levels[6].fast_pace) / 2.0;

    treadmill_pace[0].levels[7].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[7].fast_pace = 9.17324;    // 5.7 mph
    treadmill_pace[0].levels[7].speed = (treadmill_pace[0].levels[7].slow_pace + treadmill_pace[0].levels[7].fast_pace) / 2.0;

    treadmill_pace[0].levels[8].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[8].fast_pace = 10.46071;   // 6.5 mph
    treadmill_pace[0].levels[8].speed = (treadmill_pace[0].levels[8].slow_pace + treadmill_pace[0].levels[8].fast_pace) / 2.0;

    treadmill_pace[0].levels[9].slow_pace = 1.60934;    // 1.0 mph
    treadmill_pace[0].levels[9].fast_pace = 12.23098;   // 7.6 mph
    treadmill_pace[0].levels[9].speed = (treadmill_pace[0].levels[9].slow_pace + treadmill_pace[0].levels[9].fast_pace) / 2.0;

    // Easy
    treadmill_pace[1].value = 1;
    treadmill_pace[1].display_name = QStringLiteral("Easy");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[1].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[1].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[1].levels[0].slow_pace = 4.98895;    // 3.1 mph
    treadmill_pace[1].levels[0].fast_pace = 5.30802;    // 3.3 mph
    treadmill_pace[1].levels[0].speed = (treadmill_pace[1].levels[0].slow_pace + treadmill_pace[1].levels[0].fast_pace) / 2.0;

    treadmill_pace[1].levels[1].slow_pace = 5.30802;    // 3.3 mph
    treadmill_pace[1].levels[1].fast_pace = 5.79362;    // 3.6 mph
    treadmill_pace[1].levels[1].speed = (treadmill_pace[1].levels[1].slow_pace + treadmill_pace[1].levels[1].fast_pace) / 2.0;

    treadmill_pace[1].levels[2].slow_pace = 5.79362;    // 3.6 mph
    treadmill_pace[1].levels[2].fast_pace = 6.27642;    // 3.9 mph
    treadmill_pace[1].levels[2].speed = (treadmill_pace[1].levels[2].slow_pace + treadmill_pace[1].levels[2].fast_pace) / 2.0;

    treadmill_pace[1].levels[3].slow_pace = 6.11549;    // 3.8 mph
    treadmill_pace[1].levels[3].fast_pace = 6.59829;    // 4.1 mph
    treadmill_pace[1].levels[3].speed = (treadmill_pace[1].levels[3].slow_pace + treadmill_pace[1].levels[3].fast_pace) / 2.0;

    treadmill_pace[1].levels[4].slow_pace = 6.75923;    // 4.2 mph
    treadmill_pace[1].levels[4].fast_pace = 7.24203;    // 4.5 mph
    treadmill_pace[1].levels[4].speed = (treadmill_pace[1].levels[4].slow_pace + treadmill_pace[1].levels[4].fast_pace) / 2.0;

    treadmill_pace[1].levels[5].slow_pace = 7.40296;    // 4.6 mph
    treadmill_pace[1].levels[5].fast_pace = 7.88576;    // 4.9 mph
    treadmill_pace[1].levels[5].speed = (treadmill_pace[1].levels[5].slow_pace + treadmill_pace[1].levels[5].fast_pace) / 2.0;

    treadmill_pace[1].levels[6].slow_pace = 8.20763;    // 5.1 mph
    treadmill_pace[1].levels[6].fast_pace = 8.85137;    // 5.5 mph
    treadmill_pace[1].levels[6].speed = (treadmill_pace[1].levels[6].slow_pace + treadmill_pace[1].levels[6].fast_pace) / 2.0;

    treadmill_pace[1].levels[7].slow_pace = 9.33417;    // 5.8 mph
    treadmill_pace[1].levels[7].fast_pace = 9.97791;    // 6.2 mph
    treadmill_pace[1].levels[7].speed = (treadmill_pace[1].levels[7].slow_pace + treadmill_pace[1].levels[7].fast_pace) / 2.0;

    treadmill_pace[1].levels[8].slow_pace = 10.61884;   // 6.6 mph
    treadmill_pace[1].levels[8].fast_pace = 11.58725;   // 7.2 mph
    treadmill_pace[1].levels[8].speed = (treadmill_pace[1].levels[8].slow_pace + treadmill_pace[1].levels[8].fast_pace) / 2.0;

    treadmill_pace[1].levels[9].slow_pace = 12.39192;   // 7.7 mph
    treadmill_pace[1].levels[9].fast_pace = 13.51846;   // 8.4 mph
    treadmill_pace[1].levels[9].speed = (treadmill_pace[1].levels[9].slow_pace + treadmill_pace[1].levels[9].fast_pace) / 2.0;

    // Moderate
    treadmill_pace[2].value = 2;
    treadmill_pace[2].display_name = QStringLiteral("Moderate");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[2].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[2].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[2].levels[0].slow_pace = 5.47176;    // 3.4 mph
    treadmill_pace[2].levels[0].fast_pace = 5.79362;    // 3.6 mph
    treadmill_pace[2].levels[0].speed = (treadmill_pace[2].levels[0].slow_pace + treadmill_pace[2].levels[0].fast_pace) / 2.0;

    treadmill_pace[2].levels[1].slow_pace = 5.95456;    // 3.7 mph
    treadmill_pace[2].levels[1].fast_pace = 6.27642;    // 3.9 mph
    treadmill_pace[2].levels[1].speed = (treadmill_pace[2].levels[1].slow_pace + treadmill_pace[2].levels[1].fast_pace) / 2.0;

    treadmill_pace[2].levels[2].slow_pace = 6.43736;    // 4.0 mph
    treadmill_pace[2].levels[2].fast_pace = 6.75923;    // 4.2 mph
    treadmill_pace[2].levels[2].speed = (treadmill_pace[2].levels[2].slow_pace + treadmill_pace[2].levels[2].fast_pace) / 2.0;

    treadmill_pace[2].levels[3].slow_pace = 6.75923;    // 4.2 mph
    treadmill_pace[2].levels[3].fast_pace = 7.24203;    // 4.5 mph
    treadmill_pace[2].levels[3].speed = (treadmill_pace[2].levels[3].slow_pace + treadmill_pace[2].levels[3].fast_pace) / 2.0;

    treadmill_pace[2].levels[4].slow_pace = 7.40296;    // 4.6 mph
    treadmill_pace[2].levels[4].fast_pace = 7.88576;    // 4.9 mph
    treadmill_pace[2].levels[4].speed = (treadmill_pace[2].levels[4].slow_pace + treadmill_pace[2].levels[4].fast_pace) / 2.0;

    treadmill_pace[2].levels[5].slow_pace = 8.0467;     // 5.0 mph
    treadmill_pace[2].levels[5].fast_pace = 8.69043;    // 5.4 mph
    treadmill_pace[2].levels[5].speed = (treadmill_pace[2].levels[5].slow_pace + treadmill_pace[2].levels[5].fast_pace) / 2.0;

    treadmill_pace[2].levels[6].slow_pace = 9.01230;    // 5.6 mph
    treadmill_pace[2].levels[6].fast_pace = 9.65604;    // 6.0 mph
    treadmill_pace[2].levels[6].speed = (treadmill_pace[2].levels[6].slow_pace + treadmill_pace[2].levels[6].fast_pace) / 2.0;

    treadmill_pace[2].levels[7].slow_pace = 10.13884;   // 6.3 mph
    treadmill_pace[2].levels[7].fast_pace = 10.94351;   // 6.8 mph
    treadmill_pace[2].levels[7].speed = (treadmill_pace[2].levels[7].slow_pace + treadmill_pace[2].levels[7].fast_pace) / 2.0;

    treadmill_pace[2].levels[8].slow_pace = 11.74818;   // 7.3 mph
    treadmill_pace[2].levels[8].fast_pace = 12.55285;   // 7.8 mph
    treadmill_pace[2].levels[8].speed = (treadmill_pace[2].levels[8].slow_pace + treadmill_pace[2].levels[8].fast_pace) / 2.0;

    treadmill_pace[2].levels[9].slow_pace = 13.67939;   // 8.5 mph
    treadmill_pace[2].levels[9].fast_pace = 14.48406;   // 9.0 mph
    treadmill_pace[2].levels[9].speed = (treadmill_pace[2].levels[9].slow_pace + treadmill_pace[2].levels[9].fast_pace) / 2.0;

    // Challenging
    treadmill_pace[3].value = 3;
    treadmill_pace[3].display_name = QStringLiteral("Challenging");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[3].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[3].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[3].levels[0].slow_pace = 5.95456;    // 3.7 mph
    treadmill_pace[3].levels[0].fast_pace = 6.43736;    // 4.0 mph
    treadmill_pace[3].levels[0].speed = (treadmill_pace[3].levels[0].slow_pace + treadmill_pace[3].levels[0].fast_pace) / 2.0;

    treadmill_pace[3].levels[1].slow_pace = 6.43736;    // 4.0 mph
    treadmill_pace[3].levels[1].fast_pace = 6.92016;    // 4.3 mph
    treadmill_pace[3].levels[1].speed = (treadmill_pace[3].levels[1].slow_pace + treadmill_pace[3].levels[1].fast_pace) / 2.0;

    treadmill_pace[3].levels[2].slow_pace = 6.92016;    // 4.3 mph
    treadmill_pace[3].levels[2].fast_pace = 7.40296;    // 4.6 mph
    treadmill_pace[3].levels[2].speed = (treadmill_pace[3].levels[2].slow_pace + treadmill_pace[3].levels[2].fast_pace) / 2.0;

    treadmill_pace[3].levels[3].slow_pace = 7.40296;    // 4.6 mph
    treadmill_pace[3].levels[3].fast_pace = 8.0467;     // 5.0 mph
    treadmill_pace[3].levels[3].speed = (treadmill_pace[3].levels[3].slow_pace + treadmill_pace[3].levels[3].fast_pace) / 2.0;

    treadmill_pace[3].levels[4].slow_pace = 8.0467;     // 5.0 mph
    treadmill_pace[3].levels[4].fast_pace = 8.69043;    // 5.4 mph
    treadmill_pace[3].levels[4].speed = (treadmill_pace[3].levels[4].slow_pace + treadmill_pace[3].levels[4].fast_pace) / 2.0;

    treadmill_pace[3].levels[5].slow_pace = 8.85137;    // 5.5 mph
    treadmill_pace[3].levels[5].fast_pace = 9.65604;    // 6.0 mph
    treadmill_pace[3].levels[5].speed = (treadmill_pace[3].levels[5].slow_pace + treadmill_pace[3].levels[5].fast_pace) / 2.0;

    treadmill_pace[3].levels[6].slow_pace = 9.81697;    // 6.1 mph
    treadmill_pace[3].levels[6].fast_pace = 10.78258;   // 6.7 mph
    treadmill_pace[3].levels[6].speed = (treadmill_pace[3].levels[6].slow_pace + treadmill_pace[3].levels[6].fast_pace) / 2.0;

    treadmill_pace[3].levels[7].slow_pace = 11.10445;   // 6.9 mph
    treadmill_pace[3].levels[7].fast_pace = 12.07005;   // 7.5 mph
    treadmill_pace[3].levels[7].speed = (treadmill_pace[3].levels[7].slow_pace + treadmill_pace[3].levels[7].fast_pace) / 2.0;

    treadmill_pace[3].levels[8].slow_pace = 12.71379;   // 7.9 mph
    treadmill_pace[3].levels[8].fast_pace = 13.84032;   // 8.6 mph
    treadmill_pace[3].levels[8].speed = (treadmill_pace[3].levels[8].slow_pace + treadmill_pace[3].levels[8].fast_pace) / 2.0;

    treadmill_pace[3].levels[9].slow_pace = 14.64499;   // 9.1 mph
    treadmill_pace[3].levels[9].fast_pace = 16.0934;    // 10.0 mph
    treadmill_pace[3].levels[9].speed = (treadmill_pace[3].levels[9].slow_pace + treadmill_pace[3].levels[9].fast_pace) / 2.0;

    // Hard
    treadmill_pace[4].value = 4;
    treadmill_pace[4].display_name = QStringLiteral("Hard");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[4].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[4].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[4].levels[0].slow_pace = 6.59829;    // 4.1 mph
    treadmill_pace[4].levels[0].fast_pace = 7.08110;    // 4.4 mph
    treadmill_pace[4].levels[0].speed = (treadmill_pace[4].levels[0].slow_pace + treadmill_pace[4].levels[0].fast_pace) / 2.0;

    treadmill_pace[4].levels[1].slow_pace = 7.08110;    // 4.4 mph
    treadmill_pace[4].levels[1].fast_pace = 7.56390;    // 4.7 mph
    treadmill_pace[4].levels[1].speed = (treadmill_pace[4].levels[1].slow_pace + treadmill_pace[4].levels[1].fast_pace) / 2.0;

    treadmill_pace[4].levels[2].slow_pace = 7.56390;    // 4.7 mph
    treadmill_pace[4].levels[2].fast_pace = 8.20763;    // 5.1 mph
    treadmill_pace[4].levels[2].speed = (treadmill_pace[4].levels[2].slow_pace + treadmill_pace[4].levels[2].fast_pace) / 2.0;

    treadmill_pace[4].levels[3].slow_pace = 8.20763;    // 5.1 mph
    treadmill_pace[4].levels[3].fast_pace = 8.69043;    // 5.4 mph
    treadmill_pace[4].levels[3].speed = (treadmill_pace[4].levels[3].slow_pace + treadmill_pace[4].levels[3].fast_pace) / 2.0;

    treadmill_pace[4].levels[4].slow_pace = 8.85137;    // 5.5 mph
    treadmill_pace[4].levels[4].fast_pace = 9.65604;    // 6.0 mph
    treadmill_pace[4].levels[4].speed = (treadmill_pace[4].levels[4].slow_pace + treadmill_pace[4].levels[4].fast_pace) / 2.0;

    treadmill_pace[4].levels[5].slow_pace = 9.81697;    // 6.1 mph
    treadmill_pace[4].levels[5].fast_pace = 10.62164;   // 6.6 mph
    treadmill_pace[4].levels[5].speed = (treadmill_pace[4].levels[5].slow_pace + treadmill_pace[4].levels[5].fast_pace) / 2.0;

    treadmill_pace[4].levels[6].slow_pace = 10.94351;   // 6.8 mph
    treadmill_pace[4].levels[6].fast_pace = 11.74818;   // 7.3 mph
    treadmill_pace[4].levels[6].speed = (treadmill_pace[4].levels[6].slow_pace + treadmill_pace[4].levels[6].fast_pace) / 2.0;

    treadmill_pace[4].levels[7].slow_pace = 12.23098;   // 7.6 mph
    treadmill_pace[4].levels[7].fast_pace = 13.19659;   // 8.2 mph
    treadmill_pace[4].levels[7].speed = (treadmill_pace[4].levels[7].slow_pace + treadmill_pace[4].levels[7].fast_pace) / 2.0;

    treadmill_pace[4].levels[8].slow_pace = 14.00126;   // 8.7 mph
    treadmill_pace[4].levels[8].fast_pace = 15.12780;   // 9.4 mph
    treadmill_pace[4].levels[8].speed = (treadmill_pace[4].levels[8].slow_pace + treadmill_pace[4].levels[8].fast_pace) / 2.0;

    treadmill_pace[4].levels[9].slow_pace = 16.25433;   // 10.1 mph
    treadmill_pace[4].levels[9].fast_pace = 17.54180;   // 10.9 mph
    treadmill_pace[4].levels[9].speed = (treadmill_pace[4].levels[9].slow_pace + treadmill_pace[4].levels[9].fast_pace) / 2.0;

    // Very Hard
    treadmill_pace[5].value = 5;
    treadmill_pace[5].display_name = QStringLiteral("Very Hard");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[5].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[5].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[5].levels[0].slow_pace = 7.24203;    // 4.5 mph
    treadmill_pace[5].levels[0].fast_pace = 7.88576;    // 4.9 mph
    treadmill_pace[5].levels[0].speed = (treadmill_pace[5].levels[0].slow_pace + treadmill_pace[5].levels[0].fast_pace) / 2.0;

    treadmill_pace[5].levels[1].slow_pace = 7.72483;    // 4.8 mph
    treadmill_pace[5].levels[1].fast_pace = 8.36857;    // 5.2 mph
    treadmill_pace[5].levels[1].speed = (treadmill_pace[5].levels[1].slow_pace + treadmill_pace[5].levels[1].fast_pace) / 2.0;

    treadmill_pace[5].levels[2].slow_pace = 8.36857;    // 5.2 mph
    treadmill_pace[5].levels[2].fast_pace = 9.01230;    // 5.6 mph
    treadmill_pace[5].levels[2].speed = (treadmill_pace[5].levels[2].slow_pace + treadmill_pace[5].levels[2].fast_pace) / 2.0;

    treadmill_pace[5].levels[3].slow_pace = 8.85137;    // 5.5 mph
    treadmill_pace[5].levels[3].fast_pace = 9.81697;    // 6.1 mph
    treadmill_pace[5].levels[3].speed = (treadmill_pace[5].levels[3].slow_pace + treadmill_pace[5].levels[3].fast_pace) / 2.0;

    treadmill_pace[5].levels[4].slow_pace = 9.81697;    // 6.1 mph
    treadmill_pace[5].levels[4].fast_pace = 10.62164;   // 6.6 mph
    treadmill_pace[5].levels[4].speed = (treadmill_pace[5].levels[4].slow_pace + treadmill_pace[5].levels[4].fast_pace) / 2.0;

    treadmill_pace[5].levels[5].slow_pace = 10.78258;   // 6.7 mph
    treadmill_pace[5].levels[5].fast_pace = 11.74818;   // 7.3 mph
    treadmill_pace[5].levels[5].speed = (treadmill_pace[5].levels[5].slow_pace + treadmill_pace[5].levels[5].fast_pace) / 2.0;

    treadmill_pace[5].levels[6].slow_pace = 11.90912;   // 7.4 mph
    treadmill_pace[5].levels[6].fast_pace = 13.03565;   // 8.1 mph
    treadmill_pace[5].levels[6].speed = (treadmill_pace[5].levels[6].slow_pace + treadmill_pace[5].levels[6].fast_pace) / 2.0;

    treadmill_pace[5].levels[7].slow_pace = 13.35752;   // 8.3 mph
    treadmill_pace[5].levels[7].fast_pace = 14.64499;   // 9.1 mph
    treadmill_pace[5].levels[7].speed = (treadmill_pace[5].levels[7].slow_pace + treadmill_pace[5].levels[7].fast_pace) / 2.0;

    treadmill_pace[5].levels[8].slow_pace = 15.28873;   // 9.5 mph
    treadmill_pace[5].levels[8].fast_pace = 16.73714;   // 10.4 mph
    treadmill_pace[5].levels[8].speed = (treadmill_pace[5].levels[8].slow_pace + treadmill_pace[5].levels[8].fast_pace) / 2.0;

    treadmill_pace[5].levels[9].slow_pace = 17.70274;   // 11.0 mph
    treadmill_pace[5].levels[9].fast_pace = 19.63395;   // 12.2 mph
    treadmill_pace[5].levels[9].speed = (treadmill_pace[5].levels[9].slow_pace + treadmill_pace[5].levels[9].fast_pace) / 2.0;

    // Max
    treadmill_pace[6].value = 6;
    treadmill_pace[6].display_name = QStringLiteral("Max");
    for (int i = 0; i < 10; i++) {
        treadmill_pace[6].levels[i].display_name = QStringLiteral("Level %1").arg(i+1);
        treadmill_pace[6].levels[i].slug = QStringLiteral("level_%1").arg(i+1);
    }

    treadmill_pace[6].levels[0].slow_pace = 8.0467;     // 5.0 mph
    treadmill_pace[6].levels[0].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[0].speed = (treadmill_pace[6].levels[0].slow_pace + treadmill_pace[6].levels[0].fast_pace) / 2.0;

    treadmill_pace[6].levels[1].slow_pace = 8.52950;    // 5.3 mph
    treadmill_pace[6].levels[1].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[1].speed = (treadmill_pace[6].levels[1].slow_pace + treadmill_pace[6].levels[1].fast_pace) / 2.0;

    treadmill_pace[6].levels[2].slow_pace = 9.17324;    // 5.7 mph
    treadmill_pace[6].levels[2].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[2].speed = (treadmill_pace[6].levels[2].slow_pace + treadmill_pace[6].levels[2].fast_pace) / 2.0;

    treadmill_pace[6].levels[3].slow_pace = 9.97791;    // 6.2 mph
    treadmill_pace[6].levels[3].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[3].speed = (treadmill_pace[6].levels[3].slow_pace + treadmill_pace[6].levels[3].fast_pace) / 2.0;

    treadmill_pace[6].levels[4].slow_pace = 10.78258;   // 6.7 mph
    treadmill_pace[6].levels[4].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[4].speed = (treadmill_pace[6].levels[4].slow_pace + treadmill_pace[6].levels[4].fast_pace) / 2.0;

    treadmill_pace[6].levels[5].slow_pace = 11.90912;   // 7.4 mph
    treadmill_pace[6].levels[5].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[5].speed = (treadmill_pace[6].levels[5].slow_pace + treadmill_pace[6].levels[5].fast_pace) / 2.0;

    treadmill_pace[6].levels[6].slow_pace = 13.19659;   // 8.2 mph
    treadmill_pace[6].levels[6].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[6].speed = (treadmill_pace[6].levels[6].slow_pace + treadmill_pace[6].levels[6].fast_pace) / 2.0;

    treadmill_pace[6].levels[7].slow_pace = 14.80593;   // 9.2 mph
    treadmill_pace[6].levels[7].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[7].speed = (treadmill_pace[6].levels[7].slow_pace + treadmill_pace[6].levels[7].fast_pace) / 2.0;

    treadmill_pace[6].levels[8].slow_pace = 16.89993;   // 10.5 mph
    treadmill_pace[6].levels[8].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[8].speed = (treadmill_pace[6].levels[8].slow_pace + treadmill_pace[6].levels[8].fast_pace) / 2.0;

    treadmill_pace[6].levels[9].slow_pace = 19.79488;   // 12.3 mph
    treadmill_pace[6].levels[9].fast_pace = 20.11675;   // 12.5 mph
    treadmill_pace[6].levels[9].speed = (treadmill_pace[6].levels[9].slow_pace + treadmill_pace[6].levels[9].fast_pace) / 2.0;

    connect(timer, &QTimer::timeout, this, &peloton::startEngine);

    PZP = new powerzonepack(bl, this);
    HFB = new homefitnessbuddy(bl, this);

    connect(PZP, &powerzonepack::workoutStarted, this, &peloton::pzp_trainrows);
    connect(PZP, &powerzonepack::loginState, this, &peloton::pzp_loginState);
    connect(HFB, &homefitnessbuddy::workoutStarted, this, &peloton::hfb_trainrows);

    QString userId = settings.value(QZSettings::peloton_current_user_id, QZSettings::default_peloton_current_user_id).toString();
    qDebug() << "userId" << userId;
    
    if (!getPelotonTokenForUser(QZSettings::peloton_accesstoken, userId, QZSettings::default_peloton_accesstoken).toString().length()) {
        qDebug() << QStringLiteral("invalid peloton credentials");
        return;
    }

    startEngine();
}

void peloton::pzp_loginState(bool ok) { emit pzpLoginState(ok); }

void peloton::hfb_trainrows(QList<trainrow> *list) {
    trainrows.clear();
    for (const trainrow r : qAsConst(*list)) {

        trainrows.append(r);
    }
    if (trainrows.isEmpty()) {
        current_api = no_metrics;
    }

    emit workoutStarted(current_workout_name, current_instructor_name);
}

void peloton::pzp_trainrows(QList<trainrow> *list) {

    trainrows.clear();
    for (const trainrow &r : qAsConst(*list)) {

        trainrows.append(r);
    }
    if (trainrows.isEmpty()) {
        current_api = no_metrics;
    }

    emit workoutStarted(current_workout_name, current_instructor_name);
}

void peloton::startEngine() {
    if (peloton_credentials_wrong) {
        return;
    }

    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    timer->stop();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::login_onfinish);
        
    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/me"));
    qDebug() << "peloton::getMe" << url;
    QNetworkRequest request(url);

    RAWHEADER
    
    //qDebug() << getPelotonTokenForUser(QZSettings::peloton_accesstoken, userId, QZSettings::default_peloton_accesstoken).toString().toLatin1() << request.rawHeader(QByteArray("authorization"));
    
    mgr->get(request);
}

void peloton::login_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::login_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = document.object();
    int status = json[QStringLiteral("status")].toInt();

    if (log_request) {
        qDebug() << QStringLiteral("login_onfinish") << document << payload;
    } else {
        qDebug() << QStringLiteral("login_onfinish");
    }
    
    // Check if document is empty, and if so, retry after 5 seconds
    if (document.isEmpty()) {
        qDebug() << QStringLiteral("login_onfinish: Empty document received, retrying in 5 seconds");
        QTimer::singleShot(5000, this, &peloton::startEngine);
        return;
    }

    if (status != 0) {

        peloton_credentials_wrong = true;
        qDebug() << QStringLiteral("invalid peloton credentials during login ") << status;
        homeform::singleton()->setToastRequested("Peloton Auth Failed!");
        emit loginState(false);
        return;
    }

    user_id = document[QStringLiteral("id")].toString();
    total_workout = document[QStringLiteral("total_workouts")].toInt();

    QSettings settings;
    // if it's a new user
    if(user_id.compare(settings.value(QZSettings::peloton_current_user_id, QZSettings::default_peloton_current_user_id).toString()) || !tempAccessToken.isEmpty()) {
        qDebug() << "new peloton user id, saving information..." << user_id;
        settings.setValue(QZSettings::peloton_current_user_id, user_id);
        if (!tempAccessToken.isEmpty()) {
            savePelotonTokenForUser(QZSettings::peloton_accesstoken, tempAccessToken, user_id);
            savePelotonTokenForUser(QZSettings::peloton_refreshtoken, tempRefreshToken, user_id);
            savePelotonTokenForUser(QZSettings::peloton_expires, tempExpiresAt, user_id);
            tempAccessToken.clear();
            tempRefreshToken.clear();
            qDebug() << "Assigned temporary tokens to new user:" << user_id;
            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Welcome " + document[QStringLiteral("username")].toString());
            }
        } else {
            settings.setValue(getPelotonSettingKey(QZSettings::peloton_refreshtoken, user_id), settings.value(QZSettings::peloton_refreshtoken, QZSettings::default_peloton_refreshtoken).toString());
            settings.setValue(getPelotonSettingKey(QZSettings::peloton_accesstoken, user_id), settings.value(QZSettings::peloton_accesstoken, QZSettings::default_peloton_accesstoken).toString());
        }
    }
    

    qDebug() << "user_id" << user_id << "total workout" << total_workout;
    
    emit loginState(!user_id.isEmpty());

    getWorkoutList(1);
}

void peloton::workoutlist_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::workoutlist_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = current_workout.object();
    QJsonArray data = json[QStringLiteral("data")].toArray();
    qDebug() << QStringLiteral("peloton::workoutlist_onfinish data") << payload << data;

    if (data.isEmpty()) {
        qDebug() << QStringLiteral(
            "peloton::workoutlist_onfinish Peloton API doesn't answer, trying back in 10 seconds...");
        timer->start(10s);
        return;
    }

    QString id = data.at(0)[QStringLiteral("id")].toString();
    QString status = data.at(0)[QStringLiteral("status")].toString();
    start_time = data.at(0)[QStringLiteral("start_time")].toInt();

    if ((status.contains(QStringLiteral("IN_PROGRESS"),
                         Qt::CaseInsensitive) && // NOTE: removed toUpper because of qstring-insensitive-allocation
         !current_workout_status.contains(QStringLiteral("IN_PROGRESS"))) ||
        (status.contains(QStringLiteral("IN_PROGRESS"), Qt::CaseInsensitive) && id != current_workout_id) ||
        testMode) { // NOTE: removed toUpper because of qstring-insensitive-allocation

        if (testMode)
            id = "eaa6f381891443b995f68f89f9a178be";
        current_workout_id = id;

        // starting a workout
        qDebug() << QStringLiteral("peloton::workoutlist_onfinish workoutlist_onfinish IN PROGRESS!");

        if ((bluetoothManager && bluetoothManager->device()) || testMode) {
            getSummary(id);
            timer->start(1min); // timeout request
            current_workout_status = status;
        } else {
            timer->start(10s); // check for a status changed
            // I don't need to set current_workout_status because the bike was missing and then I didn't set the
            // workout
        }
    } else {

        // getSummary(current_workout_id); // debug
        timer->start(10s); // check for a status changed
        current_workout_status = status;
        current_workout_id = id;
    }

    if (log_request) {
        qDebug() << QStringLiteral("peloton::workoutlist_onfinish") << current_workout;
    }
    qDebug() << QStringLiteral("peloton::workoutlist_onfinish current workout id") << current_workout_id;
}

void peloton::summary_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::summary_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout_summary = QJsonDocument::fromJson(payload, &parseError);

    if (log_request) {
        qDebug() << QStringLiteral("peloton::summary_onfinish") << current_workout_summary;
    } else {
        qDebug() << QStringLiteral("peloton::summary_onfinish");
    }

    workout_retry_count = 0;

    getWorkout(current_workout_id);
}

void peloton::instructor_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::instructor_onfinish);

    QSettings settings;
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    instructor = QJsonDocument::fromJson(payload, &parseError);
    current_instructor_name = instructor.object()[QStringLiteral("name")].toString();

    if (log_request) {
        qDebug() << QStringLiteral("instructor_onfinish") << instructor;
    } else {
        qDebug() << QStringLiteral("instructor_onfinish");
    }

    QString air_time = current_original_air_time.toString(settings.value(QZSettings::peloton_date_format, QZSettings::default_peloton_date_format).toString());
    qDebug() << QStringLiteral("air_time ") + air_time;
    if (settings.value(QZSettings::peloton_date, QZSettings::default_peloton_date)
            .toString()
            .contains(QStringLiteral("Before"))) {
        current_workout_name = air_time + QStringLiteral(" ") + current_workout_name;
    } else if (settings.value(QZSettings::peloton_date, QZSettings::default_peloton_date)
                   .toString()
                   .contains(QStringLiteral("After"))) {
        current_workout_name = current_workout_name + QStringLiteral(" ") + air_time;
    }
    emit workoutChanged(current_workout_name, current_instructor_name);

    /*
    if (workout_name.toUpper().contains(QStringLiteral("POWER ZONE"))) {
        qDebug() << QStringLiteral("!!Peloton Power Zone Ride Override!!");
        getPerformance(current_workout_id);
    } else*/
    { getRide(current_ride_id); }
}

void peloton::downloadImage() {
    if (current_image_downloaded) {
        delete current_image_downloaded;
        current_image_downloaded = 0;
    }
    if (!current_image_url.isEmpty()) {
        current_image_downloaded = new fileDownloader(current_image_url);
    }
}

void peloton::workout_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::workout_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    workout = QJsonDocument::fromJson(payload, &parseError);

    if (workout.isNull() && workout_retry_count < 3) {
        workout_retry_count++;
        qDebug() << "Empty JSON document received, retry attempt" << workout_retry_count;
        QTimer::singleShot(2000, this, [this]() {
            getWorkout(current_workout_id);
        });
        return;
    } else if (workout.isNull() && workout_retry_count >= 3) {
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("Error: Failed to load workout data after 3 attempts");
    }
    workout_retry_count = 0;

    QJsonObject ride = workout.object()[QStringLiteral("ride")].toObject();
    current_workout_name = ride[QStringLiteral("title")].toString();
    current_instructor_id = ride[QStringLiteral("instructor_id")].toString();
    current_ride_id = ride[QStringLiteral("id")].toString();
    current_workout_type = ride[QStringLiteral("fitness_discipline")].toString();
    current_pedaling_duration = ride[QStringLiteral("duration")].toInt();
    current_image_url = ride[QStringLiteral("image_url")].toString();

    qint64 time = ride[QStringLiteral("scheduled_start_time")].toInt();
    qDebug() << QStringLiteral("scheduled_start_time") << time;
    qDebug() << QStringLiteral("duration") << current_pedaling_duration;

    current_original_air_time = QDateTime::fromSecsSinceEpoch(time, Qt::UTC);

    if (log_request) {
        qDebug() << QStringLiteral("peloton::workout_onfinish") << workout;
    } else {
        qDebug() << QStringLiteral("peloton::workout_onfinish");
    }

    getInstructor(current_instructor_id);
}

int peloton::getIntroOffset() {
    return first_target_metrics_start_offset;
}

void peloton::ride_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::ride_onfinish);

    first_target_metrics_start_offset = 60; // default value

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject ride = document.object();

    // ride.pedaling_start_offset and instructor_cues[0].offset.start is
    // generally 60s for the intro, but let's ignore this since we assume
    // people are starting the workout after the intro
    QJsonArray instructor_cues = ride[QStringLiteral("instructor_cues")].toArray();

    trainrows.clear();
    if (instructor_cues.count() > 0)
        trainrows.reserve(instructor_cues.count() + 1);

    QSettings settings;
    QString difficulty =
        settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();
    QJsonObject segments = ride[QStringLiteral("segments")].toObject();
    QJsonArray segments_segment_list = segments[QStringLiteral("segment_list")].toArray();

    for (int i = 0; i < instructor_cues.count(); i++) {
        QJsonObject instructor_cue = instructor_cues.at(i).toObject();
        QJsonObject offsets = instructor_cue[QStringLiteral("offsets")].toObject();
        QJsonObject resistance_range = instructor_cue[QStringLiteral("resistance_range")].toObject();
        QJsonObject cadence_range = instructor_cue[QStringLiteral("cadence_range")].toObject();

        if (resistance_range.count() == 0 && cadence_range.count() == 0) {
            qDebug() << "no resistance and cadence found!";
            continue;
        }

        trainrow r;
        int duration = offsets[QStringLiteral("end")].toInt() - offsets[QStringLiteral("start")].toInt();
        if (i != 0) {
            // offsets have a 1s gap
            duration++;
        }

        r.lower_requested_peloton_resistance = resistance_range[QStringLiteral("lower")].toInt();
        r.upper_requested_peloton_resistance = resistance_range[QStringLiteral("upper")].toInt();

        r.lower_cadence = cadence_range[QStringLiteral("lower")].toInt();
        r.upper_cadence = cadence_range[QStringLiteral("upper")].toInt();

        r.average_requested_peloton_resistance =
            (r.lower_requested_peloton_resistance + r.upper_requested_peloton_resistance) / 2;
        r.average_cadence = (r.lower_cadence + r.upper_cadence) / 2;

        if (bluetoothManager && bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                r.lower_resistance = ((bike *)bluetoothManager->device())
                                         ->pelotonToBikeResistance(resistance_range[QStringLiteral("lower")].toInt());
                r.upper_resistance = ((bike *)bluetoothManager->device())
                                         ->pelotonToBikeResistance(resistance_range[QStringLiteral("upper")].toInt());
                r.average_resistance = ((bike *)bluetoothManager->device())
                                           ->pelotonToBikeResistance(r.average_requested_peloton_resistance);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                r.lower_resistance =
                    ((elliptical *)bluetoothManager->device())
                        ->pelotonToEllipticalResistance(resistance_range[QStringLiteral("lower")].toInt());
                r.upper_resistance =
                    ((elliptical *)bluetoothManager->device())
                        ->pelotonToEllipticalResistance(resistance_range[QStringLiteral("upper")].toInt());
                r.average_resistance = ((elliptical *)bluetoothManager->device())
                                           ->pelotonToEllipticalResistance(r.average_requested_peloton_resistance);
            }
        }

        // Set for compatibility
        if (difficulty == QStringLiteral("average")) {
            r.resistance = r.average_resistance;
            r.requested_peloton_resistance = r.average_requested_peloton_resistance;
            r.cadence = r.average_cadence;
        } else if (difficulty == QStringLiteral("upper")) {
            r.resistance = r.upper_resistance;
            r.requested_peloton_resistance = r.upper_requested_peloton_resistance;
            r.cadence = r.upper_cadence;
        } else { // lower
            r.resistance = r.lower_resistance;
            r.requested_peloton_resistance = r.lower_requested_peloton_resistance;
            r.cadence = r.lower_cadence;
        }

        // in order to have compact rows in the training program to have an Remaining Time tile set correctly
        if (i == 0 ||
            (r.lower_requested_peloton_resistance != trainrows.last().lower_requested_peloton_resistance ||
             r.upper_requested_peloton_resistance != trainrows.last().upper_requested_peloton_resistance ||
             r.lower_cadence != trainrows.last().lower_cadence || r.upper_cadence != trainrows.last().upper_cadence)) {
            r.duration = QTime(0, 0, 0).addSecs(duration);
            trainrows.append(r);
        } else {
            trainrows.last().duration = trainrows.last().duration.addSecs(duration);
        }
    }

    bool atLeastOnePower = false;
    if (trainrows.empty() && !segments_segment_list.isEmpty() &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::ROWING &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL) {
        foreach (QJsonValue o, segments_segment_list) {
            QJsonArray subsegments_v2 = o["subsegments_v2"].toArray();
            if (!subsegments_v2.isEmpty()) {
                foreach (QJsonValue s, subsegments_v2) {
                    trainrow r;
                    QString zone = s["display_name"].toString();
                    int len = s["length"].toInt();
                    if (!zone.toUpper().compare(QStringLiteral("SPIN UPS")) ||
                        !zone.toUpper().compare(QStringLiteral("SPIN-UPS"))) {
                        bool peloton_spinups_autoresistance =
                            settings
                                .value(QZSettings::peloton_spinups_autoresistance,
                                       QZSettings::default_peloton_spinups_autoresistance)
                                .toBool();
                        uint32_t Duration = len;
                        if (peloton_spinups_autoresistance) {
                            double PowerLow = 0.5;
                            double PowerHigh = 0.83;
                            for (uint32_t i = 0; i < Duration; i++) {
                                trainrow row;
                                row.duration = QTime(0, 0, 1, 0);
                                row.rampDuration =
                                    QTime((Duration - i) / 3600, (Duration - i) / 60, (Duration - i) % 60, 0);
                                row.rampElapsed = QTime(i / 3600, i / 60, i % 60, 0);
                                if (PowerHigh > PowerLow) {
                                    row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) *
                                                settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                                } else {
                                    row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) *
                                                settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                                }
                                qDebug() << row.duration << "power" << row.power << row.rampDuration << row.rampElapsed;
                                trainrows.append(row);
                            }
                        } else {
                            r.duration = QTime(0, len / 60, len % 60, 0);
                            r.power = -1;
                            trainrows.append(r);
                        }
                    } else if (!zone.toUpper().compare(QStringLiteral("DESCENDING RECOVERY"))) {
                        uint32_t Duration = len;
                        double PowerLow = 0.5;
                        double PowerHigh = 0.45;
                        for (uint32_t i = 0; i < Duration; i++) {
                            trainrow row;
                            row.duration = QTime(0, 0, 1, 0);
                            row.rampDuration =
                                QTime((Duration - i) / 3600, (Duration - i) / 60, (Duration - i) % 60, 0);
                            row.rampElapsed = QTime(i / 3600, i / 60, i % 60, 0);
                            if (PowerHigh > PowerLow) {
                                row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) *
                                            settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                            } else {
                                row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) *
                                            settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                            }
                            qDebug() << row.duration << "power" << row.power << row.rampDuration << row.rampElapsed;
                            trainrows.append(row);
                        }
                    } else if (!zone.toUpper().compare(QStringLiteral("RECOVERY"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.45;
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("FLAT ROAD"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("SWEET SPOT"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.91;
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("INTERVALS"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.75;
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 1"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 2"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.66;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 3"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.83;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 4"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.98;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 5"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.13;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 6"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.35;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 7"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.5;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else {
                        if(len > 0 && atLeastOnePower) {
                            r.duration = QTime(0, len / 60, len % 60, 0);
                            r.power = -1;
                            qDebug() << "ERROR not handled!" << zone;
                            trainrows.append(r);
                        }
                    }
                }
            }
        }
        // this list doesn't have nothing useful for this session
        if (!atLeastOnePower) {
            trainrows.clear();
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        QJsonObject target_metrics_data_list = ride[QStringLiteral("target_metrics_data")].toObject();
        QJsonArray pace_intensities_list = target_metrics_data_list[QStringLiteral("pace_intensities")].toArray();

        int pace_count = 0;        
        rower_pace_offset = -1;

        foreach (QJsonValue o, pace_intensities_list) {
            if(o["value"].toInt() < 0) {
                if(abs(o["value"].toInt()) > rower_pace_offset)
                    rower_pace_offset = abs(o["value"].toInt());
            }
        }
        
        qDebug() << "rower_pace_offset" << rower_pace_offset;

        foreach (QJsonValue o, pace_intensities_list) {
            qDebug() << o;
            pace_count = o["value"].toInt() + rower_pace_offset;
            if (pace_count < 5 && pace_count >= 0) {
                rower_pace[pace_count].display_name = o["display_name"].toString();
                rower_pace[pace_count].value = o["value"].toInt();

                QJsonArray levels = o["pace_levels"].toArray();
                if (levels.count() > 10) {
                    qDebug() << "peloton pace levels had been changed!";
                }
                int count = 0;
                foreach (QJsonValue level, levels) {
                    if(level["slug"].toString().split("_").count() > 1 ) {
                        count = level["slug"].toString().split("_")[1].toInt() - 1;
                        if (count >= 0 && count < 11) {
                            rower_pace[pace_count].levels[count].fast_pace = level["fast_pace"].toDouble();
                            rower_pace[pace_count].levels[count].slow_pace = level["slow_pace"].toDouble();
                            rower_pace[pace_count].levels[count].display_name = level["display_name"].toString();
                            rower_pace[pace_count].levels[count].slug = level["slug"].toString();
                            
                            qDebug() << count << level << rower_pace[pace_count].levels[count].display_name
                            << rower_pace[pace_count].levels[count].fast_pace
                            << rower_pace[pace_count].levels[count].slow_pace
                            << rower_pace[pace_count].levels[count].slug;
                        } else {
                            qDebug() << level["slug"].toString() << "slug error";
                        }
                    } else {
                        qDebug() << level["slug"].toString() << "slug count error";
                    }
                }
                qDebug() << pace_count << rower_pace[pace_count].display_name << rower_pace[pace_count].value;
            } else {
                qDebug() << "pace_count error!";
            }
        }
    }

    QJsonObject target_metrics_data_list = ride[QStringLiteral("target_metrics_data")].toObject();
    if (!target_metrics_data_list.isEmpty()) {
        QJsonArray target_metrics = target_metrics_data_list["target_metrics"].toArray();
        if (!target_metrics.isEmpty()) {
            QJsonObject first_metric = target_metrics[0].toObject();
            QJsonObject offsets = first_metric["offsets"].toObject();
            if (!offsets.isEmpty()) {
                first_target_metrics_start_offset = offsets["start"].toInt();
                qDebug() << "First target metrics start offset:" << first_target_metrics_start_offset;
            }
        }
    }
    if (trainrows.empty() && !target_metrics_data_list.isEmpty() &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::ROWING &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL) {
        QJsonArray target_metrics = target_metrics_data_list["target_metrics"].toArray();

        bool atLeastOnePower = false;
        int lastEnd = 60; // Starting offset, similar to performance_onfinish

        // Convert QJsonArray to QList for sorting by start time
        QList<QJsonValue> sortedMetrics;
        for (const QJsonValue &metric : target_metrics) {
            sortedMetrics.append(metric);
        }

               // Sort the list by "start" time to ensure proper order
        std::sort(sortedMetrics.begin(), sortedMetrics.end(), [](const QJsonValue &a, const QJsonValue &b) {
            int startA = a.toObject()[QStringLiteral("offsets")].toObject()[QStringLiteral("start")].toInt();
            int startB = b.toObject()[QStringLiteral("offsets")].toObject()[QStringLiteral("start")].toInt();
            return startA < startB;
        });

        for (const QJsonValue& segment : sortedMetrics) {
            QJsonObject segmentObj = segment.toObject();
            QJsonObject offsets = segmentObj["offsets"].toObject();
            QJsonArray metrics = segmentObj["metrics"].toArray();

            int start = offsets["start"].toInt();
            int end = offsets["end"].toInt();

            // Check if there's a gap from previous segment and add filler if needed
            if (!trainrows.isEmpty()) {
                int expectedStart = lastEnd + 1;
                if (start > expectedStart) {
                    // Add gap row for missing time
                    trainrow gapRow;
                    int gapDuration = start - expectedStart;
                    gapRow.duration = QTime(0, gapDuration / 60, gapDuration % 60, 0);
                    gapRow.power = -1; // No power target for gap
                    qDebug() << "Adding gap row of" << gapDuration << "seconds from" << expectedStart << "to" << (start - 1);
                    trainrows.append(gapRow);
                }
            }

            lastEnd = end;

            trainrow r;
            r.duration = QTime(0, 0, 0).addSecs(end - start + 1);

            bool isPowerZone = false;
            bool hasResistanceCadence = false;
            int powerZone = 0;
            int lowerResistance = 0, upperResistance = 0;
            int lowerCadence = 0, upperCadence = 0;

            // Analyze metrics to determine workout type
            for (const QJsonValue& metricValue : metrics) {
                QJsonObject metric = metricValue.toObject();
                QString name = metric["name"].toString();

                if (name == "power_zone") {
                    isPowerZone = true;
                    int lower = metric["lower"].toInt();
                    int upper = metric["upper"].toInt();

                    // Use difficulty setting for power zone
                    if (difficulty == QStringLiteral("average")) {
                        powerZone = (lower + upper) / 2;
                    } else if (difficulty == QStringLiteral("upper")) {
                        powerZone = upper;
                    } else { // lower
                        powerZone = lower;
                    }
                } else if (name == "resistance") {
                    hasResistanceCadence = true;
                    lowerResistance = metric["lower"].toInt();
                    upperResistance = metric["upper"].toInt();
                } else if (name == "cadence") {
                    hasResistanceCadence = true;
                    lowerCadence = metric["lower"].toInt();
                    upperCadence = metric["upper"].toInt();
                }
            }

            if (isPowerZone) {
                // Handle power zone workout
                switch(powerZone) {
                case 1:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
                    break;
                case 2:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.66;
                    break;
                case 3:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.83;
                    break;
                case 4:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.98;
                    break;
                case 5:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.13;
                    break;
                case 6:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.35;
                    break;
                case 7:
                    r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.5;
                    break;
                default:
                    qDebug() << "ERROR: Unhandled power zone!" << powerZone;
                    r.power = -1;
                    break;
                }
                if (r.power != -1) {
                    atLeastOnePower = true;
                }
                qDebug() << r.duration << "power zone" << powerZone << "power" << r.power << "time range" << start << "-" << end;
            } else if (hasResistanceCadence) {
                // Handle resistance/cadence workout
                r.lower_requested_peloton_resistance = lowerResistance;
                r.upper_requested_peloton_resistance = upperResistance;
                r.lower_cadence = lowerCadence;
                r.upper_cadence = upperCadence;

                r.average_requested_peloton_resistance = (lowerResistance + upperResistance) / 2;
                r.average_cadence = (lowerCadence + upperCadence) / 2;

                if (bluetoothManager && bluetoothManager->device()) {
                    if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                        r.lower_resistance = ((bike *)bluetoothManager->device())
                                                 ->pelotonToBikeResistance(lowerResistance);
                        r.upper_resistance = ((bike *)bluetoothManager->device())
                                                 ->pelotonToBikeResistance(upperResistance);
                        r.average_resistance = ((bike *)bluetoothManager->device())
                                                   ->pelotonToBikeResistance(r.average_requested_peloton_resistance);
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                        r.lower_resistance = ((elliptical *)bluetoothManager->device())
                                                 ->pelotonToEllipticalResistance(lowerResistance);
                        r.upper_resistance = ((elliptical *)bluetoothManager->device())
                                                 ->pelotonToEllipticalResistance(upperResistance);
                        r.average_resistance = ((elliptical *)bluetoothManager->device())
                                                   ->pelotonToEllipticalResistance(r.average_requested_peloton_resistance);
                    }
                }

                       // Set values based on difficulty preference
                if (difficulty == QStringLiteral("average")) {
                    r.resistance = r.average_resistance;
                    r.requested_peloton_resistance = r.average_requested_peloton_resistance;
                    r.cadence = r.average_cadence;
                } else if (difficulty == QStringLiteral("upper")) {
                    r.resistance = r.upper_resistance;
                    r.requested_peloton_resistance = r.upper_requested_peloton_resistance;
                    r.cadence = r.upper_cadence;
                } else { // lower
                    r.resistance = r.lower_resistance;
                    r.requested_peloton_resistance = r.lower_requested_peloton_resistance;
                    r.cadence = r.lower_cadence;
                }

                qDebug() << r.duration << "resistance" << r.lower_requested_peloton_resistance << "-" << r.upper_requested_peloton_resistance
                         << "cadence" << r.lower_cadence << "-" << r.upper_cadence << "time range" << start << "-" << end;
            }

            if (isPowerZone || hasResistanceCadence) {
                trainrows.append(r);
            }
        }

               // Check duration consistency
        QTime duration(0,0,0,0);
        foreach(trainrow r, trainrows) {
            duration = duration.addSecs(QTime(0,0,0,0).secsTo(r.duration));
            qDebug() << duration << r.duration;
        }

        if (current_workout_type.contains("bootcamp", Qt::CaseInsensitive)) {
            qDebug() << "Skipping pedaling duration check due to bootcamp workout type:" << current_workout_type;
        } else {
            int diff = current_pedaling_duration - QTime(0,0,0,0).secsTo(duration);
            if (diff > 0 && diff < 10) {
                qDebug() << "WARNING: The difference between expected and actual duration is positive but less than 10 seconds:" << diff << "seconds";
            } else if(diff > 0) {
                qDebug() << "peloton sends less metrics than expected, let's remove this and fallback on HFB" << diff << current_pedaling_duration;
                trainrows.clear();
            }
        }

        // this list doesn't have nothing useful for this session
        if (!atLeastOnePower && trainrows.isEmpty()) {
            trainrows.clear();
        }
    }
    
    if (trainrows.empty() && !target_metrics_data_list.isEmpty() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        QJsonObject target_metrics_data = ride["target_metrics_data"].toObject();
        QJsonArray target_metrics = target_metrics_data["target_metrics"].toArray();
        
        if (!target_metrics.isEmpty()) {
            bool treadmill_force_speed = settings.value(QZSettings::treadmill_force_speed,
                                                      QZSettings::default_treadmill_force_speed).toBool();
            int peloton_treadmill_level = settings.value(QZSettings::peloton_treadmill_level,
                                                       QZSettings::default_peloton_treadmill_level).toInt() - 1;
            
            if (peloton_treadmill_level < 0 || peloton_treadmill_level > 9)
                peloton_treadmill_level = 0;

            double miles = 1.0;
            if (settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool()) { // i didn't find the unit in the json
                miles = 1.60934;
            }

            for (const QJsonValue& segment : target_metrics) {
                QJsonObject segmentObj = segment.toObject();
                QJsonArray metrics = segmentObj["metrics"].toArray();
                QJsonObject offsets = segmentObj["offsets"].toObject();
                QString segment_type = segmentObj["segment_type"].toString();
                bool floorSegment = false;

                if(segment_type.startsWith("floor")) { // bootcamp
                    floorSegment = true;
                }
                
                // Skip if no metrics or invalid offsets
                if ((metrics.isEmpty() || offsets.isEmpty()) && !floorSegment)
                    continue;

                double speed_lower = -1;
                double speed_upper = -1;
                double inc_lower = -100;
                double inc_upper = -100;
                int pace_intensity_lower = -1;
                int pace_intensity_upper = -1;

                // Process metrics (speed, incline, pace_intensity)
                for (const QJsonValue& metric : metrics) {
                    QJsonObject metricObj = metric.toObject();
                    QString metricName = metricObj["name"].toString().toLower();

                    if (metricName == "pace_intensity") {
                        pace_intensity_lower = metricObj["lower"].toInt();
                        pace_intensity_upper = metricObj["upper"].toInt();
                        
                        speed_lower = treadmill_pace[pace_intensity_lower].levels[peloton_treadmill_level].slow_pace;
                        speed_upper = treadmill_pace[pace_intensity_upper].levels[peloton_treadmill_level].fast_pace;

                        miles = 1; // the pace intensity are always in km/h
                    }
                    else if (metricName == "speed") {
                        speed_lower = metricObj["lower"].toDouble();
                        speed_upper = metricObj["upper"].toDouble();
                    }
                    else if (metricName == "incline") {
                        inc_lower = metricObj["lower"].toDouble();
                        inc_upper = metricObj["upper"].toDouble();
                    }
                }

                // Create training row
                trainrow row;
                if (speed_lower != -1)
                    row.forcespeed = treadmill_force_speed;

                // Set duration
                int start = offsets["start"].toInt();
                int end = offsets["end"].toInt();
                row.duration = QTime(0, 0, 0).addSecs(end - start + 1);

                // Apply difficulty settings
                if (difficulty.toUpper() == "LOWER") {
                    if (speed_lower != -1)
                        row.speed = speed_lower * miles;
                    if (inc_lower != -100)
                        row.inclination = inc_lower;
                    if (pace_intensity_lower != -1)
                        row.pace_intensity = pace_intensity_lower;
                }
                else if (difficulty.toUpper() == "UPPER") {
                    if (speed_lower != -1)
                        row.speed = speed_upper * miles;
                    if (inc_lower != -100)
                        row.inclination = inc_upper;
                    if (pace_intensity_upper != -1)
                        row.pace_intensity = pace_intensity_upper;
                }
                else { // AVERAGE
                    if (speed_lower != -1)
                        row.speed = (speed_lower + speed_upper) / 2.0 * miles;
                    if (inc_lower != -100)
                        row.inclination = (inc_lower + inc_upper) / 2.0;
                    if (pace_intensity_lower != -1)
                        row.pace_intensity = (pace_intensity_lower + pace_intensity_upper) / 2;
                }

                // Store range values
                if (speed_lower != -1) {
                    row.lower_speed = speed_lower * miles;
                    row.average_speed = (speed_lower + speed_upper) / 2.0 * miles;
                    row.upper_speed = speed_upper * miles;
                }

                if (inc_lower != -100) {
                    // Apply inclination adjustments
                    double offset = settings.value(QZSettings::zwift_inclination_offset,
                                                QZSettings::default_zwift_inclination_offset).toDouble();
                    double gain = settings.value(QZSettings::zwift_inclination_gain,
                                              QZSettings::default_zwift_inclination_gain).toDouble();

                    row.lower_inclination = inc_lower * gain + offset;
                    row.average_inclination = (inc_lower + inc_upper) / 2.0 * gain + offset;
                    row.upper_inclination = inc_upper * gain + offset;
                    row.inclination = row.inclination * gain + offset;
                }

                qDebug() << row.duration << row.speed << row.inclination;
                trainrows.append(row);
            }
            
            QTime duration(0,0,0,0);
            foreach(trainrow r, trainrows) {
                duration = duration.addSecs(QTime(0,0,0,0).secsTo(r.duration));
                qDebug() << duration << r.duration;
            }
            if (current_workout_type.contains("bootcamp", Qt::CaseInsensitive)) {
                qDebug() << "Skipping pedaling duration check due to bootcamp workout type:" << current_workout_type;
            } else {
                int diff = current_pedaling_duration - QTime(0,0,0,0).secsTo(duration);
                if (diff > 0 && diff < 10) {
                    qDebug() << "WARNING: The difference between expected and actual duration is positive but less than 10 seconds:" << diff << "seconds";
                } else if(diff > 0) {
                    qDebug() << "peloton sends less metrics than expected, let's remove this and fallback on HFB" << diff << current_pedaling_duration;
                    trainrows.clear();
                }
            }
        }
    }


    if (log_request) {
        qDebug() << "peloton::ride_onfinish" << trainrows.length() << ride;
    } else {
        qDebug() << "peloton::ride_onfinish" << trainrows.length();
    }

    if (!trainrows.isEmpty()) {
        emit workoutStarted(current_workout_name, current_instructor_name);
        timer->start(30s); // check for a status changed
    } else {
        // fallback
        getPerformance(current_workout_id);
    }
}

void peloton::performance_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::performance_onfinish);

    QSettings settings;
    QString difficulty =
        settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    performance = QJsonDocument::fromJson(payload, &parseError);
    current_api = peloton_api;

    QJsonObject json = performance.object();
    QJsonObject target_performance_metrics = json[QStringLiteral("target_performance_metrics")].toObject();
    QJsonObject target_metrics_performance_data = json[QStringLiteral("target_metrics_performance_data")].toObject();
    QJsonArray segment_list = json[QStringLiteral("segment_list")].toArray();
    trainrows.clear();

    if(!target_metrics_performance_data.isEmpty() && bluetoothManager->device() &&
        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
        QJsonArray targetMetrics = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();

        if (targetMetrics.count() > 0)
            trainrows.reserve(targetMetrics.count());

        QSettings settings;
        QString difficulty =
            settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();
        bool powerZoneFound = false;
        
        for (int i = 0; i < targetMetrics.count(); i++) {
            QJsonObject targetMetric = targetMetrics.at(i).toObject();
            QJsonObject offsets = targetMetric[QStringLiteral("offsets")].toObject();
            QJsonArray metrics = targetMetric[QStringLiteral("metrics")].toArray();

                   // Find resistance and cadence metrics
            int lowerResistance = 0, upperResistance = 0, lowerCadence = 0, upperCadence = 0;
            for (QJsonValue metricValue : metrics) {
                QJsonObject metric = metricValue.toObject();
                QString name = metric[QStringLiteral("name")].toString();

                if (name == QStringLiteral("resistance")) {
                        lowerResistance = metric[QStringLiteral("lower")].toInt();
                        upperResistance = metric[QStringLiteral("upper")].toInt();
                } else if (name == QStringLiteral("cadence")) {
                        lowerCadence = metric[QStringLiteral("lower")].toInt();
                        upperCadence = metric[QStringLiteral("upper")].toInt();
                } else if (name == QStringLiteral("power_zone")) {
                    powerZoneFound = true;
                    break;
                }
            }

            if(powerZoneFound == true)
                break;
            
            trainrow r;
            int duration = offsets[QStringLiteral("end")].toInt() - offsets[QStringLiteral("start")].toInt();
            if (i != 0) {
                // offsets have a 1s gap
                duration++;
            }

            r.lower_requested_peloton_resistance = lowerResistance;
            r.upper_requested_peloton_resistance = upperResistance;
            r.lower_cadence = lowerCadence;
            r.upper_cadence = upperCadence;

            r.average_requested_peloton_resistance =
                (r.lower_requested_peloton_resistance + r.upper_requested_peloton_resistance) / 2;
            r.average_cadence = (r.lower_cadence + r.upper_cadence) / 2;

            if (bluetoothManager && bluetoothManager->device()) {
                if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                        r.lower_resistance = ((bike *)bluetoothManager->device())
                                                 ->pelotonToBikeResistance(lowerResistance);
                        r.upper_resistance = ((bike *)bluetoothManager->device())
                                                 ->pelotonToBikeResistance(upperResistance);
                        r.average_resistance = ((bike *)bluetoothManager->device())
                                                   ->pelotonToBikeResistance(r.average_requested_peloton_resistance);
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                        r.lower_resistance =
                            ((elliptical *)bluetoothManager->device())
                                ->pelotonToEllipticalResistance(lowerResistance);
                        r.upper_resistance =
                            ((elliptical *)bluetoothManager->device())
                                ->pelotonToEllipticalResistance(upperResistance);
                        r.average_resistance = ((elliptical *)bluetoothManager->device())
                                                   ->pelotonToEllipticalResistance(r.average_requested_peloton_resistance);
                }
            }

                   // Set for compatibility
            if (difficulty == QStringLiteral("average")) {
                r.resistance = r.average_resistance;
                r.requested_peloton_resistance = r.average_requested_peloton_resistance;
                r.cadence = r.average_cadence;
            } else if (difficulty == QStringLiteral("upper")) {
                r.resistance = r.upper_resistance;
                r.requested_peloton_resistance = r.upper_requested_peloton_resistance;
                r.cadence = r.upper_cadence;
            } else { // lower
                r.resistance = r.lower_resistance;
                r.requested_peloton_resistance = r.lower_requested_peloton_resistance;
                r.cadence = r.lower_cadence;
            }

                   // Compact rows in the training program
            if (i == 0 ||
                (r.lower_requested_peloton_resistance != trainrows.last().lower_requested_peloton_resistance ||
                 r.upper_requested_peloton_resistance != trainrows.last().upper_requested_peloton_resistance ||
                 r.lower_cadence != trainrows.last().lower_cadence ||
                 r.upper_cadence != trainrows.last().upper_cadence)) {
                r.duration = QTime(0, 0, 0).addSecs(duration);
                trainrows.append(r);
            } else {
                trainrows.last().duration = trainrows.last().duration.addSecs(duration);
            }
        }
        
        foreach(trainrow r, trainrows) {
            qDebug() << r.duration << r.average_cadence << r.average_resistance;
        }

        QJsonArray targetMetricsList = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();
        
        bool atLeastOnePower = false;
        if (trainrows.empty() && !targetMetricsList.isEmpty() &&
            bluetoothManager->device()->deviceType() != bluetoothdevice::ROWING &&
            bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL) {

            int lastEnd = 60;
            
            // Convert QJsonArray in QList for sorting
            QList<QJsonValue> sortedMetrics;
            for (const QJsonValue &metric : targetMetricsList) {
                sortedMetrics.append(metric);
            }

            // sort the list for "start"
            std::sort(sortedMetrics.begin(), sortedMetrics.end(), [](const QJsonValue &a, const QJsonValue &b) {
                int startA = a.toObject()[QStringLiteral("offsets")].toObject()[QStringLiteral("start")].toInt();
                int startB = b.toObject()[QStringLiteral("offsets")].toObject()[QStringLiteral("start")].toInt();
                return startA < startB;
            });

            for (QJsonValue metric : sortedMetrics) {
                QJsonObject metricObj = metric.toObject();
                QJsonObject offsets = metricObj[QStringLiteral("offsets")].toObject();
                int start = offsets[QStringLiteral("start")].toInt();
                int end = offsets[QStringLiteral("end")].toInt();
                int len = end - start + 1;
                
                // Check if there's a gap from previous segment
                if (!trainrows.isEmpty()) {
                   int prevEnd = start - 1; // Expected previous end
                   if (lastEnd < prevEnd) {
                       // Add gap row
                       trainrow gapRow;
                       gapRow.duration = QTime(0, (prevEnd - lastEnd + 1) / 60, (prevEnd - lastEnd + 1) % 60, 0);
                       gapRow.power = -1;
                       qDebug() << "adding a gap row of " << gapRow.duration << " seconds because start was " << start << " and end " << lastEnd;
                       trainrows.append(gapRow);
                   }
                }
                
                lastEnd = end;

                QJsonArray metricsArray = metricObj[QStringLiteral("metrics")].toArray();
                if (!metricsArray.isEmpty()) {
                        QJsonObject powerMetric = metricsArray[0].toObject();
                        int zone = powerMetric[QStringLiteral("lower")].toInt();

                        trainrow r;
                        r.duration = QTime(0, len / 60, len % 60, 0);

                        switch(zone) {
                        case 1: // Zone 1 / Recovery
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
                        break;
                        case 2: // Zone 2
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.66;
                        break;
                        case 3: // Zone 3
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.83;
                        break;
                        case 4: // Zone 4 / Sweet Spot
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.98;
                        break;
                        case 5: // Zone 5
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.13;
                        break;
                        case 6: // Zone 6
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.35;
                        break;
                        case 7: // Zone 7
                            r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.5;
                        break;
                        default:
                            r.power = -1;
                        break;
                        }

                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power << "zone" << zone;
                }
            }

            // If this list doesn't have anything useful for this session
            if (!atLeastOnePower) {
                trainrows.clear();
            }
        }
    } else if (!target_metrics_performance_data.isEmpty() && bluetoothManager->device() &&
        bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        double miles = 1;
        bool treadmill_force_speed =
            settings.value(QZSettings::treadmill_force_speed, QZSettings::default_treadmill_force_speed).toBool();
        int peloton_treadmill_level =
            settings.value(QZSettings::peloton_treadmill_level, QZSettings::default_peloton_treadmill_level).toInt() -
            1;
        if(peloton_treadmill_level < 0 || peloton_treadmill_level > 9)
            peloton_treadmill_level = 0;
        QJsonArray target_metrics = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();
        QJsonObject splits_data = json[QStringLiteral("splits_data")].toObject();
        if (!splits_data[QStringLiteral("distance_marker_display_unit")].toString().toUpper().compare("MI"))
            miles = 1.60934;
        trainrows.reserve(target_metrics.count() + 2);
        for (int i = 0; i < target_metrics.count(); i++) {
            QJsonObject metrics = target_metrics.at(i).toObject();
            QJsonArray metrics_ar = metrics[QStringLiteral("metrics")].toArray();
            QJsonObject offset = metrics[QStringLiteral("offsets")].toObject();
            QString segment_type = metrics[QStringLiteral("segment_type")].toString();
            qDebug() << metrics << metrics_ar << offset;
            if (metrics_ar.count() > 0 && !offset.isEmpty()) {
                double speed_lower = -1;
                double speed_upper = -1;
                double speed_average = -1;
                double inc_lower = -100;
                double inc_upper = -100;
                double inc_average = -100;
                int paceintensity_lower = -1;
                int paceintensity_upper = -1;
                int paceintensity_avg = -1;

                for(int metrics=0; metrics<metrics_ar.count(); metrics++) {
                    QJsonObject oo = metrics_ar.at(metrics).toObject();
                    if(oo[QStringLiteral("name")].toString().toLower() == "speed") {
                        speed_lower = oo[QStringLiteral("lower")].toDouble();
                        speed_upper = oo[QStringLiteral("upper")].toDouble();
                        speed_average = (((speed_upper - speed_lower) / 2.0) + speed_lower) * miles;
                    } else if(oo[QStringLiteral("name")].toString().toLower() == "incline") {
                        inc_lower = oo[QStringLiteral("lower")].toDouble();
                        inc_upper = oo[QStringLiteral("upper")].toDouble();
                        inc_average = ((inc_upper - inc_lower) / 2.0) + inc_lower;
                    } else if(oo[QStringLiteral("name")].toString().toLower() == "pace_intensity") {
                        paceintensity_lower = oo[QStringLiteral("lower")].toInt();
                        paceintensity_upper = oo[QStringLiteral("upper")].toInt();
                        paceintensity_avg = ((paceintensity_upper - paceintensity_lower) / 2.0) + paceintensity_lower;
                        if(paceintensity_lower < 7) {
                            speed_lower = treadmill_pace[paceintensity_lower].levels[peloton_treadmill_level].slow_pace;
                            speed_upper = treadmill_pace[paceintensity_upper].levels[peloton_treadmill_level].fast_pace;
                            speed_average = (((speed_upper - speed_lower) / 2.0) + speed_lower) * miles;
                            miles = 1; // the pace intensity are always in km/h
                        }
                    }
                }
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                // keeping the same bike behaviour
                /*if(i == 0 && offset_start > 0) {
                    trainrow r;
                    r.forcespeed = false;
                    r.duration = QTime(0, 0, 0, 0);
                    r.duration = r.duration.addSecs(offset_start);
                    trainrows.append(r);
                    qDebug() << i << r.duration << r.speed << r.inclination;
                }*/
                trainrow r;
                if(speed_lower != -1)
                    r.forcespeed = treadmill_force_speed;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);

                if (!difficulty.toUpper().compare(QStringLiteral("LOWER"))) {
                    if(speed_lower != -1)
                        r.speed = speed_lower * miles;
                    if(inc_lower != -100)
                        r.inclination = inc_lower;
                    if(paceintensity_avg != -1) {
                        r.pace_intensity = paceintensity_lower;
                    }
                } else if (!difficulty.toUpper().compare(QStringLiteral("UPPER"))) {
                    if(speed_lower != -1)
                        r.speed = speed_upper * miles;
                    if(inc_lower != -100)
                        r.inclination = inc_upper;
                    if(paceintensity_avg != -1) {
                        r.pace_intensity = paceintensity_upper;
                    }
                } else {
                    if(speed_lower != -1)
                        r.speed = (((speed_upper - speed_lower) / 2.0) + speed_lower) * miles;
                    if(inc_lower != -100)
                        r.inclination = ((inc_upper - inc_lower) / 2.0) + inc_lower;
                    if(paceintensity_avg != -1) {
                        r.pace_intensity = paceintensity_avg;
                    }
                }

                double offset =
                    settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset)
                        .toDouble();
                double gain =
                    settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain)
                        .toDouble();

                if(inc_lower != -100) {
                    r.lower_inclination = inc_lower;
                    r.average_inclination = inc_average;
                    r.upper_inclination = inc_upper;
                    r.inclination *= gain;
                    r.inclination += offset;
                    r.lower_inclination *= gain;
                    r.lower_inclination += offset;
                    r.average_inclination *= gain;
                    r.average_inclination += offset;
                    r.upper_inclination *= gain;
                    r.upper_inclination += offset;
                }

                if(speed_lower != -1) {
                    r.lower_speed = speed_lower * miles;
                    r.average_speed = speed_average * miles;
                    r.upper_speed = speed_upper * miles;
                }
                trainrows.append(r);
                qDebug() << i << r.duration << r.speed << r.inclination;
            } else if (segment_type.contains("floor") || segment_type.contains("free_mode")) {
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                trainrow r;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                trainrows.append(r);
                qDebug() << i << r.duration << r.speed << r.inclination;
            }
        }
    } else if (!target_metrics_performance_data.isEmpty() && bluetoothManager->device() &&
               bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        QJsonArray target_metrics = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();
        trainrows.reserve(target_metrics.count() + 2);
        for (int i = 0; i < target_metrics.count(); i++) {
            QJsonObject metrics = target_metrics.at(i).toObject();
            QJsonArray metrics_ar = metrics[QStringLiteral("metrics")].toArray();
            QJsonObject offset = metrics[QStringLiteral("offsets")].toObject();
            QString segment_type = metrics[QStringLiteral("segment_type")].toString();
            if (metrics_ar.count() > 1 && !offset.isEmpty()) {
                QJsonObject strokes_rate = metrics_ar.at(0).toObject();
                QJsonObject pace_intensity = metrics_ar.at(1).toObject();
                int peloton_rower_level =
                    settings.value(QZSettings::peloton_rower_level, QZSettings::default_peloton_rower_level).toInt() -
                    1;
                double strokes_rate_lower = strokes_rate[QStringLiteral("lower")].toDouble();
                double strokes_rate_upper = strokes_rate[QStringLiteral("upper")].toDouble();
                int pace_intensity_lower = pace_intensity[QStringLiteral("lower")].toInt() + rower_pace_offset;
                int pace_intensity_upper = pace_intensity[QStringLiteral("upper")].toInt() + rower_pace_offset;
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                double strokes_rate_average = ((strokes_rate_upper - strokes_rate_lower) / 2.0) + strokes_rate_lower;
                trainrow r;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                if (!difficulty.toUpper().compare(QStringLiteral("LOWER"))) {
                    r.cadence = strokes_rate_lower;
                } else if (!difficulty.toUpper().compare(QStringLiteral("UPPER"))) {
                    r.cadence = strokes_rate_upper;
                } else {
                    r.cadence = ((strokes_rate_upper - strokes_rate_lower) / 2.0) + strokes_rate_lower;
                }

                if (pace_intensity_lower >= 0 && pace_intensity_lower < 5) {
                    r.average_speed =
                        (rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].fast_pace) +
                         rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].slow_pace)) /
                        2.0;
                    r.upper_speed =
                        rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].fast_pace);
                    r.lower_speed =
                        rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].slow_pace);

                    if (!difficulty.toUpper().compare(QStringLiteral("LOWER"))) {
                        r.pace_intensity = pace_intensity_lower;
                        r.speed = r.lower_speed;
                    } else if (!difficulty.toUpper().compare(QStringLiteral("UPPER"))) {
                        r.pace_intensity = pace_intensity_upper;
                        r.speed = r.upper_speed;
                    } else {
                        r.pace_intensity = (pace_intensity_upper + pace_intensity_lower) / 2;
                        r.speed = r.average_speed;
                    }
                    r.forcespeed = 1;
                }                

                r.lower_cadence = strokes_rate_lower;
                r.average_cadence = strokes_rate_average;
                r.upper_cadence = strokes_rate_upper;

                trainrows.append(r);
                qDebug() << i << r.duration << r.cadence << r.speed << r.upper_speed << r.lower_speed;
            } else if (segment_type.contains("floor") || segment_type.contains("free_mode")) {
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                trainrow r;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                trainrows.append(r);
                qDebug() << i << r.duration << r.cadence;
            }
        }
    }
    // Target METS it's quite useless so I removed, no one use this
    /* else if (!segment_list.isEmpty() && bluetoothManager->device()->deviceType() != bluetoothdevice::BIKE) {
        trainrows.reserve(segment_list.count() + 1);
        foreach (QJsonValue o, segment_list) {
            int len = o["length"].toInt();
            int mets = o["intensity_in_mets"].toInt();
            if (len > 0) {
                trainrow r;
                r.duration = QTime(0, len / 60, len % 60, 0);
                r.mets = mets;
                trainrows.append(r);
            }
        }
    }*/

    if (log_request) {
        qDebug() << QStringLiteral("peloton::performance_onfinish") << trainrows.length() << performance;
    } else {
        qDebug() << QStringLiteral("peloton::performance_onfinish") << trainrows.length();
    }

    if (!trainrows.isEmpty()) {

        emit workoutStarted(current_workout_name, current_instructor_name);
    } else {

        if (!PZP->searchWorkout(current_ride_id)) {
            current_api = homefitnessbuddy_api;
            HFB->searchWorkout(current_original_air_time.date(), current_instructor_name, current_pedaling_duration,
                               current_ride_id);
        } else {
            current_api = powerzonepack_api;
        }
    }

    timer->start(30s); // check for a status changed
}

double peloton::rowerpaceToSpeed(double pace) {
    float whole, fractional;

    fractional = std::modf(pace, &whole);
    double seconds = whole * 60.0;
    seconds += (fractional * 100.0);
    seconds *= 2.0;

    return 3600.0 / seconds;
}

void peloton::getInstructor(const QString &instructor_id) {
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::instructor_onfinish);

    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/instructor/") + instructor_id);
    qDebug() << "peloton::getInstructor" << url;
    QNetworkRequest request(url);

    RAWHEADER

    mgr->get(request);
}

void peloton::getRide(const QString &ride_id) {
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::ride_onfinish);

    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/ride/") + ride_id +
             QStringLiteral("/details?stream_source=multichannel"));
    qDebug() << "peloton::getRide" << url;
    QNetworkRequest request(url);

    RAWHEADER

    mgr->get(request);
}

void peloton::getPerformance(const QString &workout) {
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::performance_onfinish);

    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/workout/") + workout +
             QStringLiteral("/performance_graph?every_n=") + QString::number(peloton_workout_second_resolution));
    qDebug() << "peloton::getPerformance" << url;
    QNetworkRequest request(url);

    RAWHEADER

    mgr->get(request);
}

void peloton::getWorkout(const QString &workout) {
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::workout_onfinish);

    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/workout/") + workout);
    qDebug() << "peloton::getWorkout" << url;
    QNetworkRequest request(url);

    RAWHEADER

    mgr->get(request);
}

void peloton::getSummary(const QString &workout) {
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::summary_onfinish);

    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/workout/") + workout + QStringLiteral("/summary"));
    qDebug() << "peloton::getSummary" << url;
    QNetworkRequest request(url);

    RAWHEADER

    mgr->get(request);
}

void peloton::getWorkoutList(int num) {
    Q_UNUSED(num)
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    //    if (num == 0) { //NOTE: clang-analyzer-deadcode.DeadStores
    //        num = this->total_workout;
    //    }

    int limit = 1; // for now we don't need more than 1 workout
    // int pages = num / limit; //NOTE: clang-analyzer-deadcode.DeadStores
    // int rem = num % limit; //NOTE: clang-analyzer-deadcode.DeadStores

    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::workoutlist_onfinish);

    int current_page = 0;

    QUrl url(QStringLiteral("https://api-3p.onepeloton.com/api/v1/user") +
             QStringLiteral("/workouts?sort_by=-created&page=") + QString::number(current_page) +
             QStringLiteral("&limit=") + QString::number(limit));
    qDebug() << "peloton::getWorkoutList" << url;
    QNetworkRequest request(url);

    RAWHEADER

    mgr->get(request);
}

void peloton::setTestMode(bool test) { testMode = test; }

void peloton::onPelotonGranted() {

    pelotonAuthWebVisible = false;
    emit pelotonWebVisibleChanged(pelotonAuthWebVisible);
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    tempAccessToken = pelotonOAuth->token();
    tempRefreshToken = pelotonOAuth->refreshToken();
    tempExpiresAt = QDateTime::currentDateTime();

    qDebug() << QStringLiteral("peloton authenticathed");
    
    peloton_connect();
    
    if(homeform::singleton())
        homeform::singleton()->setPelotonPopupVisible(true);
    if(!timer->isActive()) {
        peloton_credentials_wrong = false;
        startEngine();
    }
}

void peloton::onPelotonAuthorizeWithBrowser(const QUrl &url) {

    // ui->textBrowser->append(tr("Open with browser:") + url.toString());
    QSettings settings;
    bool strava_auth_external_webbrowser =
        settings.value(QZSettings::strava_auth_external_webbrowser, QZSettings::default_strava_auth_external_webbrowser)
            .toBool();
#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS))
    strava_auth_external_webbrowser = true;
#endif
    pelotonAuthUrl = url.toString();
    emit pelotonAuthUrlChanged(pelotonAuthUrl);

    if (strava_auth_external_webbrowser)
        QDesktopServices::openUrl(url);
    else {
        pelotonAuthWebVisible = true;
        emit pelotonWebVisibleChanged(pelotonAuthWebVisible);
    }
}

void peloton::replyDataReceived(const QByteArray &v) {

    qDebug() << v;

    QByteArray data;
    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();
    QString s(v);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(s.toUtf8());

    tempAccessToken = jsonResponse[QStringLiteral("access_token")].toString();
    tempRefreshToken = jsonResponse[QStringLiteral("refresh_token")].toString();

    qDebug() << "Peloton tokens received successfully, expires at:" << jsonResponse[QStringLiteral("expires_at")];

    QString urlstr = QStringLiteral("https://www.peloton.com/oauth/token?");
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("client_id"), QStringLiteral(PELOTON_CLIENT_ID_S));
#ifdef PELOTON_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    params.addQueryItem("client_secret", STRINGIFY(PELOTON_SECRET_KEY));
#endif

    params.addQueryItem(QStringLiteral("code"), peloton_code);
    data.append(params.query(QUrl::FullyEncoded).toUtf8());

    // trade-in the temporary access code retrieved by the Call-Back URL for the finale token
    QUrl url = QUrl(urlstr);

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    // now get the final token - but ignore errors
    if (manager) {

        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    // connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this,
    // SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> & ))); connect(manager,
    // SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestFinished(QNetworkReply*)));
    manager->post(request, data);
}

void peloton::onSslErrors(QNetworkReply *reply, const QList<QSslError> &error) {

    reply->ignoreSslErrors();
    qDebug() << QStringLiteral("peloton::onSslErrors") << error;
}

void peloton::networkRequestFinished(QNetworkReply *reply) {

    QSettings settings;
    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();

    // we can handle SSL handshake errors, if we got here then some kind of protocol was agreed
    if (reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::SslHandshakeFailedError) {

        QByteArray payload = reply->readAll(); // JSON
        QString refresh_token;
        QString access_token;

        // parse the response and extract the tokens, pretty much the same for all services
        // although polar choose to also pass a user id, which is needed for future calls
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            refresh_token = document[QStringLiteral("refresh_token")].toString();
            access_token = document[QStringLiteral("access_token")].toString();
        }

        tempAccessToken = access_token;
        tempRefreshToken = refresh_token;
        tempExpiresAt = QDateTime::currentDateTime();

        qDebug() << "Peloton tokens refreshed successfully";

    } else {

        // general error getting response
        QString error =
            QString(tr("Error retrieving access token, %1 (%2)")).arg(reply->errorString()).arg(reply->error());
        qDebug() << error << reply->url() << reply->readAll();
    }
}

void peloton::callbackReceived(const QVariantMap &values) {
    qDebug() << QStringLiteral("peloton::callbackReceived") << values;
    if (!values.value(QZSettings::peloton_code).toString().isEmpty()) {
        peloton_code = values.value(QZSettings::peloton_code).toString();

        qDebug() << peloton_code;
    }
}

QOAuth2AuthorizationCodeFlow *peloton::peloton_connect() {
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    if (pelotonOAuth) {

        delete pelotonOAuth;
        pelotonOAuth = nullptr;
    }
    if (pelotonReplyHandler) {

        delete pelotonReplyHandler;
        pelotonReplyHandler = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    OAuth2Parameter parameter;
    pelotonOAuth = new QOAuth2AuthorizationCodeFlow(manager, this);
    pelotonOAuth->setScope(QStringLiteral("openid offline_access 3p.profile:r 3p.workout:r"));
    pelotonOAuth->setClientIdentifier(QStringLiteral(PELOTON_CLIENT_ID_S));
    pelotonOAuth->setAuthorizationUrl(QUrl(QStringLiteral("https://auth.onepeloton.com/oauth/authorize")));
    pelotonOAuth->setAccessTokenUrl(QUrl(QStringLiteral("https://auth.onepeloton.com/oauth/token")));
    pelotonOAuth->setModifyParametersFunction(
        buildModifyParametersFunction(QUrl(QLatin1String("")), QUrl(QLatin1String(""))));
    pelotonReplyHandler = new QOAuthHttpServerReplyHandler(QHostAddress(QStringLiteral("127.0.0.1")), 18080, this);
    connect(pelotonReplyHandler, &QOAuthHttpServerReplyHandler::replyDataReceived, this, &peloton::replyDataReceived);
    connect(pelotonReplyHandler, &QOAuthHttpServerReplyHandler::callbackReceived, this, &peloton::callbackReceived);

    pelotonOAuth->setReplyHandler(pelotonReplyHandler);

    return pelotonOAuth;
}

void peloton::peloton_connect_clicked() {
    timer->stop();
    
    QLoggingCategory::setFilterRules(QStringLiteral("qt.networkauth.*=true"));

    peloton_connect();
    connect(pelotonOAuth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &peloton::onPelotonAuthorizeWithBrowser);
    connect(pelotonOAuth, &QOAuth2AuthorizationCodeFlow::granted, this, &peloton::onPelotonGranted);

    pelotonOAuth->grant();
    // qDebug() <<
    // QAbstractOAuth2::post("https://www.peloton.com/oauth/authorize?client_id=7976&scope=activity:read_all,activity:write&redirect_uri=http://127.0.0.1&response_type=code&approval_prompt=force");
}

QAbstractOAuth::ModifyParametersFunction peloton::buildModifyParametersFunction(const QUrl &clientIdentifier, const QUrl &clientIdentifierSharedKey) {
    return [clientIdentifier, clientIdentifierSharedKey](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
            parameters->insert(QStringLiteral("audience"), QStringLiteral("https://api-3p.onepeloton.com/"));
            parameters->insert(QStringLiteral("responseType"), QStringLiteral("code")); /* Request refresh token*/
            parameters->insert(QStringLiteral("approval_prompt"),
                               QStringLiteral("force")); /* force user check scope again */
            QByteArray code = parameters->value(QStringLiteral("code")).toByteArray();
            // DON'T TOUCH THIS LINE, THANKS Roberto Viola
            (*parameters)[QStringLiteral("code")] = QUrl::fromPercentEncoding(code); // NOTE: Old code replaced by
        }
        if (stage == QAbstractOAuth::Stage::RefreshingAccessToken) {
            parameters->insert(QStringLiteral("client_id"), clientIdentifier);
            parameters->insert(QStringLiteral("client_secret"), clientIdentifierSharedKey);
        }
    };
}

void peloton::peloton_refreshtoken() {

    QSettings settings;
    // QUrlQuery params; //NOTE: clazy-unuse-non-tirial-variable

    QString userId = settings.value(QZSettings::peloton_current_user_id).toString();

    // If no user is logged in yet, just use the regular method
    if (userId.isEmpty()) {
        if (settings.value(QZSettings::peloton_refreshtoken).toString().isEmpty()) {
            peloton_connect();
            return;
        }
    }

    QNetworkRequest request(QUrl(QStringLiteral("https://auth.onepeloton.com/oauth/token?")));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // set params
    QString data;
    data += QStringLiteral("client_id=" PELOTON_CLIENT_ID_S);
    data += QStringLiteral("&refresh_token=") + (tempRefreshToken.isEmpty() ? getPelotonTokenForUser(QZSettings::peloton_refreshtoken, userId).toString() : tempRefreshToken);
    data += QStringLiteral("&grant_type=refresh_token");
    
    // make request
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, data.toLatin1());

    // blocking request
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply->error() != 0) {
        homeform::singleton()->setToastRequested("Peloton Auth Failed!");
        qDebug() << QStringLiteral("Got error") << reply->errorString().toStdString().c_str();
        return;
    }

    // lets extract the access token, and possibly a new refresh token
    QByteArray r = reply->readAll();
    qDebug() << QStringLiteral("Got response:") << r.data();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(r, &parseError);

    // failed to parse result !?
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << tr("JSON parser error") << parseError.errorString();
    }

    QString access_token = document[QStringLiteral("access_token")].toString();
    QString refresh_token = document[QStringLiteral("refresh_token")].toString();

    qDebug() << "userid: " << userId;
    tempAccessToken = access_token;
    tempRefreshToken = refresh_token;
    tempExpiresAt = QDateTime::currentDateTime();
    
    homeform::singleton()->setToastRequested("Peloton Login OK!");
    
}
