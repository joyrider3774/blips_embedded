#include <stdint.h>
#include <string.h>
#include "Sound.h"
#include "Common.h"


uint8_t music_note, music_tempo, music_loop, music_on, sound_on;
uint8_t selecting_music;
//the selected tune, played straight from flash instead of being copied into ram.
//NULL when none was selected
static const uint16_t* music_data = NULL;
//number of values in the selected tune, music_note steps through it 2 at a time
uint8_t music_length;

const float sfxSustain = (100.0f * 30.0f / 18.0f);
//constexpr so the tunes below are filled in by the compiler: data in flash can not be
//written by initialisation code at startup
constexpr float musModifier = (60.0f / 30.0f);

// Winner
const uint16_t music_winner[] PLATFORM_PROGMEM = {
    523,  (uint16_t)(100.0f / musModifier),
    659,  (uint16_t)(100.0f / musModifier),
    783,  (uint16_t)(100.0f / musModifier),
    1046, (uint16_t)(300.0f / musModifier),
    1318, (uint16_t)(500.0f / musModifier),
    0, 0
};

// Coin
const uint16_t music_coin[] PLATFORM_PROGMEM = {
    988,  (uint16_t)(60.0f / musModifier),
    1568, (uint16_t)(150.0f / musModifier),
    0, 0
};

// Explosion
const uint16_t music_explosion[] PLATFORM_PROGMEM = {
    220, (uint16_t)(40.0f / musModifier),
    180, (uint16_t)(40.0f / musModifier),
    150, (uint16_t)(40.0f / musModifier),
    120, (uint16_t)(40.0f / musModifier),
    90,  (uint16_t)(50.0f / musModifier),
    60,  (uint16_t)(60.0f / musModifier),
    40,  (uint16_t)(90.0f / musModifier),
    0, 0
};

//clear
const uint16_t music_clear[] PLATFORM_PROGMEM = {
    523,  (uint16_t)(100.0f / musModifier),  // C5
    659,  (uint16_t)(100.0f / musModifier),  // E5
    784,  (uint16_t)(100.0f / musModifier),  // G5
    1047, (uint16_t)(150.0f / musModifier),  // C6
    1319, (uint16_t)(200.0f / musModifier),  // E6
    0, 0
};

//the length of a tune is kept in the uint8_t music_length, at most 254 values keeps
//music_note (stepping 2 at a time past the end) from wrapping
#define TUNELEN(t) (sizeof(t) / sizeof(uint16_t))
static_assert((TUNELEN(music_winner) <= 254) && (TUNELEN(music_coin) <= 254) && (TUNELEN(music_explosion) <= 254) && (TUNELEN(music_clear) <= 254), "a tune does not fit in music_length");

void stopMusic(void)
{
    
}

void setMusicOn(uint8_t value)
{
    music_on = value;
}

void setSoundOn(uint8_t value)
{
    sound_on = value;
}

uint8_t isMusicOn(void)
{
    return music_on;
}

uint8_t isSoundOn(void)
{
    return sound_on;
}


void initSound(void)
{
    sound_on = 0;
}

void deInitSound(void)
{

}

uint8_t isMusicPlaying()
{
	return (music_note < music_length);
}

void SelectMusic(uint8_t musicFile, uint8_t loop)
{
    selecting_music = 1;
    switch (musicFile) 
    {			
        case musWinner:
            music_data = music_winner;
            music_length = TUNELEN(music_winner);
            break;
        case musClear:
            music_data = music_clear;
            music_length = TUNELEN(music_clear);
            break;
        case musCoin:
            music_data = music_coin;
            music_length = TUNELEN(music_coin);
            break;
        case musExplosion:
            music_data = music_explosion;
            music_length = TUNELEN(music_explosion);
            break;
        default:
            music_data = NULL;
            music_length = 0;
            break;
    }
    music_note = 0;
    music_tempo = 0;
    music_loop = loop;
    selecting_music = 0;
}

void playNote()
{    
    if(music_note < music_length)
    {
        //the tunes are in flash, PLATFORM_READ_WORD is the only safe way to read them
        Platform_PlayTone(PLATFORM_READ_WORD(&music_data[music_note]), 0);

        //Set the new delay to wait
        //the note length is in ms, the tempo counts frames. Written as 60/FPS it divided by
        //zero once the frame rate went above 60
        uint32_t frames = (uint32_t)PLATFORM_READ_WORD(&music_data[music_note + 1]) * FPS / 1000;
        music_tempo = (frames > 255) ? 255 : (uint8_t)frames;

        //Skip to the next note
        music_note += 2;
               
        if (music_note > music_length - 1)
        {
            if(music_loop)
            {
                music_note = 0;
            }
        }
    }
}


void musicTimer()
{
    //for nintendo systems as sega one checks it earlier
    if (selecting_music)
    {
        return;
    }

    //Play some music
    if (music_tempo == 0)
    {
        if(music_on)
        {
            playNote();
        }
    }
    //Else wait for the next note to play
    else 
    {
        music_tempo--;        
    }
}

void initMusic()
{
	music_note = 0;
	music_data = NULL;
	music_length = 0;
	music_tempo = 0;
	music_loop = 0;
	//set to 1 so nothing plays until a music was selected
	selecting_music = 1;
}

void deInitMusic(void)
{

}

void playGameExplodeSound(void)
{
    if (sound_on)
    {
        SelectMusic(musExplosion,0);
    }
}

void playGameCollectSound(void)
{
    if (sound_on)
    {
        SelectMusic(musCoin,0);
    }
}

void playGameMoveSound(void)
{
    if (sound_on)
    {
        Platform_PlayTone(800, (uint16_t)sfxSustain/10);
    }
}

void playLevelDoneSound(void)
{
    if (sound_on)
    {
        SelectMusic(musWinner,0);
    }
}

void playErrorSound(void)
{
    if (sound_on)
    {
        Platform_PlayTone(210, (uint16_t)sfxSustain);
    }
}

void playMenuSelectSound(void)
{
    if (sound_on)
    {
        Platform_PlayTone(1250, (uint16_t)sfxSustain);
    }
}

void playMenuBackSound(void)
{
    if (sound_on)
    {
        Platform_PlayTone(1000, (uint16_t)sfxSustain);
    }
}

void playMenuSound(void)
{
    if (sound_on)
    {
        Platform_PlayTone(900, (uint16_t)sfxSustain);
    }
}

void processSound(void)
{
    if (selecting_music)
    {
        return;
    }
    
    musicTimer();
}