#ifndef QTELNET_H
#define QTELNET_H

#include <QObject>
#include <qtcpsocket.h>
#include <qsize.h>
#include <QString>

#define IncommingBufferSize	(1500)

class QTelnet : public QTcpSocket
{
Q_OBJECT

public:
	enum SocketStatus
	{
		Disconnected,
		Resolving,		// Resolving host
		Connecting,		// Connecting to host.
		Connected		// Connected to host.
	};

protected:
	enum TelnetStateCodes
	{
		STATE_DATA          = (char)0,
		STATE_IAC           = (char)1,
		STATE_IACSB         = (char)2,
		STATE_IACWILL       = (char)3,
		STATE_IACDO         = (char)4,
		STATE_IACWONT       = (char)5,
		STATE_IACDONT       = (char)6,
		STATE_IACSBIAC      = (char)7,
		STATE_IACSBDATA     = (char)8,
		STATE_IACSBDATAIAC  = (char)9,
		STATE_DATAR         = (char)10,
		STATE_DATAN         = (char)11
	};
	enum TelnetCodes
	{
		// Negociación entrada/salida (cliente<->servidor)
		IAC                 = (char)255,  // Inicia la secuencia para la negociación telnet.
		EOR                 = (char)239,  // Estando en la negociación, End Of Record.
		WILL                = (char)251,  // Estando en la negociación, Acepta el protocolo?
		WONT                = (char)252,  // Estando en la negociación, Acepta el protocolo?
		DO                  = (char)253,  // Estando en la negociación, Protocolo aceptado.
		DONT                = (char)254,  // Estando en la negociación, Protocolo denegado.
		SB                  = (char)250,  // Estando en la negociación, inicia secuencia de sub-negociación.
		SE                  = (char)240,  // Estando en la sub-negociación, fin de sub-negociación.

		// Negociación de salida (cliente->servidor)
		TELOPT_BINARY       = (char)0,    // Estando en la negociación, pide modo binario.
		TELOPT_ECHO         = (char)1,    // Estando en la negociación, pide echo local.
		TELOPT_SGA          = (char)2,    // Estando en la negociación, pide Supress Go Ahead.
		TELOPT_EOR          = (char)25,   // Estando en la negociación, informa End Of Record.
		TELOPT_NAWS         = (char)31,   // Estando en la negociación, Negotiate About Window Size.
		TELOPT_TTYPE        = (char)24    // Estando en la negociación, Terminal Type.
	};
	enum TelnetQualifiers
	{
		TELQUAL_IS          = (char)0,
		TELQUAL_SEND        = (char)1
	};

private:
	static const char IACWILL[2];
	static const char IACWONT[2];
	static const char IACDO[2];
	static const char IACDONT[2];
	static const char IACSB[2];
	static const char IACSE[2];
	static char _sendCodeArray[2];
	static char _arrCRLF[2];
	static char _arrCR[2];

	QSize m_winSize;          // Tamaño de la pantalla en caracteres.
	QSize m_oldWinSize;       // Tamaño de la pantalla que se envió por última vez al server. Para no enviar el mismo dato.
	enum TelnetStateCodes m_negotiationState;
	char m_receivedDX[256];     // What IAC DO(NT) request do we have received already ?
	char m_receivedWX[256];     // What IAC WILL/WONT request do we have received already ?
	char m_sentDX[256];         // What IAC DO/DONT request do we have sent already ?
	char m_sentWX[256];         // What IAC WILL/WONT request do we have sent already ?
	void resetProtocol();

	char m_buffIncoming[IncommingBufferSize];
	char m_buffProcessed[IncommingBufferSize];
	QByteArray m_buffSB;
	int m_actualSB;

	void emitEndOfRecord()              { Q_EMIT(endOfRecord());      }
	void emitEchoLocal(bool bEcho)      { Q_EMIT(echoLocal(bEcho));   }

	void sendTelnetControl(char codigo);
	void handleSB(void);
	void transpose(const char *buf, int iLen);

	void willsReply(char action, char reply);
	void wontsReply(char action, char reply);
	void doesReply(char action, char reply);
	void dontsReply(char action, char reply);

	void sendSB(char code, char *arr, int iLen);
	qint64 doTelnetInProtocol(qint64 buffSize);

public:
	explicit QTelnet(QObject *parent = 0);

	virtual void connectToHost(const QString &host, quint16 port);
	void sendData(const QByteArray &ba);
	void setCustomCRLF(char lf = 13, char cr = 10);
	void setCustomCR(char cr = 10, char cr2 = 0);

	void writeCustomCRLF();
	void writeCustomCR();

	void write(const char c);

	bool isConnected() const;
	bool testBinaryMode() const;
	void setWindSize(QSize s)   {m_winSize = s;}
	void sendWindowSize();

	QString peerInfo()const;

signals:
	void newData(const char *buff, int len);
	void endOfRecord();
	void echoLocal(bool echo);

private slots:
	void socketError(QAbstractSocket::SocketError err);
	void onReadyRead();
};

#endif // QTELNET_H
