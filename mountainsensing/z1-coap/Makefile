all: z1-coap
TARGET = z1-feshie

CONTIKI=../../
PROTOBUF:=../protocol-buffers/
NANOPB:=$(PROTOBUF)tools/nanopb/

CONTIKI_WITH_IPV6=1
CFLAGS += -DUIP_CONF_IPV6=1
CFLAGS += -DREST=coap_rest_implementation

# Disable human readable process names - they're stored in .data
CFLAGS += -DPROCESS_CONF_NO_PROCESS_NAMES=1

# Disable the accelerometer
CFLAGS += -DFESHIE_NO_ACC

SMALL=1

APPS += erbium er-coap rest-engine

PROJECTDIRS += $(NANOPB) $(PROTOBUF)c/ ../common/

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_SOURCEFILES += pb_decode.c pb_encode.c pb_common.c all.c store.c sampler.c er-server.c res_date.c res_sample.c res_config.c res_reboot.c res_routes.c res_uptime.c

include $(CONTIKI)/Makefile.include

install: z1-coap.upload

CUSTOM_RULE_LINK=1
%.$(TARGET): %.co $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES) contiki-$(TARGET).a $(LD) $(LDFLAGS) $(TARGET_STARTFILES) ${filter-out %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -o $@ -lm
