// This file contains the Home Assistant Discovery implementation for MQTTPublisher

void MQTTPublisher::publishDiscoveryConfig() {
    if (!isConnected()) return;
    
    qDebug() << "Publishing Home Assistant discovery configuration";
    
    QString baseTopic = getBaseTopic();
    QString controlTopic = getControlTopic();
    
    // Common sensors for all devices
    publishSensorDiscovery("speed_current", "Speed", baseTopic + "speed/current", "km/h", "", "mdi:speedometer");
    publishSensorDiscovery("speed_avg", "Average Speed", baseTopic + "speed/avg", "km/h", "", "mdi:speedometer");
    publishSensorDiscovery("distance", "Distance", baseTopic + "distance", "km", "distance", "mdi:map-marker-distance");
    publishSensorDiscovery("calories", "Calories", baseTopic + "calories", "kcal", "", "mdi:fire");
    publishSensorDiscovery("elapsed_time", "Elapsed Time", baseTopic + "elapsed/minutes", "min", "duration", "mdi:timer");
    publishSensorDiscovery("heart_current", "Heart Rate", baseTopic + "heart/current", "bpm", "", "mdi:heart-pulse");
    publishSensorDiscovery("heart_avg", "Average Heart Rate", baseTopic + "heart/avg", "bpm", "", "mdi:heart-pulse");
    publishSensorDiscovery("watts_current", "Power", baseTopic + "watts/current", "W", "power", "mdi:flash");
    publishSensorDiscovery("watts_avg", "Average Power", baseTopic + "watts/avg", "W", "power", "mdi:flash");
    publishSensorDiscovery("elevation", "Elevation Gain", baseTopic + "elevation", "m", "", "mdi:elevation-rise");
    
    // Binary sensors
    QString statusBaseTopic = QString("QZ/%1/").arg(m_userNickname);
    publishBinarySensorDiscovery("connected", "Device Connected", statusBaseTopic + "device/connected", "connectivity", "mdi:bluetooth");
    publishBinarySensorDiscovery("paused", "Workout Paused", statusBaseTopic + "device/paused", "", "mdi:pause");
    
    // Control entities based on device type
    if (m_device) {
        switch (m_device->deviceType()) {
            case bluetoothdevice::BIKE: {
                // Bike-specific sensors
                publishSensorDiscovery("bike_resistance", "Resistance", baseTopic + "bike/resistance/current", "", "", "mdi:tune");
                publishSensorDiscovery("bike_cadence", "Cadence", baseTopic + "bike/cadence/current", "rpm", "", "mdi:rotate-right");
                publishSensorDiscovery("bike_gears", "Gears", baseTopic + "bike/gears", "", "", "mdi:cog");
                publishSensorDiscovery("bike_power_zone", "Power Zone", baseTopic + "bike/power_zone", "", "", "mdi:target");
                
                // Bike controls
                publishNumberDiscovery("bike_resistance", "Bike Resistance", baseTopic + "bike/resistance/current", controlTopic + "bike/resistance", 0, 32, 1, "", "mdi:tune");
                publishNumberDiscovery("bike_power", "Target Power", baseTopic + "bike/target_power", controlTopic + "bike/power", 0, 1000, 5, "W", "mdi:flash");
                publishNumberDiscovery("bike_cadence", "Target Cadence", baseTopic + "bike/target_cadence", controlTopic + "bike/cadence", 0, 200, 1, "rpm", "mdi:rotate-right");
                
                // Bike buttons
                QJsonObject gearUpConfig;
                gearUpConfig["name"] = "Gear Up";
                gearUpConfig["unique_id"] = QString("qz_%1_gear_up").arg(m_userNickname);
                gearUpConfig["command_topic"] = controlTopic + "bike/gears_up";
                gearUpConfig["device"] = getDeviceInfo();
                gearUpConfig["icon"] = "mdi:plus";
                QString gearUpTopic = getDiscoveryTopic("button", "gear_up");
                m_client->publish(QMqttTopicName(gearUpTopic), QJsonDocument(gearUpConfig).toJson(QJsonDocument::Compact), 1, true);
                
                QJsonObject gearDownConfig;
                gearDownConfig["name"] = "Gear Down";
                gearDownConfig["unique_id"] = QString("qz_%1_gear_down").arg(m_userNickname);
                gearDownConfig["command_topic"] = controlTopic + "bike/gears_down";
                gearDownConfig["device"] = getDeviceInfo();
                gearDownConfig["icon"] = "mdi:minus";
                QString gearDownTopic = getDiscoveryTopic("button", "gear_down");
                m_client->publish(QMqttTopicName(gearDownTopic), QJsonDocument(gearDownConfig).toJson(QJsonDocument::Compact), 1, true);
                
                break;
            }
            case bluetoothdevice::TREADMILL: {
                // Treadmill-specific sensors
                publishSensorDiscovery("treadmill_inclination", "Inclination", baseTopic + "treadmill/inclination/current", "%", "", "mdi:angle-acute");
                publishSensorDiscovery("treadmill_cadence", "Cadence", baseTopic + "treadmill/cadence/current", "spm", "", "mdi:run");
                publishSensorDiscovery("stride_length", "Stride Length", baseTopic + "treadmill/stride_length", "cm", "", "mdi:ruler");
                
                // Treadmill controls
                publishNumberDiscovery("treadmill_speed", "Treadmill Speed", baseTopic + "treadmill/target_speed", controlTopic + "treadmill/speed", 0, 25, 0.1, "km/h", "mdi:speedometer");
                publishNumberDiscovery("treadmill_inclination", "Treadmill Inclination", baseTopic + "treadmill/inclination/current", controlTopic + "treadmill/inclination", -10, 40, 0.5, "%", "mdi:angle-acute");
                
                break;
            }
            case bluetoothdevice::ROWING: {
                // Rowing-specific sensors
                publishSensorDiscovery("rowing_resistance", "Resistance", baseTopic + "rowing/resistance/current", "", "", "mdi:tune");
                publishSensorDiscovery("rowing_cadence", "Stroke Rate", baseTopic + "rowing/cadence/current", "spm", "", "mdi:rowing");
                publishSensorDiscovery("strokes_count", "Strokes Count", baseTopic + "rowing/strokes_count", "", "", "mdi:counter");
                
                // Rowing controls
                publishNumberDiscovery("rowing_resistance", "Rowing Resistance", baseTopic + "rowing/resistance/current", controlTopic + "rowing/resistance", 0, 32, 1, "", "mdi:tune");
                publishNumberDiscovery("rowing_power", "Target Power", baseTopic + "rowing/target_power", controlTopic + "rowing/power", 0, 1000, 5, "W", "mdi:flash");
                
                break;
            }
            case bluetoothdevice::ELLIPTICAL: {
                // Elliptical-specific sensors  
                publishSensorDiscovery("elliptical_resistance", "Resistance", baseTopic + "elliptical/resistance/current", "", "", "mdi:tune");
                publishSensorDiscovery("elliptical_cadence", "Cadence", baseTopic + "elliptical/cadence/current", "rpm", "", "mdi:rotate-right");
                
                // Elliptical controls
                publishNumberDiscovery("elliptical_resistance", "Elliptical Resistance", baseTopic + "elliptical/resistance/current", controlTopic + "elliptical/resistance", 0, 32, 1, "", "mdi:tune");
                publishNumberDiscovery("elliptical_speed", "Elliptical Speed", baseTopic + "elliptical/target_speed", controlTopic + "elliptical/speed", 0, 25, 0.1, "km/h", "mdi:speedometer");
                
                break;
            }
            default:
                break;
        }
    }
    
    // Common control entities
    publishNumberDiscovery("fan_speed", "Fan Speed", statusBaseTopic + "device/fan_speed", controlTopic + "fan", 0, 100, 1, "%", "mdi:fan");
    
    // Control buttons
    QJsonObject startConfig;
    startConfig["name"] = "Start Workout";
    startConfig["unique_id"] = QString("qz_%1_start").arg(m_userNickname);
    startConfig["command_topic"] = controlTopic + "start";
    startConfig["device"] = getDeviceInfo();
    startConfig["icon"] = "mdi:play";
    QString startTopic = getDiscoveryTopic("button", "start");
    m_client->publish(QMqttTopicName(startTopic), QJsonDocument(startConfig).toJson(QJsonDocument::Compact), 1, true);
    
    QJsonObject stopConfig;
    stopConfig["name"] = "Stop Workout";
    stopConfig["unique_id"] = QString("qz_%1_stop").arg(m_userNickname);
    stopConfig["command_topic"] = controlTopic + "stop";
    stopConfig["device"] = getDeviceInfo();
    stopConfig["icon"] = "mdi:stop";
    QString stopTopic = getDiscoveryTopic("button", "stop");
    m_client->publish(QMqttTopicName(stopTopic), QJsonDocument(stopConfig).toJson(QJsonDocument::Compact), 1, true);
    
    // Pause switch
    publishSwitchDiscovery("pause", "Pause Workout", statusBaseTopic + "device/paused", controlTopic + "pause", "mdi:pause");
    
    qDebug() << "Home Assistant discovery configuration published";
}