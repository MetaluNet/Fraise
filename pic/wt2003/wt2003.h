/*********************************************************************
 *
 *                WT2003S MP3 player for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  june 2020     Original.
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/

#ifndef _WT2003_H
#define _WT2003_H
void wt2003_Init();
void wt2003_lowISR();
void wt2003_deInit();

//void wt2003_SelectPlayerDevice(byte device);
void wt2003_SpecifyMusicPlay(word index);
//void wt2003_SpecifyfolderPlay(byte folder, byte index);
void wt2003_PlayPause(void);
void wt2003_PlayNext(void);
void wt2003_PlayPrevious(void);
void wt2003_PlayLoop(byte loop); // 0:no_loop ; 1:single ; 2:all ; 3:random
void wt2003_Stop(void);
void wt2003_SetVolume(byte volume);
byte wt2003_IsPlaying(void);

void wt2003_printStatus(void);

#endif

