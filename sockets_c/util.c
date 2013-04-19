/*
 * gmp ja HASH apurutiineja kurssille OHJ-5010
 *
 * GMP manual: http://www.gnu.org/software/gmp/manual/html_node
 *
 * Jyke Jokinen 2012
 * K‰yttˆoikeudet: http://creativecommons.org/licenses/by-nc/1.0/fi/
 *
 */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <nettle/sha.h>

/* --------------------------------------------------------------------- */
void panic(char* str)
{
  perror(str);
  exit(1);
}

/* --------------------------------------------------------------------- */
void set_random64( mpz_t var )
{
  const unsigned int SIZE = 64;
  unsigned char data[SIZE];
  FILE* rawdata = NULL;

  /* koodi suostuu toimimaan vain jos koneelta lˆytyy urandom */
  /* kryptografian kannalta oikeampi lukupaikka olisi /dev/random,
     mutta harjoitustyˆn kannalta t‰m‰ on riitt‰v‰ (ja nopeampi)
  */
  rawdata = fopen("/dev/urandom", "r");
  if( rawdata == NULL ) panic("urandom");

  if( fread(&data[0], 1, SIZE, rawdata) != SIZE )
    panic("fread urandom");

  fclose(rawdata);

  /* muutetaan luettu satunnainen bin‰‰rijono GMPint-luvuksi,
     joka palautetaan kutsujalle */
  mpz_import( var, sizeof(data), 1, 1, 0, 0, data );  
  return;
}

/* --------------------------------------------------------------------- */
/* apurutiini, jolla lis‰t‰‰n HASH:iin mpz-luvun sis‰lt‰m‰ data */
void sha256_update_mpz( struct sha256_ctx* context, const mpz_t luku )
{
  /* haetaan mpz-lukua vastaava bin‰‰ridata */
  size_t luku_len = 0;
  void*  luku_data = mpz_export( NULL, &luku_len, 1, 1, 0, 0, luku );
  if( luku_data == NULL ) panic( "sha256_update_mpz export" );

  /* lis‰t‰‰n data sha-laskentaan */
  sha256_update( context, luku_len, luku_data );

  free( luku_data ); /* k‰ytetyn muistipuskurin vapautus */
}

/* --------------------------------------------------------------------- */
/* HASH( string ) */
void sha256_s( const unsigned char* const s, mpz_t retval )
{
  /* digest lopputulos bin‰‰rin‰ */
  unsigned char digest[SHA256_DIGEST_SIZE];

  /* oletukset parametreista */
  assert( s != NULL );  /* merkkijono pit‰‰ saada */

  /* kirjaston avulla SHA256:n laskeminen: */
  { struct sha256_ctx sha_puskuri;  /* tietorakenne datan ker‰‰miseen */
    size_t s_len = strlen( (char*) s );
    assert( s_len > 0 );

    sha256_init( &sha_puskuri );    /* alustetaan tietorakenne */
    /* lis‰t‰‰n datalohkot: */
    sha256_update( &sha_puskuri, s_len, s );

    /* pyydet‰‰n datasta bin‰‰rinen sha256-hash -arvo: */
    sha256_digest( &sha_puskuri, SHA256_DIGEST_SIZE, digest );
  }

  /* tehd‰‰n digest-datasta GMPint */
  mpz_import( retval, SHA256_DIGEST_SIZE, 1, 1, 0, 0, digest );
}


/* --------------------------------------------------------------------- */
/* HASH( GMPint, password ) */
void sha256_zs( mpz_t z, const unsigned char* const s, mpz_t retval )
{
  /* digest lopputulos bin‰‰rin‰ */
  unsigned char digest[SHA256_DIGEST_SIZE];

  /* oletukset parametreista */
  assert( s != NULL );  /* merkkijono pit‰‰ saada */
  assert( mpz_cmpabs_ui( z, 0 ) != 0 ); /* nollaa ei hyv‰ksyt‰ */

  /* kirjaston avulla SHA256:n laskeminen: */
  { struct sha256_ctx sha_puskuri;  /* tietorakenne datan ker‰‰miseen */
    size_t s_len = strlen( (char*) s );
    assert( s_len > 0 );

    sha256_init( &sha_puskuri );    /* alustetaan tietorakenne */
    /* lis‰t‰‰n datalohkot: */
    sha256_update_mpz( &sha_puskuri, z );
    sha256_update( &sha_puskuri, s_len, s );

    /* pyydet‰‰n datasta bin‰‰rinen sha256-hash -arvo: */
    sha256_digest( &sha_puskuri, SHA256_DIGEST_SIZE, digest );
  }

  /* tehd‰‰n digest-datasta GMPint */
  mpz_import( retval, SHA256_DIGEST_SIZE, 1, 1, 0, 0, digest );
}

