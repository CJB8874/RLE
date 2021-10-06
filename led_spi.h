#ifndef LED_SPI_H                                                               
#define LED_SPI_H                                                               

/*This function bit-bangs out the LED frame array on the
 * SPI pins of the TS-7600 board using the nbus library
 * develop by Technologic.
 */
void led_display(int led_cnt, unsigned char *led_array);                        
                                                                                
#endif    
