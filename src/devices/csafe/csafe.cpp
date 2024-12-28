#include "csafe.h" // Include the header file containing csafe_dic definitions
#include <QList>
#include <QMap>

csafe::csafe() {

    // Short Commands
    /*
    cmds["CSAFE_GETSTATUS_CMD"] = QList<int>() << 0x80 << QList<int>();
    cmds["CSAFE_RESET_CMD"] = QList<int>() << 0x81 << QList<int>();
    cmds["CSAFE_GOIDLE_CMD"] = QList<int>() << 0x82 << QList<int>();
    cmds["CSAFE_GOHAVEID_CMD"] = QList<int>() << 0x83 << QList<int>();
    cmds["CSAFE_GOINUSE_CMD"] = QList<int>() << 0x85 << QList<int>();
    cmds["CSAFE_GOFINISHED_CMD"] = QList<int>() << 0x86 << QList<int>();
    cmds["CSAFE_GOREADY_CMD"] = QList<int>() << 0x87 << QList<int>();
    cmds["CSAFE_BADID_CMD"] = QList<int>() << 0x88 << QList<int>();
    cmds["CSAFE_GETVERSION_CMD"] = QList<int>() << 0x91 << QList<int>();
    cmds["CSAFE_GETID_CMD"] = QList<int>() << 0x92 << QList<int>();
    cmds["CSAFE_GETUNITS_CMD"] = QList<int>() << 0x93 << QList<int>();
    cmds["CSAFE_GETSERIAL_CMD"] = QList<int>() << 0x94 << QList<int>();
    cmds["CSAFE_GETODOMETER_CMD"] = QList<int>() << 0x9B << QList<int>();
    cmds["CSAFE_GETERRORCODE_CMD"] = QList<int>() << 0x9C << QList<int>();
    cmds["CSAFE_GETTWORK_CMD"] = QList<int>() << 0xA0 << QList<int>();
    cmds["CSAFE_GETHORIZONTAL_CMD"] = QList<int>() << 0xA1 << QList<int>();
    cmds["CSAFE_GETCALORIES_CMD"] = QList<int>() << 0xA3 << QList<int>();
    cmds["CSAFE_GETPROGRAM_CMD"] = QList<int>() << 0xA4 << QList<int>();
    cmds["CSAFE_GETPACE_CMD"] = QList<int>() << 0xA6 << QList<int>();
    cmds["CSAFE_GETCADENCE_CMD"] = QList<int>() << 0xA7 << QList<int>();
    cmds["CSAFE_GETUSERINFO_CMD"] = QList<int>() << 0xAB << QList<int>();
    cmds["CSAFE_GETHRCUR_CMD"] = QList<int>() << 0xB0 << QList<int>();
    cmds["CSAFE_GETPOWER_CMD"] = QList<int>() << 0xB4 << QList<int>();
    // Long Commands
    cmds["CSAFE_AUTOUPLOAD_CMD"] = QList<int>() << 0x01 << QList<int>() << 1;
    cmds["CSAFE_IDDIGITS_CMD"] = QList<int>() << 0x10 << QList<int>() << 1;
    cmds["CSAFE_SETTIME_CMD"] = QList<int>() << 0x11 << QList<int>() << 1 << 1 << 1;
    cmds["CSAFE_SETDATE_CMD"] = QList<int>() << 0x12 << QList<int>() << 1 << 1 << 1;
    cmds["CSAFE_SETTIMEOUT_CMD"] = QList<int>() << 0x13 << QList<int>() << 1;
    cmds["CSAFE_SETUSERCFG1_CMD"] = QList<int>() << 0x1A << QList<int>() << 0;
    cmds["CSAFE_SETTWORK_CMD"] = QList<int>() << 0x20 << QList<int>() << 1 << 1 << 1;
    cmds["CSAFE_SETHORIZONTAL_CMD"] = QList<int>() << 0x21 << QList<int>() << 2 << 1;
    cmds["CSAFE_SETCALORIES_CMD"] = QList<int>() << 0x23 << QList<int>() << 2;
    cmds["CSAFE_SETPROGRAM_CMD"] = QList<int>() << 0x24 << QList<int>() << 1 << 1;
    cmds["CSAFE_SETPOWER_CMD"] = QList<int>() << 0x34 << QList<int>() << 2 << 1;
    cmds["CSAFE_GETCAPS_CMD"] = QList<int>() << 0x70 << QList<int>() << 1;

    // PM3 Specific Short Commands
    cmds["CSAFE_PM_GET_WORKOUTTYPE"] = QList<int>() << 0x89 << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_DRAGFACTOR"] = QList<int>() << 0xC1 << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_STROKESTATE"] = QList<int>() << 0xBF << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_WORKTIME"] = QList<int>() << 0xA0 << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_WORKDISTANCE"] = QList<int>() << 0xA3 << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_ERRORVALUE"] = QList<int>() << 0xC9 << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_WORKOUTSTATE"] = QList<int>() << 0x8D << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_WORKOUTINTERVALCOUNT"] = QList<int>() << 0x9F << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_INTERVALTYPE"] = QList<int>() << 0x8E << QList<int>() << 0x1A;
    cmds["CSAFE_PM_GET_RESTTIME"] = QList<int>() << 0xCF << QList<int>() << 0x1A;

    // PM3 Specific Long Commands
    cmds["CSAFE_PM_SET_SPLITDURATION"] = QList<int>() << 0x05 << QList<int>() << 1 << 4 << 0x1A;
    cmds["CSAFE_PM_GET_FORCEPLOTDATA"] = QList<int>() << 0x6B << QList<int>() << 1 << 0x1A;
    cmds["CSAFE_PM_SET_SCREENERRORMODE"] = QList<int>() << 0x27 << QList<int>() << 1 << 0x1A;
    cmds["CSAFE_PM_GET_HEARTBEATDATA"] = QList<int>() << 0x6C << QList<int>() << 1 << 0x1A;*/

    cmds["CSAFE_PM_GET_WORKTIME"] = populateCmd(0xa0, QList<int>(), 0x1a);
    cmds["CSAFE_PM_GET_WORKDISTANCE"] = populateCmd(0xa3, QList<int>(), 0x1a);

    // LIFE FITNESS specific commands
    cmds["CSAFE_LF_GET_DETAIL"] = populateCmd(0xd0, QList<int>());

    // Generic long commands
    cmds["CSAFE_SETPROGRAM_CMD"] = populateCmd(0x24, QList<int>() << 1 << 1);
    cmds["CSAFE_SETUSERINFO_CMD"] = populateCmd(0x2B, QList<int>() << 2 << 1 << 1 << 1);
    cmds["CSAFE_SETLEVEL_CMD"] = populateCmd(0x2D, QList<int>() << 1);

    // Generic Short Commands
    cmds["CSAFE_GETSTATUS_CMD"] = populateCmd(0x80, QList<int>());
    cmds["CSAFE_GOINUSE_CMD"] = populateCmd(0x85, QList<int>());
    cmds["CSAFE_GETCALORIES_CMD"] = populateCmd(0xa3, QList<int>());
    cmds["CSAFE_GETPROGRAM_CMD"] = populateCmd(0xA4, QList<int>());
    cmds["CSAFE_GETPACE_CMD"] = populateCmd(0xa6, QList<int>());
    cmds["CSAFE_GETCADENCE_CMD"] = populateCmd(0xa7, QList<int>());
    cmds["CSAFE_GETHORIZONTAL_CMD"] = populateCmd(0xA1, QList<int>());
    cmds["CSAFE_GETSPEED_CMD"] = populateCmd(0xA5, QList<int>());
    cmds["CSAFE_GETUSERINFO_CMD"] = populateCmd(0xAB, QList<int>());
    cmds["CSAFE_GETHRCUR_CMD"] = populateCmd(0xb0, QList<int>());
    cmds["CSAFE_GETPOWER_CMD"] = populateCmd(0xb4, QList<int>());

    // Response Data to Short Commands
    resp[0x80] = qMakePair(QString("CSAFE_GETSTATUS_CMD"), QList<int>() << 0);
    resp[0x81] = qMakePair(QString("CSAFE_RESET_CMD"), QList<int>() << 0);
    resp[0x82] = qMakePair(QString("CSAFE_GOIDLE_CMD"), QList<int>() << 0);
    resp[0x83] = qMakePair(QString("CSAFE_GOHAVEID_CMD"), QList<int>() << 0);
    resp[0x85] = qMakePair(QString("CSAFE_GOINUSE_CMD"), QList<int>() << 0);
    resp[0x86] = qMakePair(QString("CSAFE_GOFINISHED_CMD"), QList<int>() << 0);
    resp[0x87] = qMakePair(QString("CSAFE_GOREADY_CMD"), QList<int>() << 0);
    resp[0x88] = qMakePair(QString("CSAFE_BADID_CMD"), QList<int>() << 0);
    resp[0x91] = qMakePair(QString("CSAFE_GETVERSION_CMD"), QList<int>() << 1 << 1 << 1 << 2 << 2);
    resp[0x92] = qMakePair(QString("CSAFE_GETID_CMD"), QList<int>() << -5);
    resp[0x93] = qMakePair(QString("CSAFE_GETUNITS_CMD"), QList<int>() << 1);
    resp[0x94] = qMakePair(QString("CSAFE_GETSERIAL_CMD"), QList<int>() << -9);
    resp[0x9B] = qMakePair(QString("CSAFE_GETODOMETER_CMD"), QList<int>() << 4 << 1);
    resp[0x9C] = qMakePair(QString("CSAFE_GETERRORCODE_CMD"), QList<int>() << 3);
    resp[0xA0] = qMakePair(QString("CSAFE_GETTWORK_CMD"), QList<int>() << 1 << 1 << 1);
    resp[0xA1] = qMakePair(QString("CSAFE_GETHORIZONTAL_CMD"), QList<int>() << 2 << 1);
    resp[0xA3] = qMakePair(QString("CSAFE_GETCALORIES_CMD"), QList<int>() << 2);
    resp[0xA4] = qMakePair(QString("CSAFE_GETPROGRAM_CMD"), QList<int>() << 1 << 1);
    resp[0xA5] = qMakePair(QString("CSAFE_GETSPEED_CMD"), QList<int>() << 2 << 1);
    resp[0xA6] = qMakePair(QString("CSAFE_GETPACE_CMD"), QList<int>() << 2 << 1);
    resp[0xA7] = qMakePair(QString("CSAFE_GETCADENCE_CMD"), QList<int>() << 2 << 1);
    resp[0xAB] = qMakePair(QString("CSAFE_GETUSERINFO_CMD"), QList<int>() << 2 << 1 << 1 << 1);
    resp[0xB0] = qMakePair(QString("CSAFE_GETHRCUR_CMD"), QList<int>() << 1);
    resp[0xB4] = qMakePair(QString("CSAFE_GETPOWER_CMD"), QList<int>() << 2 << 1);

    // Response Data to Long Commands
    resp[0x01] = qMakePair(QString("CSAFE_AUTOUPLOAD_CMD"), QList<int>() << 0);
    resp[0x10] = qMakePair(QString("CSAFE_IDDIGITS_CMD"), QList<int>() << 0);
    resp[0x11] = qMakePair(QString("CSAFE_SETTIME_CMD"), QList<int>() << 0);
    resp[0x12] = qMakePair(QString("CSAFE_SETDATE_CMD"), QList<int>() << 0);
    resp[0x13] = qMakePair(QString("CSAFE_SETTIMEOUT_CMD"), QList<int>() << 0);
    resp[0x1A] = qMakePair(QString("CSAFE_SETUSERCFG1_CMD"), QList<int>() << 0);
    resp[0x20] = qMakePair(QString("CSAFE_SETTWORK_CMD"), QList<int>() << 0);
    resp[0x21] = qMakePair(QString("CSAFE_SETHORIZONTAL_CMD"), QList<int>() << 0);
    resp[0x23] = qMakePair(QString("CSAFE_SETCALORIES_CMD"), QList<int>() << 0);
    resp[0x24] = qMakePair(QString("CSAFE_SETPROGRAM_CMD"), QList<int>() << 0);
    resp[0x2B] = qMakePair(QString("CSAFE_SETUSERINFO_CMD"), QList<int>() << 0);
    resp[0x2D] = qMakePair(QString("CSAFE_SETLEVEL_CMD"), QList<int>() << 0);
    resp[0x34] = qMakePair(QString("CSAFE_SETPOWER_CMD"), QList<int>() << 0);
    resp[0x70] = qMakePair(QString("CSAFE_GETCAPS_CMD"), QList<int>() << 11);

    // Response Data to PM3 Specific Short Commands
    resp[0x1A89] = qMakePair(QString("CSAFE_PM_GET_WORKOUTTYPE"), QList<int>() << 1);
    resp[0x1AC1] = qMakePair(QString("CSAFE_PM_GET_DRAGFACTOR"), QList<int>() << 1);
    resp[0x1ABF] = qMakePair(QString("CSAFE_PM_GET_STROKESTATE"), QList<int>() << 1);
    resp[0x1AA0] = qMakePair(QString("CSAFE_PM_GET_WORKTIME"), QList<int>() << 4 << 1);
    resp[0x1AA3] = qMakePair(QString("CSAFE_PM_GET_WORKDISTANCE"), QList<int>() << 4 << 1);
    resp[0x1AC9] = qMakePair(QString("CSAFE_PM_GET_ERRORVALUE"), QList<int>() << 2);
    resp[0x1A8D] = qMakePair(QString("CSAFE_PM_GET_WORKOUTSTATE"), QList<int>() << 1);
    resp[0x1A9F] = qMakePair(QString("CSAFE_PM_GET_WORKOUTINTERVALCOUNT"), QList<int>() << 1);
    resp[0x1A8E] = qMakePair(QString("CSAFE_PM_GET_INTERVALTYPE"), QList<int>() << 1);
    resp[0x1ACF] = qMakePair(QString("CSAFE_PM_GET_RESTTIME"), QList<int>() << 2);

    // Response Data to PM3 Specific Long Commands
    resp[0x1A05] = qMakePair(QString("CSAFE_PM_SET_SPLITDURATION"), QList<int>() << 0);
    resp[0x1A6B] =
        qMakePair(QString("CSAFE_PM_GET_FORCEPLOTDATA"),
                  QList<int>() << 1 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2);
    resp[0x1A27] = qMakePair(QString("CSAFE_PM_SET_SCREENERRORMODE"), QList<int>() << 0);
    resp[0x1A6C] =
        qMakePair(QString("CSAFE_PM_GET_HEARTBEATDATA"),
                  QList<int>() << 1 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2 << 2);

    // LIFE FITNESS specific response
    resp[0xD0] = qMakePair(QString("CSAFE_LF_GET_DETAIL"), QList<int>() << 0x1c);
}

