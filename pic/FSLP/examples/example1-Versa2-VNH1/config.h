#ifndef _CONFIG_H_
#define _CONFIG_H_

// analog settings:
#define ANALOG_FILTER 3 
#define ANALOG_THRESHOLD 10
#define ANALOG_CHANNEL_PRECONF FSLP_analog_preconf

// FSLP settings:
#define FSLP_D1 MAEN
#define FSLP_D2 MA1
#define FSLP_SL MOTA_CURRENT
#define FSLP_R0 MAPWM // R0 must be connected to SL through 4.7k

/*
VNH1:
1(GND)
2(VDD)
3(MA2)
4(MAEN2)
5(MOTA_CURRENT)-----------|       -------------------------------
6(MAPWM)--------[ 4.7k ]--|--(SL)-|                             |
7(MAEN)--------\/------------(D2)-|  0   0   0   0   0   0   0  |
8(MA1)---------/\------------(D1)-|                             |
                                  -------------------------------

#define FSLP_D2CHAN 0
#define FSLP_SLCHAN 1
#define FSLP_SLCHAN2 2

#endif // _CONFIG_H_

