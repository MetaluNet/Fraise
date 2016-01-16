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

 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) Antoine Rousseau   nov 2015
 ****************************************************************** */

#ifndef FRUIT_H
#define FRUIT_H

/** @file */

/** @defgroup fruit Fruit module.
 *  Fruit module implements core, fraisedevice and eeparams modules.
 *  @{
 */


#include <fraisedevice.h>
#include <eeparams.h>

/** @brief Init fruit module. */
/** Init core, configure serial port as a fraise device.
*/
void fruitInit(void);

/** @brief Fruit module service routine. */
/** Check for input data from Fraise.
*/
#define fruitService() fraiseService()

/** @} 
*/
#endif //FRUIT_H

