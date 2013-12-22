/*
 * Secure Remote Password protocol 
 * Toteutuskirjasto kurssille OHJ-5010
 *
 * Noudattelee pääpiirteittäin algoritmin kuvausta:
 * http://en.wikipedia.org/wiki/Secure_Remote_Password_protocol
 *
 * GMP manual: http://www.gnu.org/software/gmp/manual/html_node
 *
 * Jyke Jokinen 2012
 * Käyttöoikeudet: http://creativecommons.org/licenses/by-nc/1.0/fi/
 *
 * katso tämän kirjaston käyttöesimerkki tiedostosta: test-srp.c
 */

#include "srp.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* -------------------------------------------------------------------- */
static bool have_g = false;
static mpz_t my_g;
/* alkuarvojen alustus (generator, module space) */
void set_g( mpz_t g )
{
  assert( ! have_g );   /* kirjaston saa alustaa vain kerran */
  mpz_init( my_g );
  mpz_set( my_g, g );
  have_g = true;
}

/* -------------------------------------------------------------------- */
static bool have_N = false;
static mpz_t my_N;
void set_N( mpz_t N )
{
  assert( ! have_N );
  mpz_init( my_N );
  mpz_set( my_N, N );
  have_N = true;
}

/* -------------------------------------------------------------------- */
/* yhteinen parametri laskemisessa, k = HASH( N, g ) */
static bool have_k = false;
static mpz_t my_k;
void calc_k()
{
  assert( have_g );
  assert( have_N );
  mpz_init( my_k );
  sha256_zz( my_N, my_g, my_k );
  have_k = true;
}


/* asetetaan käyttäjän verifier-arvo */
/* asiakas laskee arvon aina, palvelimella on etukäteen laskettu v */
static bool have_v = false;
static mpz_t my_v;
static bool have_x = false;
static mpz_t my_x;
static bool have_s = false;
static mpz_t my_s;
/* -------------------------------------------------------------------- */
void client_set_v( mpz_t salt, unsigned const char * password )
{
  mpz_init( my_s );
  mpz_set( my_s, salt );
  have_s = true;

  mpz_init( my_x );
  sha256_zs( salt, password, my_x );
  have_x = true;

  mpz_init( my_v );
  assert( have_g );
  assert( have_N );
  mpz_powm( my_v, my_g, my_x, my_N );
  have_v = true;
}
/* -------------------------------------------------------------------- */
void server_set_v( mpz_t salt, mpz_t v )
{
  mpz_init( my_s );
  mpz_set( my_s, salt );
  have_s = true;

  mpz_init( my_v );
  mpz_set( my_v, v );
  have_v = true;
}

/* -------------------------------------------------------------------- */
/* sisäinen aputulos laskutoimituksille u = HASH(A,B) */
static mpz_t my_u;
static bool have_A = false;
static mpz_t my_A;
static bool have_B = false;
static mpz_t my_B;
void calc_u()
{
  mpz_init( my_u );
  assert( have_A );
  assert( have_B );
  sha256_zz( my_A, my_B, my_u );
}


/* http://en.wikipedia.org/wiki/Secure_Remote_Password_protocol */
/* http://srp.stanford.edu/doc.html#papers */

/* asiakkaan rutiinit (Carol wikipedian dokussa) */
/* -------------------------------------------------------------------- */
static bool have_a = false;
static mpz_t my_a;
void client_init( mpz_t A )  /* arpoo a:n, laskee ja palauttaa A = g^a */
{
  mpz_init( my_a );
  set_random64( my_a );
  have_a = true;
  assert( have_g );
  assert( have_N );
  mpz_powm( my_A, my_g, my_a, my_N );  /* g^a mod N */
  mpz_set( A, my_A );
  have_A = true;
}
/* -------------------------------------------------------------------- */
void client_setB( mpz_t B)   /* Vastapäältä saatu arvo */
{
  mpz_init( my_B );
  mpz_set( my_B, B );
  have_B = true;
}

static bool have_K = false;
static mpz_t my_K;
/* -------------------------------------------------------------------- */
void client_K( mpz_t K )     /* lasketaan salausavain */
{
  mpz_t S;
  mpz_init( S );
  calc_u();
  calc_k();
  assert( have_a );
  assert( have_x );
  mpz_t ux; mpz_init( ux );
  mpz_mul( ux, my_u, my_x );   /* ux = u * x */
  mpz_t pow; mpz_init( pow );  
  mpz_add( pow, my_a, ux );    /* pow = a + ux */
  mpz_clear( ux ); /* välituloksen ux voi vapauttaa */

  assert( have_x );
  assert( have_g );
  assert( have_k );
  assert( have_B );
  mpz_t a; mpz_init( a );   /* a = g^x mod N */
  mpz_powm( a, my_g, my_x, my_N );
  mpz_t b; mpz_init( b );   /* b = k * g^x = k * a */
  mpz_mul( b, my_k, a );
  mpz_clear( a ); /* välituloksen vapautus */

  mpz_t base; mpz_init( base ); /* base = B - k*g^x = B - b */
  mpz_sub( base, my_B, b );
  mpz_clear( b ); /* välituloksen vapautus */

  mpz_powm( S, base, pow, my_N );  /* K = (B-k*g^x) ^ (a+ux)  mod N */
  mpz_clear( base ); mpz_clear( pow );

  sha256_z( S, my_K );  /* K = HASH( S ) */
  have_K = true;

  /* asetetaan paluuarvo */
  mpz_set( K, my_K );
}

