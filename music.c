#include <stdio.h>
#include "driver/ledc.h"
#include "math.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/// @brief struct for creating a melody. notes are defined by key number (40 is middle c) and t, s, e, q, h, w.
// Dotted if last char is ".".
///@param notes
/// example: 40e. -> 40 = middle c, e == eightnote, . == dotted. will play for (qt / 2 + qt/4) ms.
typedef struct melody_config
{
    char notes[32][4];

} melody_config_t;

/// @brief configuration of chord. takes the follow parameters:
/// @param root
/// the note note character.
/// @param oct
/// the octave. three and less than three doesnt play.
/// @param type
///  -2 == diminished.  -1 == minor. 0 == default. 1 == augmented. 2 == b7. 3 == 7
///@param sharp_flat
///
///@param time
///
///@param intensity
///
typedef struct chord_config
{
    char root;
    int oct;
    int type;
    int sharp_flat; //-1, 1 of root
    int time;
    int intensity;

} chord_config_t;

/// @brief configuration of buzzers. takes the follow parameters:
/// @param buzzers
/// amounts of buzzers on the board.
/// @param BUZZ1_PIN
/// the pins of the buzzers.
///@param BUZZ2_PIN
///@param BUZZ3_PIN
///@param ch1
/// the channels (and timers!) of the buzzers.
///@param ch2
///@param ch3
typedef struct music_config
{
    int buzzers;
    int BUZZ1_PIN;
    int BUZZ2_PIN;
    int BUZZ3_PIN;
    int ch1;
    int ch2;
    int ch3;
} music_config_t;

//------------------------------INTERNAL MUSIC CALCULATIONS-------------------------------------------//
/// @brief For internal use. Calculates the root.
/// @param steps amounts of steps from A. calculated in the chord() function.
/// @param oct what octave to take it from.
/// @return returns freq1 (as an integer).
int root_calculation(int steps, int oct)
{
    int A_tones[5] = {220, 440, 880, 1760, 3520};
    int freq1 = A_tones[oct - 3];
    freq1 = freq1 * pow(2, (float)steps / 12.0);
    return freq1;
}

/// @brief calculation of frequencies of the other tones, based on root. returns freq as integer.
/// @param root
/// the root frequency.
/// @param steps
/// the steps from root to calculate to.
/// @return
int freq_calculation(int root, int steps)
{
    return root * pow(2, steps / 12.0);
}

//---------------------------------------------END OF INTERNALS-------------------------------------------//

//-----------------------------------------------BUZZER SETUP---------------------------------------------//

