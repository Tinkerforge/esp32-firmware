/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_SOUND_PRESSURE_LEVEL_H
#define TF_SOUND_PRESSURE_LEVEL_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_SoundPressureLevel Sound Pressure Level Bricklet
 */

struct TF_SoundPressureLevel;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_SoundPressureLevel_DecibelHandler)(struct TF_SoundPressureLevel *sound_pressure_level, uint16_t decibel, void *user_data);
typedef void (*TF_SoundPressureLevel_SpectrumLowLevelHandler)(struct TF_SoundPressureLevel *sound_pressure_level, uint16_t spectrum_length, uint16_t spectrum_chunk_offset, uint16_t spectrum_chunk_data[30], void *user_data);
typedef void (*TF_SoundPressureLevel_SpectrumHandler)(struct TF_SoundPressureLevel *sound_pressure_level, uint16_t *spectrum, uint16_t spectrum_length, void *user_data);

#endif
/**
 * \ingroup TF_SoundPressureLevel
 *
 * Measures Sound Pressure Level in dB(A/B/C/D/Z)
 */
typedef struct TF_SoundPressureLevel {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_SoundPressureLevel_DecibelHandler decibel_handler;
    void *decibel_user_data;

    TF_SoundPressureLevel_SpectrumLowLevelHandler spectrum_low_level_handler;
    void *spectrum_low_level_user_data;

    TF_SoundPressureLevel_SpectrumHandler spectrum_handler;
    TF_HighLevelCallback spectrum_hlc;

#endif
    uint16_t magic;
    uint8_t response_expected[1];
} TF_SoundPressureLevel;

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_DECIBEL 1

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_DECIBEL_CALLBACK_CONFIGURATION 2

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_DECIBEL_CALLBACK_CONFIGURATION 3

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_SPECTRUM_LOW_LEVEL 5

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_SPECTRUM_CALLBACK_CONFIGURATION 6

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_SPECTRUM_CALLBACK_CONFIGURATION 7

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_CONFIGURATION 9

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_CONFIGURATION 10

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_RESET 243

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_READ_UID 249

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_CALLBACK_DECIBEL 4

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_CALLBACK_SPECTRUM_LOW_LEVEL 8

#endif

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_THRESHOLD_OPTION_OFF 'x'

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_THRESHOLD_OPTION_OUTSIDE 'o'

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_THRESHOLD_OPTION_INSIDE 'i'

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_THRESHOLD_OPTION_SMALLER '<'

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_THRESHOLD_OPTION_GREATER '>'

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FFT_SIZE_128 0

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FFT_SIZE_256 1

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FFT_SIZE_512 2

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_FFT_SIZE_1024 3

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_WEIGHTING_A 0

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_WEIGHTING_B 1

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_WEIGHTING_C 2

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_WEIGHTING_D 3

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_WEIGHTING_Z 4

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_WEIGHTING_ITU_R_468 5

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_SoundPressureLevel
 */
