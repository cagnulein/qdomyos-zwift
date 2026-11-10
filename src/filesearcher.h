#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include <QObject>
#include <QVariantList>
#include <QString>
#include <QStringList>

/**
 * @brief FileSearcher provides fast recursive file searching functionality for QML
 *
 * This class performs recursive directory scanning in C++ for much better performance
 * compared to QML-based solutions using FolderListModel.
 */
class FileSearcher : public QObject {
    Q_OBJECT

public:
    explicit FileSearcher(QObject *parent = nullptr);

    /**
     * @brief Search recursively for files matching a filter pattern
     * @param basePath The root directory to start searching from
     * @param filterPattern The search pattern (case-insensitive substring match on filename)
     * @param nameFilters File extensions to include (e.g., ["*.xml", "*.zwo"])
     * @return QVariantList containing search results, each with:
     *         - fileName: The file name
     *         - filePath: The full file path (as URL string)
     *         - relativePath: Path relative to basePath
     *         - isFolder: Always false for file results
     */
    Q_INVOKABLE QVariantList searchRecursively(const QString &basePath,
                                                const QString &filterPattern,
                                                const QStringList &nameFilters = QStringList() << "*.xml" << "*.zwo");

signals:
    /**
     * @brief Emitted when search completes
     * @param resultCount Number of files found
     */
    void searchCompleted(int resultCount);

private:
    /**
     * @brief Internal recursive search implementation
     * @param dir Current directory being scanned
     * @param basePath Original search root for calculating relative paths
     * @param filterPattern Search pattern (lowercase)
     * @param nameFilters File extension filters
     * @param results Output list to accumulate results
     */
    void searchDirectory(const QString &dir,
                        const QString &basePath,
                        const QString &filterPattern,
                        const QStringList &nameFilters,
                        QVariantList &results);
};

#endif // FILESEARCHER_H