/// @brief setting up buzzers. def_buzz_pins should be run first. channels will be used in order.
// if channel should not be used, give it channel -1.
void setup_buzzers(music_config_t music_conf)
{

    if (music_conf.buzzers == 0)
    { // if buzzers == 0, def_buzz_pins likely wasn't called before calling this...
        printf("buzzers variable is 0. did you run def_buzz_pins?...\n");
    }
    if (music_conf.ch1 != -1)
    {
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = 13,
            .timer_num = music_conf.ch1,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK};
        ledc_timer_config(&ledc_timer);
        ledc_channel_config_t ledc_channel_cfg = {
            .gpio_num = music_conf.BUZZ1_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = music_conf.ch1,
            .timer_sel = music_conf.ch1,
            .duty = 0,
            .hpoint = 0};
        ledc_channel_config(&ledc_channel_cfg);
    }
    if ((music_conf.ch2 != -1) && (music_conf.ch2 != music_conf.ch1))
    {
        ledc_timer_config_t ledc_timer2 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = 13,
            .timer_num = music_conf.ch2,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK};
        ledc_timer_config(&ledc_timer2);
        ledc_channel_config_t ledc_channel_cfg2 = {
            .gpio_num = music_conf.BUZZ2_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = music_conf.ch2,
            .timer_sel = music_conf.ch2,
            .duty = 0,
            .hpoint = 0};
        ledc_channel_config(&ledc_channel_cfg2);
    }
    if ((music_conf.ch3 != -1) && (music_conf.ch3 != music_conf.ch2) && (music_conf.ch3 != music_conf.ch1))
    {
        ledc_timer_config_t ledc_timer3 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = 13,
            .timer_num = music_conf.ch3,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK};
        ledc_timer_config(&ledc_timer3);
        ledc_channel_config_t ledc_channel_cfg3 = {
            .gpio_num = music_conf.BUZZ3_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = music_conf.ch3,
            .timer_sel = music_conf.ch3,
            .duty = 0,
            .hpoint = 0};
        ledc_channel_config(&ledc_channel_cfg3);
    }
}
/// @brief function for playing a singular tone. for melodies interrupted by other functions,
/// or for letting tones play indefinetly. (the one used in this project...)
/// @param music_conf
/// the music_config struct with buzzer pins and channel numbers.
///@param buzzer
/// the buzzer which is to play the given tone.
/// @param tone
/// the tone to play.
/// @param time
/// the time to play. if -1, tone will start and wont stop until stop_tone() is called.
void play_tone(music_config_t music_conf, int buzzer, int tone, int time)
{
    /*
    It should be noted, that currently the intensity of the single tone playing is hardcoded to 2000.
    This is because it was sufficient for our use; and the function already has quite a few parameters,
    and simplicity was preferred.
    */
    int tone_to_play = freq_calculation(440, tone - 49);
    int channel = 0;
    switch (buzzer)
    {
    case 1:
        channel = music_conf.ch1;
        break;
    case 2:
        channel = music_conf.ch2;
        break;
    case 3:
        channel = music_conf.ch3;
        break;

    default:
        printf("buzzer was an illegal argument... using buzzer 1.");
        channel = music_conf.ch1;
        break;
    }
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, 2000);
    ledc_set_freq(LEDC_LOW_SPEED_MODE, channel, tone_to_play);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
    if (time != -1)
    {
        vTaskDelay(time);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
    }
    return;
}

/// @brief a function to stop all currently playing tones. used in tandem with play_tone.
/// @param music_conf
/// the music_config struct with buzzer pins and channel numbers.
void stop_tone(music_config_t music_conf)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch2, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch2);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch3, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch3);
}

/// @brief function to play melody.
/// @param melody_conf
/// the struct of the melody.
/// @param default_q_time
/// the default quarternote time. essentially the bpm, but not really. :)
void play_melody(music_config_t music_conf, melody_config_t melody_conf, int qt)
{

    int mel_gen[16][2];
    float note_time;
    for (int i = 0; i < 16; i++)
    {
        mel_gen[i][0] = freq_calculation(440, ((melody_conf.notes[i][0] - 48) * 10 + (melody_conf.notes[i][1] - 48)));
        switch (melody_conf.notes[i][2])
        {
        case 119: //'w'
            note_time = 4.0;
            break;
        case 68: //'h'
            note_time = 2.0;
            break;
        case 71: // 'q'
            note_time = 1.0;
            break;
        case 65: // 'e'
            note_time = 0.5;
            break;
        case 73: // 's'
            note_time = 0.25;
            break;
        case 74: // 't'
            note_time = 0.125;
            break;
        default:
            note_time = 1.0;
            break;
        }
        if (melody_conf.notes[i][3] == 46) // dotted
        {
            note_time += note_time / 2;
        }
        mel_gen[i][1] = note_time;
    }

    for (int i = 0; i < 16; i++)
    {
        if (mel_gen[i][0] != 0)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1, 2000);
            ledc_set_freq(LEDC_LOW_SPEED_MODE, 0, mel_gen[i][0]);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1);
            vTaskDelay(pdMS_TO_TICKS((int)(qt * note_time)));
            ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch1, 0);
        }
    }
}

