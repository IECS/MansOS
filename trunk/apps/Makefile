#
# Notes:
# * new applications must be added manually
# * clean target should be improved
#   (telosb must have all targets, otherwise they will not be cleared)
#

PLATFORMS = pc telosb sadmote sm3 farmmote atmega z1
# launchpad

DEMO_TARGETS = \
	Blink					\
	BlinkCounter				\
	BlinkRadio				\
	BlinkSensor				\
	PrintADC				\
	PrintCounter				\
	PrintEcho				\
	PrintRadio				\
	PrintSensors				\

TEST_TARGETS = \
	AccelTest				\
	AccelTestZolertiaZ1			\
	AdaptiveRate				\
	AdcTest					\
	AdcToUart				\
	Ads111xTest				\
	ADXL345Test				\
	AlarmTest				\
	ApdsTest				\
	AssertTest				\
	BeeperTest				\
	BlinkTest				\
	DcoRecalibrationTest	\
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
	RadioRecvTest				\
	SdCardBlockTest				\
	SerialPipe				\
	SharingTest				\
	SleepTest				\
	SnumTest				\
	TimingTest				\
	TinyOSEncapsuleTest			\
	TMP102Test				\
	SerialRecvPacketTest			\
	SerialRecvTest				\
	SmpTest/DemoBlueLed			\
	SmpTest/DemoRedLed			\
	StackTest		            	\
	UserButtonTest				\
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
	TestBed/AdsTest				\
	TestBed/DacTest				\
	TestBed/PotTest				\
	ProtoThreadTests/01-PTYield		\
	ProtoThreadTests/02-PTBlink		\
	ProtoThreadTests/03-PTRadio		\
	FatFSTests/01-Format			\
	FatFSTests/02-Open			\
	FatFSTests/03-ReadWrite			\
	FatFSTests/04-CreateRemove		\
	FatFSTests/05-LongReadWrite		\
	FatFSTests/06-FilePrint			\
	FatFSTests/07-FileSeek			\
	WMPTest 				\


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

SEAL_TARGETS = \
	Blink					\

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
