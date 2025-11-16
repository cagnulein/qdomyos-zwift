/******************************************************************************
**
** Copyright (C) 2017 Lorenz Haas
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "qmqtttopicfilter.h"
#include <QStringView>
#include <QtCore/QDebug>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

/*!
    \class QMqttTopicFilter
    \inmodule QtMqtt
    \reentrant
    \ingroup shared

    \brief The QMqttTopicFilter class represents a MQTT topic filter.

    QMqttTopicFilter is a thin wrapper around a QString providing an expressive
    data type for MQTT topic filters. Beside the benefits of having a strong
    type preventing unintended misuse, QMqttTopicFilter provides convenient
    functions related to topic filters like isValid() or match().

    For example, the following code would fail to compile and prevent a possible
    unintended and meaningless matching of two filters, especially if the
    variable names were less expressive:

    \code
    QMqttTopicFilter globalFilter{"foo/#"};
    QMqttTopicFilter specificFilter{"foo/bar"};
    if (globalFilter.match(specificFilter)) {
        //...
    }
    \endcode

    The usability, however, is not affected since the following snippet compiles
    and runs as expected:

    \code
    QMqttTopicFilter globalFilter{"foo/#"};
    if (globalFilter.match("foo/bar")) {
        //...
    }
    \endcode

    \sa QMqttTopicName
 */

/*!
    \fn void QMqttTopicFilter::swap(QMqttTopicFilter &other)
    Swaps the MQTT topic filter \a other with this MQTT topic filter. This
    operation is very fast and never fails.
 */

/*!
    \enum QMqttTopicFilter::MatchOption

    This enum value holds the matching options for the topic filter.

    \value NoMatchOption
        No match options are set.
    \value WildcardsDontMatchDollarTopicMatchOption
        A wildcard at the filter's beginning does not match a topic name that
        starts with the dollar sign ($).
 */

class QMqttTopicFilterPrivate : public QSharedData
{
public:
    QString filter;
};

/*!
    Creates a new MQTT topic filter with the specified \a filter.
 */
QMqttTopicFilter::QMqttTopicFilter(const QString &filter) : d(new QMqttTopicFilterPrivate)
{
    d->filter = filter;
}

/*!
    Creates a new MQTT topic filter with the specified \a filter.
 */
QMqttTopicFilter::QMqttTopicFilter(const QLatin1String &filter) : d(new QMqttTopicFilterPrivate)
{
    d->filter = filter;
}

/*!
    Creates a new MQTT topic filter as a copy of \a filter.
 */
QMqttTopicFilter::QMqttTopicFilter(const QMqttTopicFilter &filter) : d(filter.d)
{
}

/*!
    Destroys the QMqttTopicFilter object.
 */
QMqttTopicFilter::~QMqttTopicFilter()
{
}

/*!
    Assigns the MQTT topic filter \a filter to this object, and returns a
    reference to the copy.
 */
QMqttTopicFilter &QMqttTopicFilter::operator=(const QMqttTopicFilter &filter)
{
    d = filter.d;
    return *this;
}

/*!
    Returns the topic filter.
 */
QString QMqttTopicFilter::filter() const
{
    return d->filter;
}

/*!
    Sets the topic filter to \a filter.
 */
void QMqttTopicFilter::setFilter(const QString &filter)
{
    d.detach();
    d->filter = filter;
}

/*!
    \since 5.12

    Returns the name of a share if the topic filter has been specified as
    a shared subscription. The format of shared subscriptions is defined
    as \c $share/sharename/topicfilter.
*/
QString QMqttTopicFilter::sharedSubscriptionName() const
{
    QString result;
    if (d->filter.startsWith(QLatin1String("$share/"))) {
        // Has to have at least two /
        // $share/<sharename>/topicfilter
        result = d->filter.section(QLatin1Char('/'), 1, 1);
    }
    return result;
}

/*!
    Returns \c true if the topic filter is valid according to the MQTT standard
    section 4.7, or \c false otherwise.
 */
