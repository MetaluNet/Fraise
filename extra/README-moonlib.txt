MOONLIB puredata externals

    tabenv : like env~, an enveloppe follower, but computing on a table, so 
	 possibly much speeder than real-time env~'s computation.
    tabsort and tabsort2 : returns the indices of the sorted table (tabsort2 
	 is bidimentionnal).
    gamme : one octave of a piano keyboard used to filter/choose notes in a 
	 selected scale.
    absolutepath/relativepath : to use datas (sounds, texts, presets, images, 
	 programs...) nested in the patch's directory (and in subdirs).
    sarray and slist : to creates shared dynamic arrays or lists with symbols.
    sfread2~ and readsfv~ : to pitch the direct-from-disk reading of sound files.
    dinlet~ : an inlet~ with a default value (when nothing is connected to it).
    mknob : a round knob ala iemgui vslider (with its "properties" window).
    dispatch : creates one bus name for many buttons' buses: from the N pairs  
	 (slider1-snd/slider1-rcv) ... (sliderN-snd/sliderN-rcv), creates only 
	 one pair of buses named (slider-snd/slider-rcv) , in which datas are 
	 prepended by the number of the "sub-bus".
    joystik : an improvment of Joseph A. Sarlo's joystick.
    image :  an improvment  of Guenter Geiger's one. Same name, but it's 
	 compatible. Here you can share images through different objects, preload 
	 a list of images, and animate this list.
    ndmetro : a metronome/counter allowing continuous tempo change and other features.
    
    and some others...


  In order to have sfread2~ working with big files in direct-from-disk 
mode you have to hack pd sources: change 
	mlockall(MCL_FUTURE) 
with 
	mlockall(MCL_CURRENT)
in s_inter.c . If not the whole file will be loaded in memory when opening it.

Latest source can be found (and issues can be reported) at: 
https://github.com/MetaluNet/moonlib

Antoine Rousseau