#define TF_SOUND_PRESSURE_LEVEL_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_SoundPressureLevel
 *
 * This constant is used to identify a Sound Pressure Level Bricklet.
 *
 * The {@link sound_pressure_level_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_SOUND_PRESSURE_LEVEL_DEVICE_IDENTIFIER 290

/**
 * \ingroup TF_SoundPressureLevel
 *
 * This constant represents the display name of a Sound Pressure Level Bricklet.
 */
#define TF_SOUND_PRESSURE_LEVEL_DEVICE_DISPLAY_NAME "Sound Pressure Level Bricklet"

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Creates the device object \c sound_pressure_level with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_sound_pressure_level_create(TF_SoundPressureLevel *sound_pressure_level, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Removes the device object \c sound_pressure_level from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_sound_pressure_level_destroy(TF_SoundPressureLevel *sound_pressure_level);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_sound_pressure_level_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_sound_pressure_level_get_response_expected(TF_SoundPressureLevel *sound_pressure_level, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Changes the response expected flag of the function specified by the
 * \c function_id parameter. This flag can only be changed for setter
 * (default value: *false*) and callback configuration functions
 * (default value: *true*). For getter functions it is always enabled.
 *
 * Enabling the response expected flag for a setter function allows to detect
 * timeouts and other error conditions calls of this setter as well. The device
 * will then send a response for this purpose. If this flag is disabled for a
 * setter function then no response is sent and errors are silently ignored,
 * because they cannot be detected.
 */
int tf_sound_pressure_level_set_response_expected(TF_SoundPressureLevel *sound_pressure_level, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_sound_pressure_level_set_response_expected_all(TF_SoundPressureLevel *sound_pressure_level, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_SoundPressureLevel
 *
 * Registers the given \c handler to the Decibel callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t decibel, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_sound_pressure_level_set_decibel_callback_configuration}.
 *
 * The parameter is the same as {@link tf_sound_pressure_level_get_decibel}.
 */
int tf_sound_pressure_level_register_decibel_callback(TF_SoundPressureLevel *sound_pressure_level, TF_SoundPressureLevel_DecibelHandler handler, void *user_data);


/**
 * \ingroup TF_SoundPressureLevel
 *
 * Registers the given \c handler to the Spectrum Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t spectrum_length, uint16_t spectrum_chunk_offset, uint16_t spectrum_chunk_data[30], void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_sound_pressure_level_set_spectrum_callback_configuration}.
 *
 * The parameter is the same as {@link tf_sound_pressure_level_get_spectrum}.
 */
int tf_sound_pressure_level_register_spectrum_low_level_callback(TF_SoundPressureLevel *sound_pressure_level, TF_SoundPressureLevel_SpectrumLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_SoundPressureLevel
 *
 * Registers the given \c handler to the Spectrum callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t *spectrum, uint16_t spectrum_length, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_sound_pressure_level_set_spectrum_callback_configuration}.
 *
 * The parameter is the same as {@link tf_sound_pressure_level_get_spectrum}.
 */
int tf_sound_pressure_level_register_spectrum_callback(TF_SoundPressureLevel *sound_pressure_level, TF_SoundPressureLevel_SpectrumHandler handler, uint16_t *spectrum, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_SoundPressureLevel
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_sound_pressure_level_callback_tick(TF_SoundPressureLevel *sound_pressure_level, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the measured sound pressure in decibels.
 *
 * The Bricklet supports the weighting standards dB(A), dB(B), dB(C), dB(D),
 * dB(Z) and ITU-R 468. You can configure the weighting with {@link tf_sound_pressure_level_set_configuration}.
 *
 * By default dB(A) will be used.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_sound_pressure_level_register_decibel_callback} callback. You can set the callback configuration
 * with {@link tf_sound_pressure_level_set_decibel_callback_configuration}.
 */
int tf_sound_pressure_level_get_decibel(TF_SoundPressureLevel *sound_pressure_level, uint16_t *ret_decibel);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * The period is the period with which the {@link tf_sound_pressure_level_register_decibel_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after the value has changed. If the value didn't change
 * within the period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 *
 * It is furthermore possible to constrain the callback with thresholds.
 *
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_sound_pressure_level_register_decibel_callback} callback.
 *
 * The following options are possible:
 *
 * \verbatim
 *  "Option", "Description"
 *
 *  "'x'",    "Threshold is turned off"
 *  "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
 *  "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
 *  "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
 *  "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"
 * \endverbatim
 *
 * If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
 */
int tf_sound_pressure_level_set_decibel_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t period, bool value_has_to_change, char option, uint16_t min, uint16_t max);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the callback configuration as set by {@link tf_sound_pressure_level_set_decibel_callback_configuration}.
 */
int tf_sound_pressure_level_get_decibel_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint16_t *ret_min, uint16_t *ret_max);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the frequency spectrum. The length of the spectrum is between
 * 512 (FFT size 1024) and 64 (FFT size 128). See {@link tf_sound_pressure_level_set_configuration}.
 *
 * Each array element is one bin of the FFT. The first bin is always the
 * DC offset and the other bins have a size between 40Hz (FFT size 1024) and
 * 320Hz (FFT size 128).
 *
 * In sum the frequency of the spectrum always has a range from 0 to
 * 20480Hz (the FFT is applied to samples with a frequency of 40960Hz).
 *
 * The returned data is already equalized, which means that the microphone
 * frequency response is compensated and the weighting function is applied
 * (see {@link tf_sound_pressure_level_set_configuration} for the available weighting standards). Use
 * dB(Z) if you need the unaltered spectrum.
 *
 * The values are not in dB form yet. If you want a proper dB scale of the
 * spectrum you have to apply the formula f(x) = 20*log10(max(1, x/sqrt(2)))
 * on each value.
 */
int tf_sound_pressure_level_get_spectrum_low_level(TF_SoundPressureLevel *sound_pressure_level, uint16_t *ret_spectrum_length, uint16_t *ret_spectrum_chunk_offset, uint16_t ret_spectrum_chunk_data[30]);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * The period is the period with which the {@link tf_sound_pressure_level_register_spectrum_callback} callback is
 * triggered periodically. A value of 0 turns the callback off.
 *
 * Every new measured spectrum will be send at most once. Set the period to 1 to
 * make sure that you get every spectrum.
 */
int tf_sound_pressure_level_set_spectrum_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t period);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the callback configuration as set by
 * {@link tf_sound_pressure_level_get_spectrum_callback_configuration}.
 */
int tf_sound_pressure_level_get_spectrum_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_period);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Sets the Sound Pressure Level Bricklet configuration.
 *
 * With different FFT sizes the Bricklet has a different
 * amount of samples per second and the size of the FFT bins
 * changes. The higher the FFT size the more precise is the result
 * of the dB(X) calculation.
 *
 * Available FFT sizes are:
 *
 * * 1024: 512 bins, 10 samples per second, each bin has size 40Hz
 * * 512: 256 bins, 20 samples per second, each bin has size 80Hz
 * * 256: 128 bins, 40 samples per second, each bin has size 160Hz
 * * 128: 64 bins, 80 samples per second, each bin has size 320Hz
 *
 * The Bricklet supports different weighting functions. You can choose
 * between dB(A), dB(B), dB(C), dB(D), dB(Z) and ITU-R 468.
 *
 * dB(A/B/C/D) are the standard dB weighting curves. dB(A) is
 * often used to measure volumes at concerts etc. dB(Z) has a
 * flat response, no weighting is applied. ITU-R 468 is an ITU
 * weighting standard mostly used in the UK and Europe.
 */
int tf_sound_pressure_level_set_configuration(TF_SoundPressureLevel *sound_pressure_level, uint8_t fft_size, uint8_t weighting);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the configuration as set by {@link tf_sound_pressure_level_set_configuration}.
 */
int tf_sound_pressure_level_get_configuration(TF_SoundPressureLevel *sound_pressure_level, uint8_t *ret_fft_size, uint8_t *ret_weighting);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the error count for the communication between Brick and Bricklet.
 *
 * The errors are divided into
 *
 * * ACK checksum errors,
 * * message checksum errors,
 * * framing errors and
 * * overflow errors.
 *
 * The errors counts are for errors that occur on the Bricklet side. All
 * Bricks have a similar function that returns the errors on the Brick side.
 */
int tf_sound_pressure_level_get_spitfp_error_count(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Sets the bootloader mode and returns the status after the requested
 * mode change was instigated.
 *
 * You can change from bootloader mode to firmware mode and vice versa. A change
 * from bootloader mode to firmware mode will only take place if the entry function,
 * device identifier and CRC are present and correct.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_sound_pressure_level_set_bootloader_mode(TF_SoundPressureLevel *sound_pressure_level, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the current bootloader mode, see {@link tf_sound_pressure_level_set_bootloader_mode}.
 */
int tf_sound_pressure_level_get_bootloader_mode(TF_SoundPressureLevel *sound_pressure_level, uint8_t *ret_mode);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Sets the firmware pointer for {@link tf_sound_pressure_level_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_sound_pressure_level_set_write_firmware_pointer(TF_SoundPressureLevel *sound_pressure_level, uint32_t pointer);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_sound_pressure_level_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_sound_pressure_level_write_firmware(TF_SoundPressureLevel *sound_pressure_level, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_sound_pressure_level_set_status_led_config(TF_SoundPressureLevel *sound_pressure_level, uint8_t config);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the configuration as set by {@link tf_sound_pressure_level_set_status_led_config}
 */
int tf_sound_pressure_level_get_status_led_config(TF_SoundPressureLevel *sound_pressure_level, uint8_t *ret_config);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_sound_pressure_level_get_chip_temperature(TF_SoundPressureLevel *sound_pressure_level, int16_t *ret_temperature);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_sound_pressure_level_reset(TF_SoundPressureLevel *sound_pressure_level);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_sound_pressure_level_write_uid(TF_SoundPressureLevel *sound_pressure_level, uint32_t uid);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_sound_pressure_level_read_uid(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_uid);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the UID, the UID where the Bricklet is connected to,
 * the position, the hardware and firmware version as well as the
 * device identifier.
 *
 * The position can be 'a', 'b', 'c', 'd', 'e', 'f', 'g' or 'h' (Bricklet Port).
 * A Bricklet connected to an :ref:`Isolator Bricklet <isolator_bricklet>` is always at
 * position 'z'.
 *
 * The device identifier numbers can be found :ref:`here <device_identifier>`.
 * |device_identifier_constant|
 */
int tf_sound_pressure_level_get_identity(TF_SoundPressureLevel *sound_pressure_level, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_SoundPressureLevel
 *
 * Returns the frequency spectrum. The length of the spectrum is between
 * 512 (FFT size 1024) and 64 (FFT size 128). See {@link tf_sound_pressure_level_set_configuration}.
 *
 * Each array element is one bin of the FFT. The first bin is always the
 * DC offset and the other bins have a size between 40Hz (FFT size 1024) and
 * 320Hz (FFT size 128).
 *
 * In sum the frequency of the spectrum always has a range from 0 to
 * 20480Hz (the FFT is applied to samples with a frequency of 40960Hz).
 *
 * The returned data is already equalized, which means that the microphone
 * frequency response is compensated and the weighting function is applied
 * (see {@link tf_sound_pressure_level_set_configuration} for the available weighting standards). Use
 * dB(Z) if you need the unaltered spectrum.
 *
 * The values are not in dB form yet. If you want a proper dB scale of the
 * spectrum you have to apply the formula f(x) = 20*log10(max(1, x/sqrt(2)))
 * on each value.
 */
int tf_sound_pressure_level_get_spectrum(TF_SoundPressureLevel *sound_pressure_level, uint16_t *ret_spectrum, uint16_t *ret_spectrum_length);

#ifdef __cplusplus
}
#endif

#endif
