#ifndef CSERIAL_PORT_H
#define CSERIAL_PORT_H

// Define this to 1 to log bytes in an internal circular buffer for debug purposes
#define CSERIALPORT_LOG_BYTES	0

// NOTE: Currently works only with default port COM3 due to 1200 baud workaround
class CSerialPort
{
public:
	CSerialPort(void):
	    m_hSerialPortIn(INVALID_HANDLE_VALUE),
	    m_hSerialPortOut(INVALID_HANDLE_VALUE)
	{
	}

	virtual ~CSerialPort(void);

    virtual SnBool Open(LPCTSTR lpComPort, DWORD BaudRate);
	virtual SnBool Close(void);
	virtual SnBool Write(SnByte bVal
#if CSERIALPORT_LOG_BYTES
		,int iTrace=0
#endif
		);
	virtual SnBool Write(SnByte *bVal, SnByte bSize
#if CSERIALPORT_LOG_BYTES
		,int iTrace=0
#endif
		);
	virtual SnBool Read(SnByte* pbVal
#if CSERIALPORT_LOG_BYTES
		,int iTrace=0
#endif
		);

protected:
	HANDLE m_hSerialPortIn;
	HANDLE m_hSerialPortOut;

private:
	SnBool EnableTx(void);	
		//Private copy constructor
	CSerialPort(const CSerialPort& oOld)
	{
	}
};

#endif