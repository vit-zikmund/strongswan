/*
 * Copyright (C) 2026 Vit Zikmund
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <test_suite.h>

#include "nm_creds.h"

#include <credentials/keys/shared_key.h>
#include <utils/identification.h>

static nm_creds_t *creds;

START_SETUP(setup_creds)
{
	creds = nm_creds_create();
}
END_SETUP

START_TEARDOWN(teardown_creds)
{
	creds->destroy(creds);
}
END_TEARDOWN

/**
 * Return a clone of the first shared secret served for the given type and
 * identities, or chunk_empty if none is served.
 */
static chunk_t find_shared(shared_key_type_t type, identification_t *me,
						   identification_t *other)
{
	enumerator_t *enumerator;
	shared_key_t *key;
	chunk_t found = chunk_empty;

	enumerator = creds->set.create_shared_enumerator(&creds->set, type, me,
													 other);
	if (enumerator)
	{
		if (enumerator->enumerate(enumerator, &key, NULL, NULL))
		{
			found = chunk_clone(key->get_key(key));
		}
		enumerator->destroy(enumerator);
	}
	return found;
}

/**
 * Assert that the served secret equals expected (or that none is served if
 * expected is NULL).
 */
static void assert_secret(shared_key_type_t type, identification_t *me,
						  identification_t *other, char *expected)
{
	chunk_t found = find_shared(type, me, other);

	if (expected)
	{
		ck_assert_chunk_eq(found, chunk_from_str(expected));
	}
	else
	{
		ck_assert_int_eq(found.len, 0);
	}
	chunk_free(&found);
}

START_TEST(test_gateway_psk)
{
	identification_t *gw = identification_create_from_string("192.168.0.1");

	creds->set_psk(creds, gw, "this-is-a-gateway-psk");
	/* served for IKE shared key lookups against the gateway... */
	assert_secret(SHARED_IKE, NULL, gw, "this-is-a-gateway-psk");
	/* ...but never as an EAP/user secret */
	assert_secret(SHARED_EAP, NULL, gw, NULL);

	gw->destroy(gw);
}
END_TEST

START_TEST(test_psk_separate_from_eap)
{
	identification_t *gw = identification_create_from_string("192.168.0.1");
	identification_t *user = identification_create_from_string("user@strongswan.org");

	/* asymmetric auth: gateway PSK plus a distinct client EAP password */
	creds->set_psk(creds, gw, "this-is-a-gateway-psk");
	creds->set_username_password(creds, user, "eap-password");

	/* the IKE lookup must return the PSK, not the EAP password */
	assert_secret(SHARED_IKE, user, gw, "this-is-a-gateway-psk");
	/* the EAP lookup must return the password bound to the user */
	assert_secret(SHARED_EAP, user, NULL, "eap-password");

	gw->destroy(gw);
	user->destroy(user);
}
END_TEST

START_TEST(test_legacy_mutual_psk)
{
	identification_t *id = identification_create_from_string("user@strongswan.org");

	/* legacy mutual-PSK configs registered the key via the password only */
	creds->set_username_password(creds, id, "mutual-preshared-key");

	/* without a dedicated PSK, IKE lookups fall back to that secret */
	assert_secret(SHARED_IKE, id, NULL, "mutual-preshared-key");
	assert_secret(SHARED_EAP, id, NULL, "mutual-preshared-key");

	id->destroy(id);
}
END_TEST

START_TEST(test_psk_bound_to_gateway)
{
	identification_t *gw = identification_create_from_string("192.168.0.1");
	identification_t *other = identification_create_from_string("192.168.0.2");

	creds->set_psk(creds, gw, "this-is-a-gateway-psk");

	/* served for the configured gateway and for unconstrained queries... */
	assert_secret(SHARED_IKE, NULL, gw, "this-is-a-gateway-psk");
	assert_secret(SHARED_IKE, NULL, NULL, "this-is-a-gateway-psk");
	/* ...but not for a different peer identity */
	assert_secret(SHARED_IKE, NULL, other, NULL);

	gw->destroy(gw);
	other->destroy(other);
}
END_TEST

START_TEST(test_clear)
{
	identification_t *gw = identification_create_from_string("192.168.0.1");

	creds->set_psk(creds, gw, "this-is-a-gateway-psk");
	creds->clear(creds);
	assert_secret(SHARED_IKE, NULL, gw, NULL);

	gw->destroy(gw);
}
END_TEST

Suite *creds_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("nm creds");

	tc = tcase_create("shared secrets");
	tcase_add_checked_fixture(tc, setup_creds, teardown_creds);
	tcase_add_test(tc, test_gateway_psk);
	tcase_add_test(tc, test_psk_separate_from_eap);
	tcase_add_test(tc, test_legacy_mutual_psk);
	tcase_add_test(tc, test_psk_bound_to_gateway);
	tcase_add_test(tc, test_clear);
	suite_add_tcase(s, tc);

	return s;
}
