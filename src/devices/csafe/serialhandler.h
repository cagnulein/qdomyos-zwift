/*
 * Copyright (c) 2024 Marcel Verpaalen (marcel@verpaalen.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This emulates a serial port over a network connection.
 * e.g. as created by ser2net or hardware serial to ethernet converters
 *
 */
#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <sys/types.h>
#include <QString>
#include <QDebug>

/**
 * @brief  This is a parent class for serial port handlers
 * It defines the common interface for serial operations over physical serial port or network
 */
class SerialHandler {
  public:
    virtual ~SerialHandler() = 0; // Pure virtual destructor

    // Factory method to create the appropriate serial handler
    static SerialHandler *create(const QString &deviceFilename, uint32_t baudRate);

    // Abstract interface for serial operations
    virtual int openPort() = 0;
    virtual int closePort() = 0;
    virtual int rawWrite(uint8_t *bytes, int size) = 0;
    virtual int rawRead(uint8_t bytes[], int size, bool line = false) = 0;
    virtual int dataAvailable() = 0;
    virtual bool isOpen() const = 0;

    // Common configuration methods
    virtual void setDevice(const QString &devname) = 0;
    virtual void setTimeout(int timeout) = 0;
    virtual void setEndChar(uint8_t endChar) = 0;

  protected:
    // Protected constructor to prevent direct instantiation of this abstract class
    SerialHandler() = default;
};

inline SerialHandler::~SerialHandler() {} // Definition of the pure virtual destructor

#endif // SERIALHANDLER_H
