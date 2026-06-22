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

#include <test_runner.h>

/* declare test suite constructors */
#define TEST_SUITE(x) test_suite_t* x();
#include "tests.h"
#undef TEST_SUITE

static test_configuration_t tests[] = {
#define TEST_SUITE(x) \
	{ .suite = x, },
#include "tests.h"
	{ .suite = NULL, }
};

int main(int argc, char *argv[])
{
	/* the tested code only needs core libstrongswan, so no plugins are loaded */
	return test_runner_run("charon-nm", tests, NULL);
}
