SISALC=${bindir}/sisalc

LDADD = $(pkglibdir)/libsisal.a $(pkglibdir)/srt0.o

AM_LDFLAGS = $(parallel_link)

AM_DEFAULT_SOURCE_EXT = .sis

.sis.o:
	$(SISALC) -w -c $< -o $@

TESTSUITE = three hello 8queens bubble quicksort crypto fromC inputio modules

check_PROGRAMS = $(TESTSUITE)

fromC_SOURCES = fromC.sis fromCdriver.c

fromC_LDADD = $(pkglibdir)/libsisal.a

# A plain C test of the runtime input builtins; it needs the installed runtime
# headers, which is also where the .sis tests pick up libsisal.a.
inputio_SOURCES = inputio.c
inputio_CPPFLAGS = $(AM_CPPFLAGS) -I$(pkgincludedir)
inputio_LDADD = $(pkglibdir)/libsisal.a

fromC.$(OBJEXT): fromC.sis
	$(SISALC) -forC -w -c $(srcdir)/fromC.sis -o $(builddir)/fromC.$(OBJEXT)

# Two independently compiled SISAL programs (see the comment in
# moduleA.sis) linked with a hand-written C driver into one binary --
# regression coverage for multiple -forC translation units coexisting,
# and for passing a real argument (not just a result) across the FLI.
modules_SOURCES = moduleA.sis moduleB.sis modulesdriver.c
modules_LDADD = $(pkglibdir)/libsisal.a

moduleA.$(OBJEXT): moduleA.sis
	$(SISALC) -forC -w -c $(srcdir)/moduleA.sis -o $(builddir)/moduleA.$(OBJEXT)

moduleB.$(OBJEXT): moduleB.sis
	$(SISALC) -forC -w -c $(srcdir)/moduleB.sis -o $(builddir)/moduleB.$(OBJEXT)

dist_check_SCRIPTS = run-test

# run-test wraps each test binary, so it belongs in LOG_COMPILER.  It used to
# be passed via TESTS_ENVIRONMENT, which the old serial-tests harness ran as a
# command prefix; under the parallel-tests driver that automake has defaulted
# to since 1.13, TESTS_ENVIRONMENT sets environment variables only and the
# wrapper ended up swallowing the test name.
LOG_COMPILER = $(SHELL) $(srcdir)/run-test
AM_LOG_FLAGS = "$(DIFF)" "$(RM)" "$(srcdir)" "$(builddir)"

TESTS = $(check_PROGRAMS)

dist_noinst_DATA = ${TESTSUITE:%=%.in} ${TESTSUITE:%=%.ans}

CLEANFILES = ${TESTSUITE:%=%.out} ${TESTSUITE:%=%.out3} ${TESTSUITE:%=%.dif} ${TESTSUITE:%=%.dif3} \
	inputio-empty.dat inputio-small.dat inputio-big.dat inputio-stdin.dat

answers:
	for tst in $(TESTS); do \
	  $(LOG_COMPILER) $(AM_LOG_FLAGS) --answer $(builddir)/$${tst} ; \
	done

