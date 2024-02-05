// Audio example

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "fraise.h"
#include "pico/audio_pwm.h"
#include "voice.h"
#include "echo.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
int ledPeriod = 250;

// ---------------------------------------------------------------------
#define SAMPLES_PER_BUFFER 256

struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
            .sample_freq = 24000,
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .channel_count = 1,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    output_format = audio_pwm_setup(&audio_format, -1, &default_mono_channel_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    ok = audio_pwm_default_connect(producer_pool, false);
    assert(ok);
    audio_pwm_set_enabled(true);
   return producer_pool;
}

// ---------------------------------------------------------------------
struct audio_buffer_pool *ap;
uint vol = 128;

Echo<19997> echo1;
Echo<27001> echo2;

void setup() {
	//set_sys_clock_48mhz();
	voice_setup();
	ap = init_audio();
}

#define clip(s) ((s) > 32767 ? 32767 : (s) < -32767 ? -32767 : (s))
void loop(){
	static absolute_time_t nextLed;// = make_timeout_time_ms(100);
	static bool led = false;

	gpio_put(LED_PIN, 0);
	/*if(absolute_time_min(nextLed, get_absolute_time()) == nextLed) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}*/

	struct audio_buffer *buffer = take_audio_buffer(ap, true);
	int16_t *samples = (int16_t *) buffer->buffer->bytes;
	int32_t s;
	for (uint i = 0; i < buffer->max_sample_count; i++) {
		s = ((int32_t)vol * voice_getSample()) >> 8u; s = clip(s);
		s = s + echo1.getSample(s); s = clip(s);
		s = s + echo2.getSample(s); s = clip(s);
		samples[i] = s;
	}
	buffer->sample_count = buffer->max_sample_count;
	give_audio_buffer(ap, buffer);
	voice_update();
}

void fraise_receivebytes(const char *data, uint8_t len){
	if(data[0] == 1) ledPeriod = (int)data[1] * 10;
	else if(data[0] == 2) { // add note (32bit step + 8bit vol)
		int i = 1;
		uint32_t step = (data[i++] << 24) + (data[i++] << 16) + (data[i++] << 8) + (data[i++] << 0);
		uint8_t vol = data[i++];
		uint16_t release = (data[i++] << 8) + (data[i++] << 0);
		uint16_t lfoamp = (data[i++] << 8) + (data[i++] << 0);
		uint16_t lfofreq = (data[i++] << 8) + (data[i++] << 0);
		voice_addNote(step, vol, release, lfoamp, lfofreq);
	}
	else if(data[0] == 3) { // clear notes
		voice_clearNotes();
	}
	else if(data[0] == 4) { // vol (8bit)
		vol = (uint8_t) data[1];
	}
	else if(data[0] == 5) { // mode (8bit)
		enum audio_correction_mode m;
		switch(data[1]) {
			case 0: m = none; break;
			case 1: m = fixed_dither; break;
			case 2: m = dither; break;
			case 3: m = noise_shaped_dither; break;
			default: return;
		}
		audio_pwm_set_correction_mode(m);
	}
	else if(data[0] == 6) { // get nb notes
		printf("nbnotes %d\n", voice_getNbNotes());
	}
	else if(data[0] == 8) { // debug
		voice_debug();
	}
	else if(data[0] == 9) { // echo
		int i = 1;
		uint16_t feedback = (data[i++] << 8) + (data[i++] << 0);
		uint16_t mix = (data[i++] << 8) + (data[i++] << 0);
		echo1.config(feedback, mix);
		echo2.config(feedback, mix);
	}
	else {
		printf("rcvd ");
		for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]);
		putchar('\n');
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

