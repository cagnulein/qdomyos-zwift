#include "filesearcher.h"
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>
#include <QVariantMap>

FileSearcher::FileSearcher(QObject *parent)
    : QObject(parent) {
}

QVariantList FileSearcher::searchRecursively(const QString &basePath,
                                              const QString &filterPattern,
                                              const QStringList &nameFilters) {
    QVariantList results;

    // Convert base path from URL if needed
    QString cleanBasePath = basePath;
    if (cleanBasePath.startsWith("file://")) {
        cleanBasePath = QUrl(cleanBasePath).toLocalFile();
    }

    // Verify base path exists
    QDir baseDir(cleanBasePath);
    if (!baseDir.exists()) {
        qWarning() << "FileSearcher: Base path does not exist:" << cleanBasePath;
        emit searchCompleted(0);
        return results;
    }

    // Convert filter pattern to lowercase for case-insensitive matching
    QString lowerFilterPattern = filterPattern.toLower();

    qDebug() << "FileSearcher: Starting recursive search in" << cleanBasePath
             << "with pattern:" << filterPattern;

    // Start recursive search
    searchDirectory(cleanBasePath, cleanBasePath, lowerFilterPattern, nameFilters, results);

    qDebug() << "FileSearcher: Search completed, found" << results.size() << "files";
    emit searchCompleted(results.size());

    return results;
}

void FileSearcher::searchDirectory(const QString &dirPath,
                                    const QString &basePath,
                                    const QString &filterPattern,
                                    const QStringList &nameFilters,
                                    QVariantList &results) {
    QDir dir(dirPath);

    // Set name filters for file extensions
    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    // Process files in current directory
    QFileInfoList files = dir.entryInfoList();
    for (const QFileInfo &fileInfo : files) {
        QString fileName = fileInfo.fileName();

        // Check if filename matches filter pattern (case-insensitive)
        if (filterPattern.isEmpty() || fileName.toLower().contains(filterPattern)) {
            // Calculate relative path
            QString absolutePath = fileInfo.absoluteFilePath();
            QString relativePath = absolutePath;
            if (relativePath.startsWith(basePath)) {
                relativePath = relativePath.mid(basePath.length());
                if (relativePath.startsWith("/")) {
                    relativePath = relativePath.mid(1);
                }
            }

            // Create result entry
            QVariantMap resultEntry;
            resultEntry["fileName"] = fileName;
            resultEntry["filePath"] = QUrl::fromLocalFile(absolutePath).toString();
            resultEntry["relativePath"] = relativePath;
            resultEntry["isFolder"] = false;

            results.append(resultEntry);
        }
    }

    // Recursively process subdirectories
    dir.setNameFilters(QStringList());
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList subDirs = dir.entryInfoList();
    for (const QFileInfo &subDirInfo : subDirs) {
        searchDirectory(subDirInfo.absoluteFilePath(), basePath, filterPattern, nameFilters, results);
    }
}
