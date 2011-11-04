#
# Notes:
# * new applications must be added manually
# * clean target should be improved
#   (telosb must have all targets, otherwise they will not be cleared)
#

PLATFORMS = pc telosb epic atmega waspmote

DEMO_TARGETS = \
	Blink \
	BlinkCounter \
	BlinkRadio \
	PrintADC \
	PrintCounter \
	PrintEcho \
	PrintRadio \
	PrintSensors \

ADVANCED_TARGETS = \
	GpsProxy \
	LPL \
	Sarauj

TEST_TARGETS = \
	AlarmTest \
	AssertTest \
	BeeperTest \
	BlinkTest \
	ExtFlashTest \
	FSTest \
	IntFlashTest \
	HumidityTest \
	ListTest \
	Null \
	PDRTest \
	RadioTest \
	RadioSendTest \
	RealTimeTest \
	SemaphoreTest \
	SerialPrint \
	SharingTest \
	SnumTest \
	SleepTest \
	SocketTest \
	TdmaTest \
	UARTTest \
	UARTRecvPacketTest \
	SmpTest/DemoBlueLed \
	SmpTest/DemoRedLed \
	ExpThreadTests/0-Lockup \
	ExpThreadTests/0-Overflow-User \
	ExpThreadTests/0-Overflow-Kernel \
	ExpThreadTests/1a-Blink-mdelay \
	ExpThreadTests/1b-Blink-msleep \
	ExpThreadTests/1c-Blink-Multi \
	ExpThreadTests/2-Alarms \
	ExpThreadTests/3-Locking \
	ExpThreadTests/3-Locking-Multi \
	ExpThreadTests/4-Exit \
	ExpThreadTests/5-Radio \
	ExpThreadTests/6-MAC \
	ExpThreadTests/7-Sockets


COMMON_TARGETS = \
	$(foreach t,$(DEMO_TARGETS),demo/$(t)) \
	$(foreach t,$(ADVANCED_TARGETS),advanced/$(t)) \
	$(foreach t,$(TEST_TARGETS),tests/$(t))

telosb_TARGETS = $(COMMON_TARGETS)
telosb_TARGETS += \
	advanced/Accel3DTest \
	advanced/AccelDice \
	advanced/AccelMusic \
	advanced/AccelTrack \
	advanced/Bootloader \
	advanced/UserButtonRead \
	tests/TinyOSEncapsuleTest \
	tests/SdCardTest \
	tests/AccelTest \
	tests/FSTest \
	tests/AdcTest \

pc_TARGETS = $(COMMON_TARGETS) \
	tests/FSTest

epic_TARGETS = $(COMMON_TARGETS)

atmega_TARGETS = $(COMMON_TARGETS)

waspmote_TARGETS = $(atmega_TARGETS)

# -- generic part follows

.PHONY: all clean $(PLATFORMS)

all: $(PLATFORMS)

define PLATFORM_template
# -- build
$(1): $$($(1)_EXES)
$$($(1)_EXES):
	make -C $$(patsubst %-$(1),%,$$@) $(1)
# -- clean
$(1)-clean: $$(foreach exe,$$($(1)_EXES),$$(exe)-clean)
$$(foreach exe,$$($(1)_EXES),$$(exe)-clean):
	make -C $$(patsubst %-$(1)-clean,%,$$@) clean
endef

# this generates unique names for platform targets
$(foreach platform,$(PLATFORMS),\
	$(eval $(platform)_EXES=$(foreach app,$($(platform)_TARGETS),$(app)-$(platform))))

# this generates the actual targets
$(foreach platform,$(PLATFORMS),\
	$(eval $(call PLATFORM_template,$(platform))))

# this is simple-and-wrong global clean target
clean: telosb-clean
