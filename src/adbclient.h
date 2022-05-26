// -*- mode: c++ -*-
#ifndef ADBCLIENT_H
#define ADBCLIENT_H
#include <QString>
#include <QStringList>
#include <QTcpSocket>

extern const char *__adb_serial;

#define htoll(x) (x)
#define ltohl(x) (x)
#define MKID(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define ID_STAT MKID('S', 'T', 'A', 'T')
#define ID_LIST MKID('L', 'I', 'S', 'T')
#define ID_ULNK MKID('U', 'L', 'N', 'K')
#define ID_SEND MKID('S', 'E', 'N', 'D')
#define ID_RECV MKID('R', 'E', 'C', 'V')
#define ID_DENT MKID('D', 'E', 'N', 'T')
#define ID_DONE MKID('D', 'O', 'N', 'E')
#define ID_DATA MKID('D', 'A', 'T', 'A')
#define ID_OKAY MKID('O', 'K', 'A', 'Y')
#define ID_FAIL MKID('F', 'A', 'I', 'L')
#define ID_QUIT MKID('Q', 'U', 'I', 'T')

typedef union {
    quint32 id;
    struct {
        quint32 id;
        quint32 namelen;
    } req;
    struct {
        quint32 id;
        quint32 mode;
        quint32 size;
        quint32 time;
    } stat;
    struct {
        quint32 id;
        quint32 mode;
        quint32 size;
        quint32 time;
        quint32 namelen;
    } dent;
    struct {
        quint32 id;
        quint32 size;
    } data;
    struct {
        quint32 id;
        quint32 msglen;
    } status;
} syncmsg;

#define S_IFMT 00170000
#define S_IFSOCK 0140000
#define S_IFLNK 0120000
#define S_IFREG 0100000
#define S_IFBLK 0060000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFIFO 0010000
#define S_ISUID 0004000
#define S_ISGID 0002000
#define S_ISVTX 0001000

#define S_ISLNK(m) (((m)&S_IFMT) == S_IFLNK)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m)&S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m)&S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m)&S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m)&S_IFMT) == S_IFSOCK)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define SYNC_DATA_MAX (64 * 1024)

typedef struct syncsendbuf syncsendbuf;

struct syncsendbuf {
    quint32 id;
    quint32 size;
    char data[SYNC_DATA_MAX];
};

bool _writex(QIODevice &io, const void *data, qint64 max);
QString adb_quote_shell(const QStringList &args);

class AdbClient {
  private:
    syncsendbuf send_buffer;

    QTcpSocket adbSock;
    bool switch_socket_transport();
    bool write_data_buffer(char *file_buffer, int size, syncsendbuf *sbuf);
    bool write_data_file(const QString &path, syncsendbuf *sbuf);
    bool writex(const void *data, qint64 max);
    bool readx(void *data, qint64 max);
    void sync_quit();
    bool sync_recv(const QString &rpath, const QString &lpath);
    bool do_sync_pull(const char *rpath, const char *lpath);
    void adb_close();
    bool adb_status();
    bool do_sync_push(const char *lpath, const char *rpath);
    QString __adb_error;
    bool adb_connect(const char *service);
    bool isOK;
    bool sync_readmode(const char *path, quint32 *mode);

    bool sync_send(const QString &lpath, const QString &rpath, unsigned mtime, quint32 mode);

  public:
    QTcpSocket *getSock() { return &adbSock; };
    AdbClient(QString ip);
    ~AdbClient();

    QString adb_error() { return __adb_error; };

    static QString doAdbShell(const QStringList &cmdAndArgs, QString ip);
    static QString doAdbShell(const QString &cmdLine, QString ip);
    static AdbClient *doAdbPipe(const QStringList &cmdAndArgs, QString ip);
    static AdbClient *doAdbPipe(const QString &cmdLine, QString ip);

    static bool doAdbPull(const QString &rptah, const QString &lpath, QString ip);
    static bool doAdbPush(const QString &lpath, const QString &rpath, QString ip);
    static int doAdbKill(QString ip);
    static int doAdbForward(const QString &forwardSpec, QString ip);
};

#endif // ADBCLIENT_H
