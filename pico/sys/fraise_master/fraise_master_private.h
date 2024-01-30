/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_MASTER_PRIVATE_H
#define _FRAISE_MASTER_PRIVATE_H

#define MAX_FRUITS 128

#ifndef _FRAISE_PRIVATE_DEFINE_VARS
#define _FRAISE_PRIVATE_EXTERN extern
#else 
#define _FRAISE_PRIVATE_EXTERN
#endif

/*#define SetBit(A, k)     ( A[(k) / 32] |=  (1 << ((k) % 32)) )
#define ClearBit(A, k)   ( A[(k) / 32] &= ~(1 << ((k) % 32)) )
#define TestBit(A, k)    ( (A[(k) / 32] &   (1 << ((k) % 32))) != 0)
_FRAISE_PRIVATE_EXTERN uint32_t polled[MAX_FRUITS / 32];
_FRAISE_PRIVATE_EXTERN uint32_t detected[MAX_FRUITS / 32];

#define set_polled(k) SetBit(polled, k)
#define clear_polled(k) ClearBit(polled, k)
#define is_polled(k) TestBit(polled, k)

#define set_detected(k) SetBit(detected, k)
#define clear_detected(k) ClearBit(detected, k)
#define is_detected(k) TestBit(detected, k)*/
typedef struct {
	bool detected:1;
} fruit_vstate_t;

typedef struct {
	bool polled:1;
	bool detected_sent:1;
} fruit_state_t;

_FRAISE_PRIVATE_EXTERN volatile fruit_vstate_t fruit_vstates[MAX_FRUITS];
_FRAISE_PRIVATE_EXTERN fruit_state_t fruit_states[MAX_FRUITS];

static inline void set_polled(uint8_t id, bool v) {
	fruit_states[id].polled = v;
}
static inline bool is_polled(uint8_t id) {
	return fruit_states[id].polled;
}

static inline void set_detected(uint8_t id, bool v) {
	fruit_vstates[id].detected = v;
}
static inline bool is_detected(uint8_t id) {
	return fruit_vstates[id].detected;
}

static inline void set_detected_sent(uint8_t id, bool v) {
	fruit_states[id].detected_sent = v;
}
static inline bool is_detected_sent(uint8_t id) {
	return fruit_states[id].detected_sent;
}

#endif // _FRAISE_MASTER_PRIVATE_H

