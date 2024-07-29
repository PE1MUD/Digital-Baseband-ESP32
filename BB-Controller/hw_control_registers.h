//--------------------------------------------------------------------
//  Hardware and configuration register maps
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------

#ifndef HW_CONTROL_REGISTERS_H_
#define HW_CONTROL_REGISTERS_H_

#include <stdint.h>

// GPIO pins
#define GPIO_MASK_SW1	(1 << 6)
#define GPIO_MASK_SW2	(1 << 7)


//-----------------------------------------------------------------------
// FPGA hardware registers (write)
//-----------------------------------------------------------------------
typedef struct __attribute__((__packed__)) {
	int16_t video_preemp_a1;
	int16_t video_preemp_a2;

	int16_t video_preemp_b0;
	int16_t video_preemp_b1;

	int16_t video_preemp_b2;
	int16_t spare;

	uint32_t fm1_carrier_frequency;
	uint32_t fm2_carrier_frequency;
	uint32_t fm3_carrier_frequency;
	uint32_t fm4_carrier_frequency;

	uint16_t fm1_level : 10;
	uint16_t fm1_spare : 4;
	uint16_t fm1_fm_ena : 1;
	uint16_t fm1_am_ena : 1;

	uint16_t fm2_level : 10;
	uint16_t fm2_spare : 4;
	uint16_t fm2_fm_ena : 1;
	uint16_t fm2_am_ena : 1;

	uint16_t fm3_level : 10;
	uint16_t fm3_spare : 4;
	uint16_t fm3_fm_ena : 1;
	uint16_t fm3_am_ena : 1;

	uint16_t fm4_level : 10;
	uint16_t fm4_spare : 4;
	uint16_t fm4_fm_ena : 1;
	uint16_t fm4_am_ena : 1;

	uint32_t fm_spare : 16;	 // reg 9, d15..0

	uint32_t fm1_preemp : 2;
	uint32_t fm1_bandwidth : 2;
	uint32_t fm2_preemp : 2;
	uint32_t fm2_bandwidth : 2;
	uint32_t fm3_preemp : 2;
	uint32_t fm3_bandwidth : 2;
	uint32_t fm4_preemp : 2;
	uint32_t fm4_bandwidth : 2;

	uint32_t nicam_frequency : 11;
	uint32_t nicam_control : 4;
	uint32_t nicam_mode : 2;
	uint32_t nicam_scramble_init : 9;
	uint32_t spare2 : 6;

	uint16_t nicam_level;
	uint16_t reg11b;

	// reg12
	uint32_t audio_nco_frequency : 14;
	uint32_t audio_nco_waveform : 2;
	uint16_t reg12_spare : 16;

	uint32_t reg13;
	uint32_t reg14;

	// Register 15
	uint32_t video_overlay_enable : 1;
	uint32_t reset_peaks : 1;
	uint32_t generator_enable : 1;
	uint32_t video_filter_bypass : 1;
	uint32_t video_pattern_enable : 1;
	uint32_t spare3 : 27;
} HW_SETTINGS;


//-----------------------------------------------------------------------
// FPGA hardware registers (read)
//-----------------------------------------------------------------------
typedef struct __attribute__((__packed__)) {
	uint16_t adc1_left_audio_peak;
	uint16_t adc1_right_audio_peak;

	uint16_t adc2_left_audio_peak;
	uint16_t adc2_right_audio_peak;

	uint16_t fm1_audio_peak;
	uint16_t fm2_audio_peak;

	uint16_t fm3_audio_peak;
	uint16_t fm4_audio_peak;

	uint32_t vid_adc_clip : 1;
	uint32_t vid_low_pass_clip : 1;
	uint32_t vid_preemp_clip : 1;
	uint32_t nicam_upsampling_clip : 1;
	uint32_t baseband_clip : 1;
	uint32_t spare1 : 27;

	uint16_t adc_in_min;	// 0..1023
	uint16_t adc_in_max;	// 0..1023

	uint16_t dac_out_min;	// 0..4095
	uint16_t dac_out_max;	// 0..4095

	uint32_t nicam_reset : 1;
	uint32_t baseband_pll_locked : 1;
	uint32_t spare2: 30;

	uint16_t nicam_left_peak;
	uint16_t nicam_right_peak;
} HW_INPUTS;


