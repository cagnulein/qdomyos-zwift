#ifndef PROFILETOKENSTORE_H
#define PROFILETOKENSTORE_H

#include <QSettings>
#include <QStringList>

// Small, provider-agnostic helper for credentials that belong to a profile
// while the legacy unscoped keys are still needed by older code/configurations.
class ProfileTokenStore
{
public:
    static QString scopedKey(const QString &baseKey, const QString &userId)
    {
        return userId.isEmpty() ? baseKey : baseKey + QStringLiteral("_") + userId;
    }

    static QVariant value(const QSettings &settings, const QString &baseKey,
                          const QString &userId, const QVariant &defaultValue = QVariant())
    {
        return settings.value(scopedKey(baseKey, userId), defaultValue);
    }

    static bool hasAnyValue(const QSettings &settings, const QStringList &baseKeys,
                            const QString &userId)
    {
        if (userId.isEmpty())
            return false;

        for (const QString &baseKey : baseKeys) {
            if (!value(settings, baseKey, userId).toString().isEmpty())
                return true;
        }
        return false;
    }

    static void save(QSettings &settings, const QString &baseKey, const QVariant &valueToSave,
                     const QString &userId, bool mirrorLegacy)
    {
        settings.setValue(scopedKey(baseKey, userId), valueToSave);
        if (mirrorLegacy)
            settings.setValue(baseKey, valueToSave);
    }

    static void remove(QSettings &settings, const QString &baseKey, const QString &userId,
                       bool removeLegacy)
    {
        settings.remove(scopedKey(baseKey, userId));
        if (removeLegacy)
            settings.remove(baseKey);
    }
};

#endif // PROFILETOKENSTORE_H
