#ifndef SRP_H
#define SRP_H

#include "util.h"
#include "stdbool.h"

void set_g( mpz_t g );
void set_N( mpz_t N );

void client_set_v( mpz_t salt, unsigned const char * password );
void server_set_v( mpz_t salt, mpz_t v );

/* http://en.wikipedia.org/wiki/Secure_Remote_Password_protocol */
/* http://srp.stanford.edu/doc.html#papers */

void client_init( mpz_t A );
void client_setB( mpz_t B );
void client_K( mpz_t K );
void client_M1( mpz_t retval );
bool client_verifyM2( mpz_t server_M2 );

void server_init( mpz_t B );
void server_setA( mpz_t A );
void server_K( mpz_t K );
void server_M2( mpz_t retval );
bool server_verifyM1( mpz_t client_M1 );

#endif
