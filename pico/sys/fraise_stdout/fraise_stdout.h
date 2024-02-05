/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_STDOUT_H
#define _FRAISE_STDOUT_H

/** \file fraise_stdout.h
 *  \defgroup fraise_stdout fraise_stdout
 *
 * Setup Fraise output as a regular stdout
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enable or disable fraise_stdout the eeprom
 * \ingroup fraise_stdout
 */
void fraise_stdout_set_enabled(bool enable);

#ifdef __cplusplus
}
#endif

#endif // _FRAISE_STDOUT_H
