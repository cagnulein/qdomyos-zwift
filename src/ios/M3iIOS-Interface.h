#ifndef __M3IOS_INTERFACE_H__
#define __M3IOS_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int rssi;
    char name[50];
    char uuid[100];
    unsigned char bytes[50];
    int nbytes;
    unsigned char  major, minor, idval;
} m3i_result_t;

void m3i_callback(void * objref, m3i_result_t * res);
void qt_log(const void * msg);
#ifdef __cplusplus
}
#endif


class M3iIOS
{
public:
    M3iIOS ( void );
    ~M3iIOS( void );

    void init( void * objref );
    void startScan( m3i_result_t * conf);
    void stopScan();
    bool isScanning() const;
private:
    void * self;
};

#endif