//-----------------------------------------------------------------------
// Control registers
//-----------------------------------------------------------------------
enum VIDEO_MODE { FLAT, PAL, NTSC, SECAM };
enum VIDEO_INPUT { VIDEO_IN, VIDEO_GENERATOR, VIDEO_IN_AUTO };	// auto: enable generator on low input level
enum OSD_MODE { OSD_OFF, OSD_ON, OSD_AUTO };	// auto: enable OSD on low input level (combine with VIDEO_IN_AUTO)
enum FM_BANDWIDTH { BW_130, BW_180, BW_230, BW_280 };
enum AUDIO_INPUT { ADC1L, ADC1R, ADC2L, ADC2R, I2S1L, I2S1R, I2S2L, I2S2R, ADC1LR, ADC2LR, I2S1LR, I2S2LR, MUTE };
enum PREEMPHASIS { AUDIO_50US, AUDIO_75US, AUDIO_J17, AUDIO_FLAT};
enum NICAM_BANDWIDTH { BW_700 = 0, BW_500 = 1 };
enum NCO_MODE { NCO_CW, NCO_MORSE };
enum NCO_WAVEFORM { SINE, SQUARE, NOISE };

typedef struct __attribute__((__packed__)) {
	uint32_t video_level : 8;  		// 0..255, 128=1x
	enum VIDEO_MODE video_mode : 2;
	uint32_t invert_video : 1;
	enum OSD_MODE osd_mode : 2;		// Show OSD (free overlay and/or menu)
	enum VIDEO_INPUT video_in : 2;	// Select video input or internal sync generator
	uint32_t filter_bypass : 1;		// Bypass lowpass filter (do not bypass when using OSD or patterns!)
	uint32_t show_menu : 1;         // Show menu when osd is enabled
	uint32_t pattern_enable : 1;	// Global enable for VIT and other 'pattern' lines
	uint32_t enable : 1;			// Enable/disable video path
} VIDEO_SETTINGS;

typedef struct __attribute__((__packed__)) {
	int16_t rf_frequency_khz;
	int16_t rf_level;	// 0..1023
	enum AUDIO_INPUT input : 4;
	enum PREEMPHASIS preemphasis : 2;
	enum FM_BANDWIDTH bandwidth : 3;
	uint32_t generator_ena : 1;
	uint32_t generator_level : 4;  // 0 = FS, 1 = -6dB, 2 = -12dB, ...
	uint32_t am : 1;
	uint32_t enable : 1;
} FM_SETTINGS;

typedef struct __attribute__((__packed__)) {
	int16_t rf_frequency_khz;
	int16_t rf_level;	// 0..1023
	enum AUDIO_INPUT input_ch1 : 4;
	enum AUDIO_INPUT input_ch2 : 4;
	uint32_t generator_level_ch1 : 4;  // 0 = FS, 1 = -6dB, 2 = -12dB, ...
	uint32_t generator_level_ch2 : 4;  // 0 = FS, 1 = -6dB, 2 = -12dB, ...
	uint32_t generator_ena_ch1 : 1;
	uint32_t generator_ena_ch2 : 1;
	enum NICAM_BANDWIDTH bandwidth: 1;
	uint32_t enable : 1;
} NICAM_SETTINGS;

typedef struct __attribute__((__packed__)) {
	char morse_message[16];
	uint32_t audio_nco_frequency : 14;	// in Hz
	enum NCO_WAVEFORM audio_nco_waveform : 2;
	enum NCO_MODE audio_nco_mode : 2;
	uint32_t morse_speed : 2;	// 0 = fast, 3 = slow
	uint32_t morse_message_repeat_time : 10;  // in seconds, max 1023
	uint32_t spare : 2;
	uint32_t last_recalled_presetnr : 8;  // set on preset recall
	uint32_t user_setting1 : 8;
} GENERAL_SETTINGS;

typedef struct __attribute__((__packed__)) {
	char name[12];
	FM_SETTINGS fm[4];
	NICAM_SETTINGS nicam;
	VIDEO_SETTINGS video;
	GENERAL_SETTINGS general;
} SETTINGS;


#endif /* HW_CONTROL_REGISTERS_H_ */
