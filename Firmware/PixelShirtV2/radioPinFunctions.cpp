/*
* ----------------------------------------------------------------------------
* THE COFFEEWARE LICENSE (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* Please define your platform spesific functions in this file ...
* -----------------------------------------------------------------------------
*/

#ifdef ARDUINO

#include <avr/io.h>

#define set_bit(reg,bit) reg |= (1<<bit)
#define clr_bit(reg,bit) reg &= ~(1<<bit)
#define check_bit(reg,bit) (reg&(1<<bit))

/* ------------------------------------------------------------------------- */
void nrf24_setupPins(void)
{
  set_bit(DDRC,6); /* CE output, digital pin 5 */
  set_bit(DDRD,7); /* CSN output, digital pin 6 */
  set_bit(DDRB,1); /* SCK output */
  set_bit(DDRB,2); /* MOSI output */
  clr_bit(DDRB,3); /* MISO input */
}
/* ------------------------------------------------------------------------- */
void nrf24_ce_digitalWrite(uint8_t state)
{
  if(state) {
    set_bit(PORTC,PORTC6);
  }
  else {
    clr_bit(PORTC,PORTC6);
  }
}
/* ------------------------------------------------------------------------- */
void nrf24_csn_digitalWrite(uint8_t state)
{
  if(state) {
    set_bit(PORTD,PORTD7);
  }
  else {
    clr_bit(PORTD,PORTD7);
  }
}
/* ------------------------------------------------------------------------- */
void nrf24_sck_digitalWrite(uint8_t state)
{
  if(state) {
    set_bit(PORTB,PORTB1);
  }
  else {
    clr_bit(PORTB,PORTB1);
  }
}
/* ------------------------------------------------------------------------- */
void nrf24_mosi_digitalWrite(uint8_t state)
{
  if(state) {
    set_bit(PORTB,PORTB2);
  }
  else {
    clr_bit(PORTB,PORTB2);
  }
}
/* ------------------------------------------------------------------------- */
uint8_t nrf24_miso_digitalRead(void)
{
  return check_bit(PINB,PINB3);
}
/* ------------------------------------------------------------------------- */
#endif
