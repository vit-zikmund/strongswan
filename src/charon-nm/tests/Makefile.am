TESTS = nm_creds_tests

check_PROGRAMS = $(TESTS)

nm_creds_tests_SOURCES = \
	tests.h tests.c \
	suites/test_creds.c \
	$(top_srcdir)/src/charon-nm/nm/nm_creds.c

nm_creds_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/charon-nm/nm \
	@COVERAGE_CFLAGS@

nm_creds_tests_LDFLAGS = @COVERAGE_LDFLAGS@
nm_creds_tests_LDADD = \
	$(top_builddir)/src/libcharon/libcharon.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la
