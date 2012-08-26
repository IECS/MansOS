#
# Notes:
# * new applications must be added manually
# * clean target should be improved
#   (telosb must have all targets, otherwise they will not be cleared)
#

PLATFORMS = pc telosb sadmote farmmote atmega z1
# launchpad

DEMO_TARGETS = \
	Blink					\
	BlinkCounter				\
	BlinkRadio				\
	PrintADC				\
	PrintCounter				\
	PrintEcho				\
	PrintRadio				\
	PrintSensors				\

TEST_TARGETS = \
	AccelTest				\
	AdaptiveRate				\
	AdcTest					\
	AdsTest					\
	AlarmTest				\
	ApdsTest				\
	AssertTest				\
	BeeperTest				\
	BlinkTest				\
	ExtFlashTest				\
	FSTest					\
	GPSParseTest				\
	HumidityTest				\
	IntFlashTest				\
	IslLightSensorTest			\
	LinkTest				\
	ListTest				\
	Null					\
	PDRTest					\
	RadioSendTest				\
	RadioTest				\
	RealTimeTest				\
	SdCardTest				\
	SerialPipe				\
	SerialPrint				\
	SharingTest				\
	SleepTest				\
	SnumTest				\
	TinyOSEncapsuleTest			\
	UARTRecvPacketTest			\
	UARTTest				\
	SmpTest/DemoBlueLed			\
	SmpTest/DemoRedLed			\
	ExpThreadTests/0-Lockup			\
	ExpThreadTests/0-Overflow-User		\
	ExpThreadTests/0-Overflow-Kernel	\
	ExpThreadTests/1a-Blink-mdelay		\
	ExpThreadTests/1b-Blink-msleep		\
	ExpThreadTests/1c-Blink-Multi		\
	ExpThreadTests/2-Alarms			\
	ExpThreadTests/3-Locking		\
	ExpThreadTests/3-Locking-Multi		\
	ExpThreadTests/4-Exit			\
	ExpThreadTests/5-Radio			\
	ExpThreadTests/6-MAC			\
	ExpThreadTests/7-Sockets		\

ADVANCED_TARGETS = \
	Accel3DTest				\
	AccelDice				\
	AccelMusic				\
	AccelTrack				\
	BlinkGlow				\
	Bootloader				\
	Calendar				\
	GPSParsing				\
	GpsProxy				\
	IMU					\
	LPL					\
	Sarauj					\
	SeePoga					\
	SensorToLcd				\
	SensorToLeds				\
	TimerSense				\
	UserButtonRead				\

SEAL_TARGETS = \
	Blink					\
	Custom					\
	Sensors					\

COMMON_TARGETS = \
	$(foreach t,$(DEMO_TARGETS),demo/$(t)) \
	$(foreach t,$(ADVANCED_TARGETS),advanced/$(t)) \
	$(foreach t,$(TEST_TARGETS),tests/$(t)) \
	$(foreach t,$(SEAL_TARGETS),seal/$(t))

#COMMON_TARGETS = \
#	$(foreach t,$(DEMO_TARGETS),demo/$(t))

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
	$(eval $(platform)_EXES=$(foreach app,$(COMMON_TARGETS),$(app)-$(platform))))

# this generates the actual targets
$(foreach platform,$(PLATFORMS),\
	$(eval $(call PLATFORM_template,$(platform))))

# this is simple-and-wrong global clean target
clean: telosb-clean
