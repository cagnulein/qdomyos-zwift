// Simulator-only ADB authentication symbols. The real implementation uses
// the device-only OpenSSL archive and is excluded from the simulator target.
#include <stddef.h>

int adb_auth_sign(void *key, void *token, size_t token_size, void *sig)
{
    (void)key;
    (void)token;
    (void)token_size;
    (void)sig;
    return 0;
}

void *adb_auth_nextkey(void *current)
{
    (void)current;
    return 0;
}

int adb_auth_get_userkey(unsigned char *data, size_t len)
{
    (void)data;
    (void)len;
    return 0;
}

void adb_auth_init(void)
{
}
