# music-library-for-esp32
A library for music creation. 

Has the following five functions:
setup_buzzers()
play_melody()
play_chord()
play_tone()
stop_tone()

and uses three structs melody_config_t, chord_config_t, and music_config_t.

Good website for finding note-names:
https://sengpielaudio.com/calculator-notenames.htm




void setup_buzzers(music_config_t music_conf)
Function for setting up buzzers, which takes a music_config struct.



play_melody(melody_config_t melody_list):

play_tone():

Play_tone takes