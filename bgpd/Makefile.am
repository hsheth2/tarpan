## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I.. -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

AM_CFLAGS = '-fPIC' $(PICFLAGS)
AM_LDFLAGS = $(PILDFLAGS)

noinst_LIBRARIES = libbgp.a
#sbin_PROGRAMS = bgpd
sbin_PROGRAMS = bgpd tarpantest

BUILT_SOURCES = \
	attr_tarpan.pb-c.c attr_tarpan.pb-c.h tarpan_backpropagation.pb-c.c tarpan_backpropagation.pb-c.h
CLEANFILES = $(BUILT_SOURCES)

attr_tarpan.pb-c.c attr_tarpan.pb-c.h: attr_tarpan.proto
	@PROTOC_C@ --c_out=. attr_tarpan.proto
	
tarpan_backpropagation.pb-c.c tarpan_backpropagation.pb-c.h: tarpan_backpropagation.proto
	@PROTOC_C@ --c_out=. tarpan_backpropagation.proto

libbgp_a_SOURCES = \
	bgpd.c bgp_fsm.c bgp_aspath.c bgp_community.c bgp_attr.c \
	bgp_debug.c bgp_route.c bgp_zebra.c bgp_open.c bgp_routemap.c \
	bgp_packet.c bgp_network.c bgp_filter.c bgp_regex.c bgp_clist.c \
	bgp_dump.c bgp_snmp.c bgp_ecommunity.c bgp_mplsvpn.c bgp_nexthop.c \
	bgp_damp.c bgp_table.c bgp_advertise.c bgp_vty.c bgp_mpath.c \
	bgp_tarpan.c attr_tarpan.pb-c.c tarpan_backpropagation.pb-c.c wiser.cpp dummy.cpp

noinst_HEADERS = \
	bgp_aspath.h bgp_attr.h bgp_community.h bgp_debug.h bgp_fsm.h \
	bgp_network.h bgp_open.h bgp_packet.h bgp_regex.h bgp_route.h \
	bgpd.h bgp_filter.h bgp_clist.h bgp_dump.h bgp_zebra.h \
	bgp_ecommunity.h bgp_mplsvpn.h bgp_nexthop.h bgp_damp.h bgp_table.h \
	bgp_advertise.h bgp_snmp.h bgp_vty.h bgp_mpath.h \
	bgp_tarpan.h attr_tarpan.pb-c.h tarpan_backpropagation.pb-c.h wiser.h

bgpd_SOURCES = bgp_main.c bgpd_force_cpp.cpp
bgpd_LDADD = libbgp.a ../lib/libzebra.la @LIBCAP@ @LIBM@ -lprotobuf-c @libstdcxx_LIBS@

tarpantest_SOURCES = tarpan_test.cc
tarpantest_LDADD = libbgp.a ../lib/libzebra.la @LIBCAP@ @LIBM@ -lprotobuf-c 

examplesdir = $(exampledir)
dist_examples_DATA = bgpd.conf.sample bgpd.conf.sample2

EXTRA_DIST = BGP4-MIB.txt attr_tarpan.proto
