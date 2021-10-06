#include <stdio.h>                                                              
#include <stdint.h>                                                             
#include <unistd.h>                                                             
#include "nbus.h"                                                               
                                                                                
void led_display(int led_cnt, unsigned char *led_array)                         
{                                                                               
        uint16_t val;                                                           
        uint8_t toggle;                                                         
        int i, j;                                                               
        static unsigned char data;                                              
        nbuslock();                                                             
                                                                                
        // set the ddr to output for pins 13, 14                                
        val = nbus_peek16(0xc);                                                 
        nbus_poke16(0xc, val | (1 << 14));                                      
        val = nbus_peek16(0xc);                                                 
        nbus_poke16(0xc, val | (1 << 13));                                      
                                                                                
        val = nbus_peek16(0xa);                                                 
        nbus_poke16(0xa, val | (1 << 14));                                      
        val = nbus_peek16(0xa);

	/* Give up and then acquire the lock in case
	 * any other peripherals need it, before the 
	 * sequence is written out. 	
	 */
        nbusunlock();                                                           
                                                                                
        nbuslock();                                                             
                                                                                
        for(i = 0; i < ((led_cnt+2)*4); i++) {                                  
            data = led_array[i];                                                
            for(j = 0; j < (sizeof(data)*8); j++){                              
                // get the bit value to set                                     
                toggle = (data & (1<<j)) ? 1 : 0;                               
                // change data                                                  
                nbus_poke16(0xa, val | (toggle << 13));                         
                // raise clk                                                    
                nbus_poke16(0xa, val | (1 << 14));                              
                // lower clk                                                    
                nbus_poke16(0xa, val & ~(1 << 14));                             
                //nbuspreempt();                                                
            }                                                                   
        }  
	// After the sequence is written out release the lock	
        nbusunlock();                                                           
}               
