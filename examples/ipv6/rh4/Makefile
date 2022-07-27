CONTIKI = $(HIPTBDIR)/support/contiki-wsn430
TARGET = sky

WITH_UIP6=1
UIP_CONF_IPV6=1
SMALL=1

# Disable RPL
CFLAGS += -DUIP_CONF_IPV6_RPL=0
# Queue and retransmit packets that are sent before neighbor discovery has finished
CFLAGS += -DUIP_CONF_IPV6_QUEUE_PKT=1
# Generate simple link-local addresses fe80::<node-id>
CFLAGS += -DIEEE_802154_MAC_ADDRESS={2,0,0,0,0,0,0,0}
# All addresses use the same scheme, we can compress them down to one byte
CFLAGS += -DUIP_CONF_RH4_CMPRI=15
CFLAGS += -DUIP_CONF_RH4_CMPRE=15
# Maximum number of entries in the source route table
CFLAGS += -DUIP_CONF_DS6_SRC_ROUTE_NBU=4
# Maximum number of hops of all source routes in total
CFLAGS += -DUIP_CONF_DS6_SRC_ROUTE_ADDR_NBU=10
# The UDP checksum incorporates the IPv6 destination address. Since RH4 changes
# this field, checksum verification will always fail and is thus disabled.
# XXX This is already defined in platform/sky/contiki-conf.h and thus needs to
#     be changed there.
#CFLAGS += -DUIP_CONF_UDP_CHECKSUMS=0

all: rh4-sim

include $(CONTIKI)/Makefile.include
