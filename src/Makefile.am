bin_PROGRAMS = sars
sars_SOURCES = KRH/khrplatform.h clear-node.h clear-node.c macros.h main.c sars.c sars.h glad.c glad.h
sars_CPPFLAGS = -I@top_srcdir@/libstage/src -I@top_srcdir@/libplay/src -ffast-math
sars_LDADD = @top_builddir@/libstage/src/libstage.a @top_builddir@/libplay/src/libplay.a -lm -ldl