QList<QList<int>> csafe::populateCmd(int First, QList<int> Second, int Third) {
    QList<QList<int>> ret;
    QList<int> first;
    QList<int> second;
    QList<int> third;
    first.clear();
    second.clear();
    third.clear();
    first.append(First);
    foreach (int a, Second) {
        second.append(a);
    }
    third.append(Third);
    ret.append(first);
    ret.append(second);
    ret.append(third);
    return ret;
}

QList<QList<int>> csafe::populateCmd(int First, QList<int> Second) {
    QList<QList<int>> ret;
    QList<int> first;
    QList<int> second;
    first.clear();
    second.clear();
    first.append(First);
    foreach (int a, Second) {
        second.append(a);
    }
    ret.append(first);
    ret.append(second);
    return ret;
}

QVector<quint8> csafe::int2bytes(int numbytes, int integer) {
    if (!(0 <= integer && integer <= (1 << (8 * numbytes)))) {
        qWarning("Integer is outside the allowable range");
    }

    QVector<quint8> byte;
    for (int k = 0; k < numbytes; ++k) {
        int calcbyte = (integer >> (8 * k)) & 0xFF;
        byte.append(calcbyte);
    }

    return byte;
}

int csafe::bytes2int(const QVector<quint8> &raw_bytes) {
    int num_bytes = raw_bytes.size();
    int integer = 0;

    for (int k = 0; k < num_bytes; ++k) {
        integer = (raw_bytes[k] << (8 * k)) | integer;
    }

    return integer;
}

