/* *******************************************************************
 *
 *                Fraise device firmware v2.1
 *
 *********************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*********************************************************************
 * Copyright (c) Antoine Rousseau   nov 2015
 ****************************************************************** */

#ifndef FRUIT_H
#define FRUIT_H

/** @file */

/** @defgroup fruit Fruit module
 *  Fruit module implements @ref core, @ref fraisedevice and @ref eeparams.
 
 Any normal firmware should include Fruit module.
  
 *  Example :
 * \include fruit/examples/exampleFruit/main.c
 *  @{ */


#include <fraisedevice.h>
#include <eeparams.h>

/** @name Initialization */
/** @{ */
/** @brief Init fruit module at setup(). */
/** Init core, configure serial port as a fraise device.
*/
void fruitInit(void);
/** @} */

/** @name Service */
/** @{ */
/** @brief Fruit module service routine, to be called by main loop(). */
/** Check for input data from Fraise.*/
#define fruitService() fraiseService()
/** @} */ 

/** @} */
#endif //FRUIT_H

