WMP_SENSOR_LIGHT     = 1
WMP_SENSOR_HUMIDITY  = 2
WMP_SENSOR_BATTERY   = 3
WMP_SENSOR_ADC0      = 4
WMP_SENSOR_ADC1      = 5
WMP_SENSOR_ADC2      = 6
WMP_SENSOR_ADC3      = 7
WMP_SENSOR_ADC4      = 8
WMP_SENSOR_ADC5      = 9
WMP_SENSOR_ADC6      = 10
WMP_SENSOR_ADC7      = 11

WMP_OUTPUT_SERIAL    = 1
WMP_OUTPUT_SDCARD    = 2
WMP_OUTPUT_FILE      = 3


WMP_START_CHARACTER  = '$'


# set LED state
WMP_CMD_SET_LED      = 1
# get LED state
WMP_CMD_GET_LED      = 2
# set sensor reading period
WMP_CMD_SET_SENSOR   = 3
# get sensor reading period
WMP_CMD_GET_SENSOR   = 4
# enable/disable output to e.g. serial, file etc
WMP_CMD_SET_OUTPUT   = 5
# get output status (enabled/disabled)
WMP_CMD_GET_OUTPUT   = 6
# set local network address
WMP_CMD_SET_ADDR     = 7
# get local network address
WMP_CMD_GET_ADDR     = 8
# get list of all files on FAT filesystem
WMP_CMD_GET_FILELIST = 9
# set file name (to use for data logging)
WMP_CMD_SET_FILE     = 10
# get contents of a file
WMP_CMD_GET_FILE     = 11
# set DAC channel value
WMP_CMD_SET_DAC      = 12
# get DAC channel value
WMP_CMD_GET_DAC      = 13

# this bit is set in replies to commands
WMP_CMD_REPLY_FLAG   = 0x80
