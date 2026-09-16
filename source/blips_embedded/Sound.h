#ifndef SOUND_H
#define SOUND_H

#define musWinner 1
#define musClear 2
#define musCoin 3
#define musExplosion 4

void initSound(void);
void SelectMusic(uint8_t musicFile, uint8_t loop);
void initMusic(void);
void playMenuSound(void);
void playMenuSelectSound(void);
void playErrorSound(void);
void playLevelDoneSound(void);
void playMenuBackSound(void);
void playGameMoveSound(void);
void playGameCollectSound(void);
void playGameExplodeSound(void);
void setMusicOn(uint8_t value);
void setSoundOn(uint8_t value);
uint8_t isMusicOn(void);
uint8_t isSoundOn(void);
uint8_t isMusicPlaying();
void stopMusic(void);
void deInitMusic(void);
void deInitSound(void);
void processSound(void);


#endif