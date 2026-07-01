#include <QDateTime>
#include <QString>

// Global log filename, initialized once at startup with a timestamp-based name.
// Defined here (not in main.cpp) so that libqdomyos-zwift can reference it
// without pulling in main.o (which defines main()) and causing linker conflicts
// in the test binary.
QString logfilename = QStringLiteral("debug-") +
                      QDateTime::currentDateTime()
                          .toString()
                          .replace(QStringLiteral(":"), QStringLiteral("_"))
                          .replace(QStringLiteral(" "), QStringLiteral("_"))
                          .replace(QStringLiteral("."), QStringLiteral("_")) +
                      QStringLiteral(".log");