QString csafe::bytes2ascii(const QVector<quint8> &raw_bytes) {
    QString word;
    for (quint8 letter : raw_bytes) {
        word += QChar(letter);
    }

    return word;
}

QByteArray csafe::write(const QStringList &arguments, bool surround_msg) {
    int i = 0;
    QVector<quint8> message;
    int wrapper = 0;
    QVector<quint8> wrapped;
    int maxresponse = 3; // start & stop flag & status

    while (i < arguments.size()) {
        QString arg = arguments[i];
        if (!cmds.contains(arg)) {
            qWarning("CSAFE Command not implemented: %s", qPrintable(arg));
            return QByteArray();
        }
        const auto &cmdprop = cmds[arg];
        QVector<quint8> command;

        if (cmdprop.at(1).size() != 0) // Long Command
        {
            for (int a = 0; a < (int)cmdprop.at(1).size(); a++) {
                ++i;
                int intvalue = arguments[i].toInt();
                QVector<quint8> value = int2bytes(cmdprop.at(1).at(a), intvalue);
                command.append(value);
            }

            int cmdbytes = command.size();
            command.prepend(cmdbytes); // data byte count
        }

        command.prepend(cmdprop.at(0).at(0)); // add command id

        if (!wrapped.isEmpty() && (cmdprop.size() < 3 || cmdprop.at(2).at(0) != wrapper)) {
            wrapped.prepend(wrapped.size()); // data byte count for wrapper
            wrapped.prepend(wrapper);        // wrapper command id
            message.append(wrapped);         // adds wrapper to message
            wrapped.clear();
            wrapper = 0;
        }

        if (cmdprop.size() == 3) // checks if command needs a wrapper
        {
            if (wrapper == cmdprop.at(2).at(0)) // checks if currently in the same wrapper
            {
                wrapped.append(command);
            } else // creating a new wrapper
            {
                wrapped = command;
                wrapper = cmdprop.at(2).at(0);
                maxresponse += 2;
            }

            command.clear(); // clear command to prevent it from getting into message
        }

        int cmdid = cmdprop[0].at(0) | (wrapper << 8); // max message length
        int sum = 0;
        for (int aa = 0; aa < resp[cmdid].second.size(); aa++) {
            sum += resp[cmdid].second.at(aa);
        }
        maxresponse += qAbs(sum) * 2 + 1; // double return to account for stuffing

        message.append(command); // add completed command to final message

        ++i;
    }

    if (!wrapped.isEmpty()) // closes wrapper if message ended on it
    {
        wrapped.prepend(wrapped.size()); // data byte count for wrapper
        wrapped.prepend(wrapper);        // wrapper command id
        message.append(wrapped);         // adds wrapper to message
    }

    int checksum = 0;
    int j = 0;

    while (j < message.size()) // checksum and byte stuffing
    {
        checksum ^= message[j]; // calculate checksum

        if (0xF0 <= message[j] && message[j] <= 0xF3) // byte stuffing
        {
            message.insert(j, Byte_Stuffing_Flag);
            ++j;
            message[j] = message[j] & 0x3;
        }

        ++j;
    }

    message.append(checksum);                   // add checksum to end of message
    message.prepend(Standard_Frame_Start_Flag); // start frame
    message.append(Stop_Frame_Flag);            // stop frame

    if (message.size() > 96) // check for frame size (96 bytes)
    {
        qWarning("Message is too long: %d", message.size());
    }

    if (surround_msg) {                                         // apply non-standard wrapping for PM3 rower
        int maxmessage = qMax(message.size() + 1, maxresponse); // report IDs

        if (maxmessage <= 21) {
            message.prepend(0x01);
            message.append(QVector<quint8>(21 - message.size()));
        } else if (maxmessage <= 63) {
            message.prepend(0x04);
            message.append(QVector<quint8>(63 - message.size()));
        } else if ((message.size() + 1) <= 121) {
            message.prepend(0x02);
            message.append(QVector<quint8>(121 - message.size()));
            if (maxresponse > 121) {
                qWarning("Response may be too long to receive. Max possible length: %d", maxresponse);
            }
        } else {
            qWarning("Message too long. Message length: %d", message.size());
            message.clear();
        }
    }

    QByteArray ret;
    foreach (int a, message) {
        ret.append(a);
    }
    return ret;
}

