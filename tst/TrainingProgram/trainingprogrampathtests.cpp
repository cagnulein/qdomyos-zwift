#include "gtest/gtest.h"
#include "trainingprogrampath.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

namespace {
QString writeFile(const QString &path) {
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return QString();
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return QString();
    }
    file.write("<rows/>");
    file.close();
    return path;
}
} // namespace

TEST(TrainingProgramPath, ExistingAbsolutePathIsKeptOnEveryPlatform) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString existing = writeFile(temp.filePath(QStringLiteral("Documents/training/Ride/current.xml")));
    ASSERT_FALSE(existing.isEmpty());

    EXPECT_EQ(TrainingProgramPath::resolve(existing, QString(), false), existing);
    EXPECT_EQ(TrainingProgramPath::resolve(existing, temp.filePath(QStringLiteral("Documents")), true), existing);
}

TEST(TrainingProgramPath, RecoversOldIosContainerOnlyWhenEnabled) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString documents = temp.filePath(QStringLiteral("Documents"));
    const QString current = writeFile(documents + QStringLiteral("/training/Ride/Instructor/old.xml"));
    ASSERT_FALSE(current.isEmpty());

    const QString former = QStringLiteral(
        "/private/var/mobile/Containers/Data/Application/OLD-UUID/Documents/training/Ride/Instructor/old.xml");
    EXPECT_TRUE(TrainingProgramPath::resolve(former, documents, false).isEmpty())
        << "Android and desktop must not reinterpret obsolete iOS paths.";
    EXPECT_EQ(TrainingProgramPath::resolve(former, documents, true), QFileInfo(current).canonicalFilePath());
}

TEST(TrainingProgramPath, MissingAndUnrelatedPathsAreNotRebased) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString documents = temp.filePath(QStringLiteral("Documents"));
    const QString oldRoot = QStringLiteral(
        "/private/var/mobile/Containers/Data/Application/OLD-UUID/Documents/");

    EXPECT_TRUE(TrainingProgramPath::resolve(QString(), documents, true).isEmpty());
    EXPECT_TRUE(TrainingProgramPath::resolve(QStringLiteral("training/missing.xml"), documents, true).isEmpty());
    EXPECT_TRUE(TrainingProgramPath::resolve(oldRoot + QStringLiteral("training/missing.xml"), documents, true).isEmpty());
    EXPECT_TRUE(TrainingProgramPath::resolve(oldRoot + QStringLiteral("other.xml"), documents, true).isEmpty());
    EXPECT_TRUE(TrainingProgramPath::resolve(QStringLiteral("/tmp/Documents/training/missing.xml"), documents, true).isEmpty());
}

TEST(TrainingProgramPath, RefusesToEscapeTrainingDirectory) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString documents = temp.filePath(QStringLiteral("Documents"));
    const QString outside = writeFile(documents + QStringLiteral("/other.xml"));
    ASSERT_FALSE(outside.isEmpty());
    ASSERT_TRUE(QDir().mkpath(documents + QStringLiteral("/training")));

    const QString malicious = QStringLiteral(
        "/private/var/mobile/Containers/Data/Application/OLD-UUID/Documents/training/../other.xml");
    EXPECT_TRUE(TrainingProgramPath::resolve(malicious, documents, true).isEmpty());
}