void play_chord(music_config_t music_conf, chord_config_t chord)
{
    volatile int freq1;
    volatile int freq2;
    volatile int freq3;
    // first check if chord call was legal character.

    switch (chord.root)
    {
    case 'A':
        freq1 = root_calculation(0 + chord.sharp_flat, chord.oct);

        break;

    case 'B':
        freq1 = root_calculation(2 + chord.sharp_flat, chord.oct);
        break;

    case 'C':
        freq1 = root_calculation(3 + chord.sharp_flat, chord.oct);
        break;

    case 'D':
        freq1 = root_calculation(5 + chord.sharp_flat, chord.oct);
        break;

    case 'E':
        freq1 = root_calculation(7 + chord.sharp_flat, chord.oct);
        break;

    case 'F':
        freq1 = root_calculation(8 + chord.sharp_flat, chord.oct);
        break;

    case 'G':
        freq1 = root_calculation(10 + chord.sharp_flat, chord.oct);
        break;

    default:
        printf("chord was not a valid tone...");
        return;
    }
    int freq2_steps = chord.type == (-1 || -2) ? 3 : 4;
    int freq3_steps = chord.type == -2 ? 6 : chord.type == (-1 || 0) ? 7
                                         : chord.type == 1           ? 8
                                         : chord.type == 2           ? 10
                                                                     : 11;

    ///  -2    |    -1    |    0    |   1   |   2   |   3   |
    /// dim    |   minor  |  major  |  aug  |  b7   |   7   |
    if (music_conf.buzzers == 3)
    {
        freq2 = freq_calculation(freq1, freq2_steps);
        freq3 = freq_calculation(freq1, freq3_steps);
    }
    else if (music_conf.buzzers == 2)
    {
        freq2 = freq_calculation(freq1, freq3_steps);
    }

    chord.intensity = 750 * chord.intensity;

    if (music_conf.ch1 != -1)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1, chord.intensity);
        ledc_set_freq(LEDC_LOW_SPEED_MODE, 0, freq1);
    }
    if (music_conf.ch2 != -1)
    {

        ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch2, chord.intensity);
        ledc_set_freq(LEDC_LOW_SPEED_MODE, 1, freq2);
    }
    if (music_conf.ch3 != -1)
    {

        ledc_set_duty(LEDC_LOW_SPEED_MODE, music_conf.ch3, chord.intensity);
        ledc_set_freq(LEDC_LOW_SPEED_MODE, 2, freq3);
    }
    // debug of frequency calculations and channel calls.
    // printf("freq1 - 3 is here %d %d %d.\n", freq1, freq2, freq3);
    // printf("ch1-3 is here: %d %d %d\n", music_conf.ch1, music_conf.ch2, music_conf.ch3);

    switch (music_conf.buzzers)
    {
    case 3:
        ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch2);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch3);
        vTaskDelay(pdMS_TO_TICKS(chord.time));
        ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch1, 0);
        ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch2, 0);
        ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch3, 0);
        break;

    case 2:
        ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch2);
        vTaskDelay(pdMS_TO_TICKS(chord.time));
        ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch1, 0);
        ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch2, 0);
        break;

    case 1:
        ledc_update_duty(LEDC_LOW_SPEED_MODE, music_conf.ch1);
        vTaskDelay(pdMS_TO_TICKS(chord.time));
        ledc_stop(LEDC_LOW_SPEED_MODE, music_conf.ch1, 0);
    }
}

void app_main()
{
    //------END OF MUSIC LIB----//
    //---------------------------------------------------------------------------------------------------------------------------//

    music_config_t buzzer_config_t = {
        .BUZZ1_PIN = 10,
        .BUZZ2_PIN = 7,
        .BUZZ3_PIN = 1,
        .buzzers = 3,
        .ch1 = 0,
        .ch2 = 1,
        .ch3 = 2};
    melody_config_t intro_melody = {
        // CREATE!!!
        .notes = {""},
    };
    setup_buzzers(buzzer_config_t);
}
