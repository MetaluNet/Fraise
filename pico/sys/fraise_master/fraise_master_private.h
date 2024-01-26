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

#define SetBit(A, k)     ( A[(k) / 32] |=  (1 << ((k) % 32)) )
#define ClearBit(A, k)   ( A[(k) / 32] &= ~(1 << ((k) % 32)) )
#define TestBit(A, k)    ( (A[(k) / 32] &   (1 << ((k) % 32))) != 0)
_FRAISE_PRIVATE_EXTERN uint32_t polled[MAX_FRUITS / 32];
_FRAISE_PRIVATE_EXTERN uint32_t detected[MAX_FRUITS / 32];

#define set_polled(k) SetBit(polled, k)
#define clear_polled(k) ClearBit(polled, k)
#define is_polled(k) TestBit(polled, k)

#define set_detected(k) SetBit(detected, k)
#define clear_detected(k) ClearBit(detected, k)
#define is_detected(k) TestBit(detected, k)

#endif // _FRAISE_MASTER_PRIVATE_H

