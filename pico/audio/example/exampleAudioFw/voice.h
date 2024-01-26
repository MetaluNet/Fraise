#include <math.h>

void voice_setup();
int32_t voice_getSample();
void voice_update(); // every 256 samples
void voice_addNote(uint32_t step, uint8_t vol, uint16_t release, uint16_t lfoamp, uint16_t lfofreq);
void voice_clearNotes();
uint16_t voice_getNbNotes();
int16_t voice_getCos(uint16_t phase);
void voice_debug();
