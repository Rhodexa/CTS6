/*  
  CTS6 Magic (Chiptune Studio 6 Magic) is a polyphonic chiptune-oriented wavetable synthesizer, designed for ATMEGA328P.
  Copyright (C) 2021 Rhodune! Lab.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>

  /////////////////////////////////////////////////////////// Enough for license //

  |————— ABOUT & NEWS —————|
  CTS6 (MAGIC) will use Timer2 to produce wavetable synthesis on pins 3 and 11.
  Casually, Timer2 is used for Arduino's tone() function, which you won't be using anymore, 'cause you now got CTS MAGIC!

  · Many changes have been made since CTS5, the major upgrade is that we now use Timer2 only, for both PWM output generation and phase generation clock.
  · The second major upgrade is that CTS now can make Stereo! — Use setStereoLevels(voice, channel_l_level, channel_r_level). Channel L will be pin 11, and Channel R will be 3.
  · The third major upgrade is the existance an FM implementation. Channels 2 and 3 are acctually made up of two slots each.
      These voices are independently controllable, volume of modulator slot controls the amount of FM. 
      The modulator, however, can be redirected to the output acummulator, thus gaining a extra voice per channel.
      So one FM cahnnel, can act as two simple channels.
  · Phase accumulator clock now runs at half the original speed (~31KHz), that means less interruptions to your code.
  · Pre-made wavetables are now written in program memory, freeing RAM space.
  · Custom wavetables can be easily loaded from outside the library, via #define ADD_CUSTOM_WAVETABLES <your wavetable>
  · Stereo can be disabled if not needed. This should compile a faster code, and also let pin 11 (channel L) free.
  · FM can be disabled to get lighter code.
  
  Obvious note!: This is NOT the most efficient code, but compared with CTS3 which can only handle
  4 volume levels, mono, and only two square waves (with no PWM) before completely crashing... i think CTS6 is pretty good!

  · Made whit LOVE by Rhodune! ·
*/

#ifndef CTS6_H
#define CTS6_H

// Sorry if documentation here is a little weird.
// I don't really like comments, they make code messy. :)
namespace CTS {
#include "waves.h"

/*  Some masters' black magic for speed! */
#define FROM_PROGMEM(address_short) __LPM((uint16_t)(address_short))
#define LFSR_CLOCK() lfsr.tbit = 0; asm volatile(                  \
          "sbrc %A0, 1"                                   "\n\t"   \
          "eor  %1, %4"                                   "\n\t"   \
          "sbrc %A0, 3"                                   "\n\t"   \
          "eor  %1, %4"                                   "\n\t"   \     
          "lsr  %B0"                                      "\n\t"   \
          "ror  %A0"                                      "\n\t"   \
          "sbrc %1, 0"                                    "\n\t"   \
          "ori  %B0, 128"                                 "\n\t"   \
          : "=r" (lfsr.reg.u16), "=d" (lfsr.sbit)                  \
          : "0" (lfsr.reg.u16), "1" (lfsr.sbit), "d" ( 0x01 ))
          
#define ADD24(x, y) asm volatile(                                                                           \ 
          "add %0, %3"                                                                              "\n\t"  \
          "adc %1, %4"                                                                              "\n\t"  \
          "adc %2, %5"                                                                                      \
          : "=r" (x.u8[0]), "=r" (x.u8[1]), "=r" (x.u8[2]), "=r" (y.u8[0]), "=r" (y.u8[1]), "=r" (y.u8[2])  \
          : "0"  (x.u8[0]), "1"  (x.u8[1]), "2"  (x.u8[2]), "3"  (y.u8[0]), "4"  (y.u8[1]), "5"  (y.u8[2]))


/* /////////////////////////////////////// DANGER — WEIRD CODE AHEAD ////////////////////////////////////////////// */

