/*
 * buzzer.h
 *
 *  Created on: 14.01.2016
 *      Author: tom
 */

#ifndef LOCAL_INC_BUZZER_H_
#define LOCAL_INC_BUZZER_H_

void initBuzzer();
void startSoundTimer();
void startTimer(void);
void stopTimer(void);
void pwmEmulate(uint32_t microsec_period, uint16_t length);
void makeSound();
void makeSoundTask();

extern volatile Mailbox_Handle accelerationDataMailbox;

#endif /* LOCAL_INC_BUZZER_H_ */