/* --------------------------------------------------------------------- */
/* HASH( GMPint ) */
void sha256_z( mpz_t a, mpz_t retval )
{
  /* digest lopputulos bin‰‰rin‰ */
  unsigned char digest[SHA256_DIGEST_SIZE];

  /* oletukset parametreista */
  assert( mpz_cmpabs_ui( a, 0 ) != 0 ); /* nollaa ei hyv‰ksyt‰ */

  /* kirjaston avulla SHA256:n laskeminen: */
  { struct sha256_ctx sha_puskuri;  /* tietorakenne datan ker‰‰miseen */

    sha256_init( &sha_puskuri );    /* alustetaan tietorakenne */
    /* lis‰t‰‰n datalohkot: */
    sha256_update_mpz( &sha_puskuri, a );

    /* pyydet‰‰n datasta bin‰‰rinen sha256-hash -arvo: */
    sha256_digest( &sha_puskuri, SHA256_DIGEST_SIZE, digest );
  }

  /* tehd‰‰n digest-datasta GMPint */
  mpz_import( retval, SHA256_DIGEST_SIZE, 1, 1, 0, 0, digest );
}

/* --------------------------------------------------------------------- */
/* HASH( GMPint, GMPint ) */
void sha256_zz( mpz_t a, mpz_t b, mpz_t retval )
{
  /* digest lopputulos bin‰‰rin‰ */
  unsigned char digest[SHA256_DIGEST_SIZE];

  /* oletukset parametreista */
  assert( mpz_cmpabs_ui( a, 0 ) != 0 ); /* nollaa ei hyv‰ksyt‰ */
  assert( mpz_cmpabs_ui( a, 0 ) != 0 ); /* nollaa ei hyv‰ksyt‰ */

  /* kirjaston avulla SHA256:n laskeminen: */
  { struct sha256_ctx sha_puskuri;  /* tietorakenne datan ker‰‰miseen */

    sha256_init( &sha_puskuri );    /* alustetaan tietorakenne */
    /* lis‰t‰‰n datalohkot: */
    sha256_update_mpz( &sha_puskuri, a );
    sha256_update_mpz( &sha_puskuri, b );

    /* pyydet‰‰n datasta bin‰‰rinen sha256-hash -arvo: */
    sha256_digest( &sha_puskuri, SHA256_DIGEST_SIZE, digest );
  }

  /* tehd‰‰n digest-datasta GMPint */
  mpz_import( retval, SHA256_DIGEST_SIZE, 1, 1, 0, 0, digest );
}

/* --------------------------------------------------------------------- */
/* HASH( GMPint ) bin‰‰risen‰ */
BYTE* sha256_binz( mpz_t a )
{
  /* digest lopputulos bin‰‰rin‰ */
  BYTE* retval = malloc( SHA256_DIGEST_SIZE + 1 );
  if( retval == NULL ) {
    panic("malloc failed");
  }

  /* oletukset parametreista */
  assert( mpz_cmpabs_ui( a, 0 ) != 0 ); /* nollaa ei hyv‰ksyt‰ */

  /* kirjaston avulla SHA256:n laskeminen: */
  { struct sha256_ctx sha_puskuri;  /* tietorakenne datan ker‰‰miseen */

    sha256_init( &sha_puskuri );    /* alustetaan tietorakenne */
    /* lis‰t‰‰n datalohkot: */
    sha256_update_mpz( &sha_puskuri, a );

    /* pyydet‰‰n datasta bin‰‰rinen sha256-hash -arvo: */
    sha256_digest( &sha_puskuri, SHA256_DIGEST_SIZE, retval );
  }

  return retval;
}