/* -------------------------------------------------------------------- */
static void M1( mpz_t retval )
{
  assert( have_N );
  assert( have_g );
  assert( have_s );
  assert( have_A );
  assert( have_B );
  assert( have_K );

  /* H(N) XOR H(g) */
  BYTE* HN = sha256_binz( my_N );
  BYTE* Hg = sha256_binz( my_g );
  for( int i = 0; i < SHA256_DIGEST_SIZE; i++ ) {
    HN[i] ^= Hg[i];
  }
  mpz_t HG; mpz_init( HG );
  mpz_import( HG, SHA256_DIGEST_SIZE, 1, 1, 0, 0, HN );
  free( HN); free( Hg); /* datapuskureiden vapautus */

  /* catenoinnit H(N) XOR H(g) | s | A | B | K */
  /* XXX HUOM! H(I) jätetty pois */
  char* pALL;
  gmp_asprintf( &pALL, "%Zx%Zx%Zx%Zx%Zx", HG, my_s, my_A, my_B, my_K );

  /* yhdistetystä datasta lasketaan vielä HASH, joka on paluuarvo */
  sha256_s( (const unsigned char const*)pALL, retval );
  free( pALL );
}

static void M2( mpz_t retval )
{
  assert( have_A );
  assert( have_K );

  mpz_t my_M1; mpz_init( my_M1 );
  M1( my_M1 );

  /* A | M1 | K */
  char* pALL;
  gmp_asprintf( &pALL, "%Zx%Zx%Zx", my_A, my_M1, my_K );

  /* yhdistetystä datasta HASH, joka on paluuarvo */
  sha256_s( (const unsigned char const*)pALL, retval );
  free( pALL );
}

/* -------------------------------------------------------------------- */
void client_M1( mpz_t retval )   /* laskee ja palauttaa todistusarvon M1 */
{
  M1( retval );
}

/* -------------------------------------------------------------------- */
bool client_verifyM2( mpz_t other_M2 ) /* palauttaa true jos vastapään arvo OK */
{
  mpz_t my_M2; mpz_init( my_M2 );
  M2( my_M2 );
  
  if( mpz_cmp( my_M2, other_M2 ) == 0 ) {
    return true;
  } else {
    return false;
  }
}

/* -------------------------------------------------------------------- */
/* palvelimen rutiinit (Steve wikipedian dokussa) */
/* -------------------------------------------------------------------- */
static bool have_b = false;
static mpz_t my_b;
void server_init( mpz_t B ) /* arpoo b:n, laskee ja palauttaa B = kv + g^b */
{
  calc_k();

  mpz_init( my_b );
  set_random64( my_b );
  have_b = true;

  assert( have_v );
  assert( have_k );
  assert( have_g );
  assert( have_N );

  mpz_t pow; mpz_init( pow );
  mpz_powm( pow, my_g, my_b, my_N );  // pow = g^b mod N

  mpz_t kv; mpz_init( kv );
  mpz_mul( kv, my_k, my_v );  // kv = k * v
  mpz_add( my_B, kv, pow );   // kv + g^b mod N
  mpz_mod( my_B, my_B, my_N );
  have_B = true;
  mpz_clear( kv ); mpz_clear( pow );

  mpz_set( B, my_B );
}

/* -------------------------------------------------------------------- */
void server_setA( mpz_t A ) /* vastapään arvo */
{
  mpz_set( my_A, A );
  have_A = true;
}
/* -------------------------------------------------------------------- */
void server_K( mpz_t K )    /* lasketaan salausavain */
{
  mpz_t S;
  mpz_init( S );
  calc_u();
  assert( have_v );
  assert( have_b );
  assert( have_A );
  assert( have_N );
  mpz_t pow; mpz_init( pow );
  mpz_powm( pow, my_v, my_u, my_N );  /* pow = v^u mod N */
  mpz_t Avu; mpz_init( Avu );
  mpz_mul( Avu, my_A, pow );   /* Avu = A * pow = A * v^u */
  mpz_clear(pow);
  mpz_powm( S, Avu, my_b, my_N );  /* S = (A * v^u) ^ b  mod N */
  mpz_clear( Avu );

  sha256_z( S, my_K );  /* K = HASH( S ) */
  mpz_set( K, my_K );
  have_K = true;
}
/* -------------------------------------------------------------------- */
void server_M2( mpz_t retval )  /* lasketaan todistusarvo M2 */
{
  M2( retval );
}
/* -------------------------------------------------------------------- */
bool server_verifyM1( mpz_t other_M1 ) /* palauttaa true jos vastapään arvo OK */
{
  mpz_t my_M1; mpz_init( my_M1 );
  M1( my_M1 );
  if( mpz_cmp( my_M1, other_M1 ) == 0 ) {
    return true;
  } else {
    return false;
  }
}


