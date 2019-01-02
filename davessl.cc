/* davep 20190102 ; learning ssl library (old code laying around I'm finally checking in)
 * work in progress, obviously
 *
 * gcc -g -Wall -o davessl davessl.c -lssl
 *
 */

#include <stdio.h>

#include <openssl/ssl.h>

int main(void)
{
	SSL_CTX *ctx;

	ctx = SSL_CTX_new(TLS_method());

	SSL_CTX_free(ctx);
	return EXIT_SUCCESS;
}

