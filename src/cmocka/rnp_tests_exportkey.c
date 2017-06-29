/*
 * Copyright (c) 2017, [Ribose Inc](https://www.ribose.com).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <key_store_pgp.h>
#include <rnp.h>
#include <rnp_tests_support.h>

void
rnpkeys_exportkey_verifyUserId(void **state)
{
    /* Generate the key and export it */

    rnp_t rnp;
    int   pipefd[2];
    char *exportedkey = NULL;

    /* Initialize the rnp structure. */
    setup_rnp_common(&rnp, GPG_KEY_STORE, NULL, pipefd);

    /* Generate the key */
    set_default_rsa_key_desc(&rnp.action.generate_key_ctx, PGP_HASH_SHA256);
    assert_int_equal(1, rnp_generate_key(&rnp, NULL));

    /* Loading keyrings and checking whether they have correct key */
    assert_int_equal(rnp_key_store_load_keys(&rnp, 1), 1);
    assert_int_equal(rnp_secret_count(&rnp), 1);
    assert_int_equal(rnp_public_count(&rnp), 1);
    assert_int_equal(rnp_find_key(&rnp, getenv("LOGNAME")), 1);

    /* Try to export the key without passing userid from the interface */
    exportedkey = rnp_export_key(&rnp, NULL);
    assert_non_null(exportedkey);
    free(exportedkey);
    exportedkey = NULL;

    /* Try to export the key with specified userid parameter from the interface */
    exportedkey = rnp_export_key(&rnp, getenv("LOGNAME"));
    assert_non_null(exportedkey);
    free(exportedkey);
    exportedkey = NULL;

    /* try to export the key with specified userid parameter (which is wrong) */
    exportedkey = rnp_export_key(&rnp, "LOGNAME");
    assert_null(exportedkey);
    free(exportedkey);

    rnp_end(&rnp); // Free memory and other allocated resources.
}