bool QMqttTopicFilter::isValid() const
{
    // MQTT-4.7.3-1, MQTT-4.7.3-3, and MQTT-4.7.3-2
    const int size = d->filter.size();
    if (size == 0 || size > 65535 || d->filter.contains(QChar(QChar::Null)))
        return false;

    if (size == 1)
        return true;

    // '#' MUST be last and its own level. It MUST NOT appear more than at most once.
    const int multiLevelPosition = d->filter.indexOf(QLatin1Char('#'));
    if (multiLevelPosition != -1
        && (multiLevelPosition != size - 1 || d->filter.at(size-2) != QLatin1Char('/'))) {
        return false;
    }

    // '+' MAY occur multiple times but MUST be its own level.
    int singleLevelPosition = d->filter.indexOf(QLatin1Char('+'));
    while (singleLevelPosition != -1) {
        if ((singleLevelPosition != 0 && d->filter.at(singleLevelPosition - 1) != QLatin1Char('/'))
            || (singleLevelPosition < size - 1 && d->filter.at(singleLevelPosition + 1) != QLatin1Char('/'))) {
            return false;
        }
        singleLevelPosition = d->filter.indexOf(QLatin1Char('#'), singleLevelPosition + 1);
    }

    // Shared Subscription syntax
    // $share/shareName/TopicFilter -- must have at least 2 '/'
    if (d->filter.startsWith(QLatin1String("$share/"))) {
        const int index = d->filter.indexOf(QLatin1Char('/'), 7);
        if (index == -1 || index == 7)
            return false;
    }
    return true;
}

/*!
    Returns \c true if the topic filter matches the topic name \a name
    honoring the given \a matchOptions, or \c false otherwise.
 */
bool QMqttTopicFilter::match(const QMqttTopicName &name, MatchOptions matchOptions) const
{
    if (!name.isValid() || !isValid())
        return false;

    const QString topic = name.name();
    if (topic == d->filter)
        return true;

    if (matchOptions.testFlag(WildcardsDontMatchDollarTopicMatchOption)
        && topic.startsWith(QLatin1Char('$'))
        && (d->filter.startsWith(QLatin1Char('+'))
            || d->filter == QLatin1Char('#')
            || d->filter == QLatin1String("/#"))) {
        return false;
    }

    if (d->filter.endsWith(QLatin1Char('#'))) {
        QStringView root = QStringView(d->filter).left(d->filter.size() - 1);
        if (root.endsWith(QLatin1Char('/'))) // '#' also represents the parent level!
            root = root.left(root.size() - 1);
        return topic.startsWith(root);
    }

    if (d->filter.contains(QLatin1Char('+'))) {
        const QVector<QStringView> filterLevels = QStringView(d->filter).split(QLatin1Char('/'));
        const QVector<QStringView> topicLevels = QStringView(topic).split(QLatin1Char('/'));
        if (filterLevels.size() != topicLevels.size())
            return false;
        for (int i = 0; i < filterLevels.size(); ++i) {
            const QStringView &level = filterLevels.at(i);
            if (level != QLatin1Char('+') && level != topicLevels.at(i))
                return false;
        }
        return true;
    }

    return false;
}

/*!
    \relates QMqttTopicFilter

    Returns \c true if the topic filters \a lhs and \a rhs are equal,
    otherwise returns \c false.
 */
bool operator==(const QMqttTopicFilter &lhs, const QMqttTopicFilter &rhs) Q_DECL_NOTHROW
{
    return (lhs.d == rhs.d) || (lhs.d->filter == rhs.d->filter);
}

/*!
    \fn bool operator!=(const QMqttTopicFilter &lhs, const QMqttTopicFilter &rhs)
    \relates QMqttTopicFilter

    Returns \c true if the topic filters \a lhs and \a rhs are different,
    otherwise returns \c false.
 */

/*!
    \relates QMqttTopicFilter

    Returns \c true if the topic filter \a lhs is lexically less than the topic
    filter \a rhs; otherwise returns \c false.
 */
bool operator<(const QMqttTopicFilter &lhs, const QMqttTopicFilter &rhs) Q_DECL_NOTHROW
{
    return lhs.d->filter < rhs.d->filter;
}

/*!
    \relates QHash

    Returns the hash value for \a filter. If specified, \a seed is used to
    initialize the hash.
*/
uint qHash(const QMqttTopicFilter &filter, uint seed) Q_DECL_NOTHROW
{
    return qHash(filter.d->filter, static_cast<size_t>(seed));
}

#ifndef QT_NO_DATASTREAM
/*! \relates QMqttTopicFilter

    Writes the topic filter \a filter to the stream \a out and returns a
    reference to the stream.

    \sa{Serializing Qt Data Types}{Format of the QDataStream operators}
*/
QDataStream &operator<<(QDataStream &out, const QMqttTopicFilter &filter)
{
    out << filter.filter();
    return out;
}

/*! \relates QMqttTopicFilter

    Reads a topic filter into \a filter from the stream \a in and returns a
    reference to the stream.

    \sa{Serializing Qt Data Types}{Format of the QDataStream operators}
*/
QDataStream &operator>>(QDataStream &in, QMqttTopicFilter &filter)
{
    QString f;
    in >> f;
    filter.setFilter(f);
    return in;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QMqttTopicFilter &filter)
{
    QDebugStateSaver saver(d);
    d.nospace() << "QMqttTopicFilter(" << filter.filter() << ')';
    return d;
}
#endif

QT_END_NAMESPACE