QVector<quint8> csafe::check_message(QVector<quint8> message) {
    int i = 0;
    int checksum = 0;

    while (i < message.size()) // checksum and unstuff
    {
        if (message[i] == Byte_Stuffing_Flag) // byte unstuffing
        {
            quint8 stuffvalue = message.takeAt(i + 1);
            message[i] = 0xF0 | stuffvalue;
        }
        checksum ^= message[i]; // calculate checksum
        ++i;
    }

    if (checksum != 0) // checks checksum
    {
        qWarning("Checksum error");
        message.clear();
    } else {
        message.removeLast(); // remove checksum from end of message
    }

    return message;
}

QVariantMap csafe::read(const QVector<quint8> &transmission) {
    QVector<quint8> message;
    bool stopfound = false;
    int j = 0;
    while (j < transmission.size()) {
        int startflag = transmission[j];
        if (startflag == Extended_Frame_Start_Flag) {
            j = j + 3;
            break;
        } else if (startflag == Standard_Frame_Start_Flag) {
            ++j;
            break;
        } else {
            ++j;
        }
    }

    if (j >= transmission.size()) {
        qWarning("No Start Flag found.");
        return QVariantMap();
    }

    while (j < transmission.size()) {
        if (transmission[j] == Stop_Frame_Flag) {
            stopfound = true;
            break;
        }
        message.append(transmission[j]);
        ++j;
    }

    if (!stopfound) {
        qWarning("No Stop Flag found.");
        return QVariantMap();
    }
    message = check_message(message);
    int status = message.takeFirst();

    QVariantMap response;
    response["CSAFE_GETSTATUS_CMD"] = QVariantList() << status;

    int k = 0;
    int wrapend = -1;
    int wrapper = 0x0;

    while (k < message.size()) // loop through complete frames
    {
        QVariantList result;

        int msgcmd = message[k];
        if (k <= wrapend) {
            msgcmd = wrapper | msgcmd;
        }
        auto &msgprop = resp[msgcmd];
        ++k;

        int bytecount = message[k];
        ++k;

        if (msgprop.first == "CSAFE_SETUSERCFG1_CMD") // if wrapper command
        {
            wrapper = message[k - 2] << 8;
            wrapend = k + bytecount - 1;
            if (bytecount != 0) {
                msgcmd = wrapper | message[k];
                const auto &wrapper_msgprop = resp[msgcmd];
                msgprop.second = wrapper_msgprop.second;
                ++k;
                bytecount = message[k];
                ++k;
            }
        }

        if (msgprop.first == "CSAFE_GETCAPS_CMD") // special case for capability code
        {
            // msgprop.second = QList<int>(bytecount, 1);
        }

        if (msgprop.first == "CSAFE_GETID_CMD") // special case for get id
        {
            // msgprop.second = QList<int>(1, -bytecount);
        }

        if (abs(std::accumulate(msgprop.second.begin(), msgprop.second.end(), 0)) != 0 &&
            bytecount != abs(std::accumulate(msgprop.second.begin(), msgprop.second.end(), 0))) {
            qWarning("Warning: bytecount is an unexpected length");
        }

        for (int numbytes : msgprop.second) // extract values
        {
            QVector<quint8> raw_bytes = message.mid(k, abs(numbytes));
            QVariant value;
            if (numbytes >= 0) {
                value = bytes2int(raw_bytes);
            } else {
                value = bytes2ascii(raw_bytes);
            }
            result.append(value);
            k += abs(numbytes);
        }

        response[msgprop.first] = result;
    }

    return response;
}
