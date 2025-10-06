//
//  adb_auth.c
//  qdomyoszwift
//
//  Created by Roberto Viola on 02/04/24.
//

#include <stdio.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include "windows.h"
#  include "shlobj.h"
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif
#include <string.h>

#include "sysdeps.h"
#include "adb.h"
#include "adb_auth.h"

#if ADB_HOST


int adb_auth_generate_token(void *token, size_t token_size) {return 0;}
int adb_auth_verify(void *token, void *sig, int siglen) { return 0;}
void adb_auth_confirm_key(unsigned char *data, size_t len, atransport *t){}

#else // !ADB_HOST

int adb_auth_sign(void* key, void *token, size_t token_size, void *sig) { return 0; }
void *adb_auth_nextkey(void *current) { return NULL; }
int adb_auth_get_userkey(unsigned char *data, size_t len) { return 0; }


#endif // ADB_HOST
