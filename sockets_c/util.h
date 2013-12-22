#ifndef UTIL_H
#define UTIL_H
/*
 * gmp ja HASH apurutiineja kurssille OHJ-5010
 *
 * GMP manual: http://www.gnu.org/software/gmp/manual/html_node
 *
 * Jyke Jokinen 2012
 * Käyttöoikeudet: http://creativecommons.org/licenses/by-nc/1.0/fi/
 *
 */

#include <gmp.h>
#include <nettle/sha.h>

typedef unsigned char BYTE;

/* simppeli virheiden hallinta. tulosta ilmoitus ja lopeta suoritus */
void panic(char* str);

/* tuotetaan iso satunnaisluku GMPint-muuttujaan 'var' */
void set_random64( mpz_t var );

/* lasketaan sha256 hash-luku merkkijonosta, paluuarvo GMPint */
void sha256_s( const unsigned char* const s, mpz_t retval );

/* lasketaan sha256 hash-luku GMPint-luvusta ja merkkijonosta */
void sha256_zs( mpz_t z, const unsigned char* const s, mpz_t retval );

/* lasketaan sha256 hash-luku yhdesta GMPint-luvusta */
void sha256_z( mpz_t a,  mpz_t retval );

/* lasketaan sha256 hash-luku kahdesta GMPint-luvusta */
void sha256_zz( mpz_t a, mpz_t b, mpz_t retval );

/* palautetaan sha256 (binäärinen) hash GMPint-luvusta */
BYTE* sha256_binz( mpz_t a );

#endif