union reg_32{uint32_t u32; uint16_t u16[2]; uint8_t u8[4];}; // A reg_32 can be treated as one uint32_t, two uint16_t or four uint8_t, thus
union reg_16{uint16_t u16; uint8_t u8[2];};                  // reducing the needs for bitwise shifts. The compiler probably does this by itself...
                                                             // but this way i'm more sure it's doing what we want it to do.

  void      begin();
  void      noteOn(byte, boolean);
  void      setEnvelope(byte, byte, byte, byte, byte);
  uint32_t  getFnumber(float);
  void      setFrequency(byte, float);
  float     getFrequency(float);
  void      updateEnvelope();
  void      setStereoLevels(byte, byte, byte);
  void      setWave(byte, byte);

  reg_16 l_channel_accumulator;
  reg_16 r_channel_accumulator;
  
  struct Operator{
    reg_32 tune;
    reg_32 phase;
    reg_16 dephase;
    const byte* wave;
    uint8_t out;
  };
  
  struct Envelope{
    uint8_t keyon;
    uint8_t attenuator_a_level;
    uint8_t attenuator_b_level;
    uint8_t starting_level;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    boolean one_shot = true;
    uint8_t quantize = 0xFF;
    uint8_t state;            
    uint8_t level;
    reg_16  attenuator_a_out;
    reg_16  attenuator_b_out;
  };
  
  struct LFSR{
    reg_16  reg;
    uint8_t sbit;
    uint8_t frq;
    uint8_t out;
  };
  
  Operator op[6];    // Even though called Operatos, the are actaully voice slots,.
  Envelope eg[16];   // Envelope generators - Yeah, i know, there's way more than we need. Sacrifices for speed, OK?
  LFSR lfsr;

  void begin(){
    #ifdef DISABLE_STEREO // /!\ Disabling stereo just disconnects pin 11, it won't mix L and R channels 
      TCCR2A = 0x21;      //  any panning into L channel will not sound at all.
      TCCR2B = 0x01;
      TIMSK2 = 0x01;
      pinMode(3, OUTPUT);
    #else
      TCCR2A = 0xA1;      // Connect pin 3 and 11 to TC2 / 31372KHz sample rate
      //TCCR2A = 0xA3;      // 62744KHz sample rate
      TCCR2B = 0x01;
      TIMSK2 = 0x01;
      pinMode(3, OUTPUT); 
      pinMode(11, OUTPUT);
    #endif
     lfsr.reg.u16 = 0xF034;
  }
  
  // Fnumber is the name OPL uses for the tunning registers. 
  // Same concept applies here, Fnumber is used for op[voice].tune.u32
  uint32_t getFnumber(float freq){
    return uint32_t(16777216.0f*freq/31372.0f);}
  
  // Set a channel's frequency, do the calculations automatically
  void setFrequency(byte voice, float freq){
    op[voice].tune.u32 = getFnumber(freq);}
    
  // Get the correspoding frequency of a note based on its index.
  // 0 = A4 or 440Hz — Float and Negative numbers are allowed.
  float getFrequency(float pitch){
    return 440*(pow(1.059463094359, pitch));}
  
  // Tell a voice where in flash is its wavetable.
  // An index is used to select a wave. 0 = Sine wave, 1 = Sawtooth, etc...
  // Premade waves are defined in waves.h
  void setWave(byte voice, uint8_t wave){
    op[voice].wave = wave_rom + wave*64;}

  // CTS implements ADSR type envelopes.
  // volume will increase at 'attack' speed, as soonas it reaches maximum level it will
  // star decrementing towards 'sustain' level at 'decay' speed. Sustain level will be hold until
  // the key is realesed, at which point, the envelope level will fall to 0 at a rate of 'release'
  // We also included an startig level. Insted of the ADSR reseting back to 0, it will reset to this said
  // starting level.
  void setEnvelope(byte envelope, byte starting_level, byte attack, byte decay, byte sustain, byte release){
    eg[envelope].starting_level  = starting_level;
    eg[envelope].attack          = attack;
    eg[envelope].decay           = decay;
    eg[envelope].sustain         = sustain;
    eg[envelope].release         = release;
    eg[envelope].one_shot        = 1;
  }
  
  // Envelopes double as LFOs, if 'one_shot' is set to 0 (false), 'level' will loop
  // between sustain and it's max value, at 'attack' and 'release' rates respectively,
  // after passing through the attack region.
  // 'quantize' reduces the LFO resolution. 8 steps can be selected. Each step divides the previous resolution in half.
  void setLFO(byte envelope, byte starting_level, byte attack, byte decay, byte sustain, byte release, uint8_t quantize){
    eg[envelope].starting_level  = starting_level;
    eg[envelope].attack          = attack;
    eg[envelope].decay           = decay;
    eg[envelope].sustain         = sustain;
    eg[envelope].release         = release;
    eg[envelope].quantize        = quantize;
    eg[envelope].one_shot        = 0;
  }
  
  // Set the maximum volume of some voice for left and right channels.
  void setStereoLevels(byte att, byte level_l, byte level_r){
    eg[att].attenuator_a_level = level_l;
    eg[att].attenuator_b_level = level_r;
  }

  // Start or Stop an envelope
  void keyOn(byte index, boolean keyon){
    if(keyon) eg[index].level = eg[index].starting_level;
    eg[index].state = keyon;
  }

  /* Here's where the magic happens — A timer interrupt, of course. */
  uint8_t phase_gen_timer;
  uint8_t envelope_timer;
  uint8_t lfsr_timer;
  uint8_t current_envelope = 0;
  ISR(TIMER2_OVF_vect){
    if(phase_gen_timer){
      phase_gen_timer = 0;

      /* PULSE */
      ADD24(op[0].phase, op[0].tune);
      ADD24(op[1].phase, op[1].tune);

      /* FM1 */
      ADD24(op[2].phase, op[2].tune);
      ADD24(op[3].phase, op[3].tune);
      op[3].dephase.u16 = (FROM_PROGMEM(op[2].wave + (op[2].phase.u8[2]&0x3F)))*eg[4].attenuator_a_out.u8[1];
      op[3].out = FROM_PROGMEM(op[3].wave + ((op[3].phase.u8[2] + op[3].dephase.u8[1])&0x3F));

      /* FM2 */
      ADD24(op[4].phase, op[4].tune);
      ADD24(op[5].phase, op[5].tune);
      op[5].dephase.u16 = (FROM_PROGMEM(op[4].wave + (op[4].phase.u8[2]&0x3F)))*eg[6].attenuator_a_out.u8[1];
      op[5].out = FROM_PROGMEM(op[5].wave + ((op[5].phase.u8[2] + op[5].dephase.u8[1])&0x3F));

      /* Noise — Fabulous Linear Feedback Shift Register */
      if(lfsr_timer+lfsr.frq > 0xFF){lfsr.out = lfsr.reg.u8[0]; lfsr_timer = 0;}
      lfsr_timer += lfsr.frq;
  
      l_channel_accumulator.u16 = (
        op[3].out * eg[5].attenuator_a_out.u8[1]+
        op[5].out * eg[7].attenuator_a_out.u8[1]+
        lfsr.out * eg[8].attenuator_a_out.u8[1]
      );
  
      r_channel_accumulator.u16 = (
        op[3].out * eg[5].attenuator_b_out.u8[1]+
        op[5].out * eg[7].attenuator_b_out.u8[1]+
        lfsr.out * eg[8].attenuator_b_out.u8[1]
      );

      OCR2A =
        l_channel_accumulator.u8[1]+
        ((op[0].phase.u8[2] < eg[0].attenuator_a_out.u8[1])) * eg[1].attenuator_a_out.u8[1]+
        ((op[1].phase.u8[2] < eg[2].attenuator_a_out.u8[1])) * eg[3].attenuator_a_out.u8[1];
        
      OCR2B = 
        r_channel_accumulator.u8[1]+
        ((op[0].phase.u8[2] < eg[0].attenuator_b_out.u8[1])) * eg[1].attenuator_b_out.u8[1]+
        ((op[1].phase.u8[2] < eg[2].attenuator_b_out.u8[1])) * eg[3].attenuator_b_out.u8[1];
  
      /* ///////////// Envelope/LFO Generation ///////////// */
      if(envelope_timer == 29){
        envelope_timer = 0;
        switch(eg[current_envelope].state){
          case 0:
            eg[current_envelope].state  = (eg[current_envelope].level + eg[current_envelope].attack > 255);
            eg[current_envelope].level +=  eg[current_envelope].attack;
            break;
         case 1:            
            eg[current_envelope].state  = (eg[current_envelope].level - eg[current_envelope].decay < eg[current_envelope].sustain)+one_shot;
            eg[current_envelope].level -=  eg[current_envelope].decay;
            break;
         case 3:
            eg[current_envelope].state  = (eg[current_envelope].level - eg[current_envelope].release < 1)+3;
            eg[current_envelope].level -=  eg[current_envelope].release;
            break;
         case 4:
            eg[current_envelope].level = 0;
            break;
        }
        eg[current_envelope].attenuator_a_out.u16 = (eg[current_envelope].level & eg[current_envelope].quatize) * eg[current_envelope].attenuator_a_level;
        eg[current_envelope].attenuator_b_out.u16 = (eg[current_envelope].level & eg[current_envelope].quatize) * eg[current_envelope].attenuator_b_level;
        current_envelope=(current_envelope+1)&0x0F; // 4-bit counter. 0···15
      }
      envelope_timer++;
    }
    phase_gen_timer++;
    
    // let's always clock the LFSR to get an even more random output. By the time we read the LFSR register, it would have advanced more than one bit.
     // This will cause the output to look closer to white noise rather than random ramps.
    LFSR_CLOCK(); 
  }
}
#endif
