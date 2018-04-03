/*********************************************************************
 *
 *                KT403 MP3 player for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  mar 2018     Original.
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

// adapted from:
/*
 * MP3Player_KT403A.h
 * A library for Grove-Serial MP3 Player V2.0
 *
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Wuruibin
 * Created Time: Dec 2015
 * Modified Time:
 * 
 * The MIT License (MIT)
 */

#ifndef _KT403_H
#define _KT403_H
void kt403_Init();
void kt403_lowISR();

void kt403_SelectPlayerDevice(byte device);
void kt403_SpecifyMusicPlay(word index);
void kt403_SpecifyfolderPlay(byte folder, byte index);
void kt403_PlayPause(void);
void kt403_PlayResume(void);
void kt403_PlayNext(void);
void kt403_PlayPrevious(void);
void kt403_PlayLoop(byte loop);
void kt403_Stop(void);
void kt403_SetVolume(byte volume);
void kt403_IncreaseVolume(void);
void kt403_DecreaseVolume(void);
byte kt403_IsPlaying(void);

void kt403_printStatus(void);

#endif

