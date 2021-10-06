//==================================================================
// Name:				Date:		GEE105
/*==================================================================
 *	EDIT THE FOLLOWING DEFINES
 *==================================================================
 * The brightness can be set to anywhere between 0-31, with 0 being
 * the dimmest and 31 being the brightest.
 */
#define BRIGHTNESS (8)
/* This determines how sensitive the display is the noise. The
 * lower the number the more sensitive, and the higher the more
 * sensitive. This value can take on a range form approximately
 * 30,000 - 0
 */
#define SENSITIVITY 5000 
/* Color Setting ==================================================
 * The LEDs can be set using RGB colors. The colors that they are
 * set to are listed below. Note that colors do not combine
 * in the way you might expect. Red and green for example make 
 * orange. This is becaus RGB is a different way of describing
 * color, RGB being additive color mixing where the standard kind
 * from an art class is subtractive color mixing.
 *
 * To use an RGB color wheel to pick out your colors
 * go to the following website:
 * https://www.w3schools.com/colors/colors_rgb.asp
 */
// ----------------------------------------------------------------
// First Bar: Green
#define b1_red    0
#define b1_green  255
#define b1_blue   0
// ----------------------------------------------------------------
// Second Bar: Red
#define b2_red   255
#define b2_green 0
#define b2_blue  0
// ----------------------------------------------------------------
// Third Bar: Purple
#define b3_red   150
#define b3_green 0
#define b3_blue  255
// ----------------------------------------------------------------
// Fourth Bar: Yellow
#define b4_red   255
#define b4_green 255
#define b4_blue  0
// ----------------------------------------------------------------
// Fifth Bar: Blue
#define b5_red   0
#define b5_green 0
#define b5_blue  255
// ----------------------------------------------------------------
// Sixth Bar: Orange
#define b6_red   255
#define b6_green 125
#define b6_blue  0
// ----------------------------------------------------------------
// Seventh Bar: Cyan
#define b7_red   0
#define b7_green 255
#define b7_blue  150
// ----------------------------------------------------------------
// Eighth Bar: 
#define b8_red   255
#define b8_green 0 
#define b8_blue  255
/*=================================================================
 *	DO NOT EDIT ANYTHIN BELOW THIS POINT UNLESS TOLD TO
 *=================================================================
 */

/* This parts of this code used to interact with the alsa is modified from      
 * Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html.           
 * To compile:                                                                  
 * gcc -o led_spectrum led_spectrum.c nbus.c led_spi.c -lm -lasound -mcpu=arm9  
 * Or use the Makefile in the directory.                                                                             
 * This program takes the FFT of audio input, from a USB microphone, and outputs
 * the FFT to an LED strip. Each frequency bin has a different color, which can 
 * be changed by alter the RGB values in the on_col_val_strp array.             
 */                                                                             
                                                                                
#include <stdio.h>                                                              
#include <stdlib.h>                                                             
#include <stdint.h>                                                             
#include <unistd.h>                                                             
#include <alsa/asoundlib.h>                                                     
#include <math.h>                                                               
#include "led_spi.h"                                                            

// Microphone will be card 1 if nothing else is hooked up                       
#define CARD_NUM "hw:1"                                                         
// Microphone only has a single channel                                         
#define CHAN_NUM 1                                                              
                                                                                
#define LED_ROWS 7                                                              
#define LED_COLS 8                                                              
#define LED_CNT LED_ROWS*LED_COLS                                               
                                                                                
#define BUF_SIZE 16                                                             
// N is assumed to be greater than 4 and a power of 2                           
#define PI2 6.28318530718                                                       
                                                                                
/* This sets the brightness of the LEDs, the first 3                            
 * bits need to 1s, hence the 0xE0. So the value can be                         
 * set between 0-31.                                                            
 * NOTE: Please do not set it to above 10 in the                                
 * lab, it is too bright.                                                       
 */                                                                             
#define BRIGHTNESS_SET (0xE0 | BRIGHTNESS)                                                   
                                                                                
/* This number was set for visually seeing the spectrum                         
 * not to officially set the magnitude between 0-1. The maximum                 
 * value is well above 40000, 15000 was chosen so that different                
 * frequencies besides DC could be seen, given how few frequency                
 * bins there are at the low buffer size.                                       
 */                                                                             
#define NORM_NUM (float) SENSITIVITY                                                         
                                                                                
// Twiddle factors (16th roots of unity)                                        
const float twid[] = {                                                          
 1.00000, 0.92388, 0.70711, 0.38268,-0.00000,-0.38269,-0.70711,-0.92388,        
-1.00000,-0.92388,-0.70710,-0.38267, 0.00001, 0.38269, 0.70712, 0.92388         
};                                                                              
                                                                                
//Follows the following format:                                                 
// brightness, blue, green, red                                                 
uint8_t on_col_val_strp[8][4] = {                                               
  {BRIGHTNESS_SET, b1_blue, b1_green, b1_red},                                           
  {BRIGHTNESS_SET, b2_blue, b2_green, b2_red},                                             
  {BRIGHTNESS_SET, b3_blue, b3_green, b3_red},                                        
  {BRIGHTNESS_SET, b4_blue, b4_green, b4_red},                                          
  {BRIGHTNESS_SET, b5_blue, b5_green, b5_red},                                            
  {BRIGHTNESS_SET, b6_blue, b6_green, b6_red},                                          
  {BRIGHTNESS_SET, b7_blue, b7_green, b7_red},                                           
  {BRIGHTNESS_SET, b8_blue, b8_green, b8_red}                                     
};                                                                              

// There are used if you change it to do a matrix
uint8_t on_col_val_mtrx[4] = {BRIGHTNESS, 0, 0, 255};                           
// This displays zero color if the light is off                                                                                
uint8_t off_col_val[4] = {BRIGHTNESS, 0, 0, 0};                                 

// These are function initializations 
void fftReal(int16_t in[BUF_SIZE], int32_t intmdi[BUF_SIZE/2+1], int32_t intmdr[BUF_SIZE/2+1], int32_t out[BUF_SIZE/2+1]);                                      
void fftToLED_matrix(float *val, uint8_t* led_arr);                             
void fftToLED_strip(float *val, uint8_t* led_arr); 
int  rand_range(int low, int up);                             

int main (){                                                  
  int i, j;                                                                     
  int err;                                                                      
  int16_t *buffer;
  // Buffers to hold the real and imaginary components of fft  
  int32_t intmdr[BUF_SIZE/2 + 1];                                               
  int32_t intmdi[BUF_SIZE/2 + 1];                                               
  uint32_t out[BUF_SIZE/2 + 1];                                                 
  float norm_out[BUF_SIZE/2 + 1];                                               
  uint8_t led_arr[(LED_CNT + 2)*4];                                             
                                                                                
  int buffer_size = BUF_SIZE;                                                   
  // This is the maximum sampling frequency for usb mic                         
  unsigned int rate = 48000;                                                    
  snd_pcm_t *capture_handle;                                                    
  snd_pcm_hw_params_t *hw_params;                                               
  // The microphone outsputs signed 16 bit values                               
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;                              
  
  // The following code setups up the connection to the USB microphone and checks for all errors  
  printf("Starting program\nPress Ctrl-C to quit\n");                           
  while(1){  
  //=====================================================================================================
  // WRITE YOUR CODE HERE FOR RANDOM LED COLORS:
  //=====================================================================================================
  // * You can use the rand range function to generate a random number within a given range (0-255) etc
  // * Use a for loop to set the values in on_col_val_strp array
  
  //=====================================================================================================
  // DO NOT CHANGE ANYTHING BELOW THIS POINT:
  //=====================================================================================================
  if ((err = snd_pcm_open (&capture_handle, CARD_NUM, SND_PCM_STREAM_CAPTURE, 0)) < 0) {                                                                        
    fprintf (stderr, "error opening audio device %s (%s)\n", CARD_NUM, snd_strerror (err));                                                                     
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {                      
    fprintf (stderr, "error allocating hardware params (%s)\n", snd_strerror (err));                                                                            
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {          
    fprintf (stderr, "error initializing hardware params (%s)\n", snd_strerror (err));                                                                          
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {                                                    
    fprintf (stderr, "error setting access type (%s)\n", snd_strerror (err));   
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {                                                                           
    fprintf (stderr, "error setting sample format (%s)\n", snd_strerror (err)); 
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {                                                                      
    fprintf (stderr, "error setting sample rate (%s)\n", snd_strerror (err));   
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, CHAN_NUM)) < 0) {                                                                       
    fprintf (stderr, "error setting channel count (%s)\n", snd_strerror (err)); 
    exit (1);                                                                   
  }                                                                             
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {              
    fprintf (stderr, "error setting parameters (%s)\n", snd_strerror (err));    
    exit (1);                                                                   
  }                                                                             
  snd_pcm_hw_params_free (hw_params);                                           
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {                           
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));                                                                      
    exit (1);                                                                   
  }                                                                             
                                                                                
  buffer = malloc(buffer_size * snd_pcm_format_width(format) / 8);              
    // Capture the audio camples and store them in the buffer                                                                            
    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_size)) != buffer_size) {                                                                                 
      fprintf (stderr, "error reading from device (%s)\n", snd_strerror (err));                                                                            
      exit (1);                                                                 
    }                                                                           

    // Returns the power of the fft    
    fftReal(buffer, intmdi, intmdr, out);

    // Normalize the output                                                                             
    for(j = 0; j < BUF_SIZE/2; j++){                                            
        norm_out[j] = (out[j]/NORM_NUM) * LED_ROWS;                             
    }
    // Convert the buffer into led_frames/structure    
    fftToLED_strip(norm_out, led_arr);
    // Send the led_frame out to the LED strip through SPI    
    led_display(LED_CNT, led_arr);                                              
  // You need to free dyamically allocated memory                                                                              
  free(buffer);
  // Close the audio connection - this also frees allocated memory  
  snd_pcm_close (capture_handle);                                               
}                                                                               
  fprintf(stdout, "audio interface closed\n");                                  
                                                                                
  return 0;                                                                     
}                                                                               


// Computes the real FFT of the input, and takes the power spectrum of the output
void fftReal(int16_t in[BUF_SIZE], int32_t intmdi[BUF_SIZE/2+1], int32_t intmdr[
BUF_SIZE/2+1], int32_t out[BUF_SIZE/2+1]){                                      
    int n, k;                                                                   
                                                                                
    // Calculate DFT and power spectrum up to Nyquist frequency                 
    int to_sin = 3*BUF_SIZE/4; // index offset for sin                          
    int a, b;                                                                   
    for (k=0 ; k<=BUF_SIZE/2 ; ++k){                                            
        intmdr[k] = 0; intmdi[k] = 0;                                           
        a = 0;                                                                  
        b = to_sin;                                                             
        for (n=0 ; n < BUF_SIZE; ++n){                                          
            intmdr[k] += in[n] * twid[a%BUF_SIZE];                              
            intmdi[k] -= in[n] * twid[b%BUF_SIZE];                              
            a += k;                                                             
            b += k;                                                             
        }                                                                       
        out[k] = sqrt(intmdr[k]*intmdr[k] + intmdi[k]*intmdi[k]);               
    }                                                                           
}                                                                               

/* This converts of normalized values into a series of LED
 * frames set based on the row and cols set in the defines.
 * For example [3, 2, 7, ...] would set 3 LEDs on in the first
 * column, 2 LEDs on in the second column, 7 in the third etc.
 */
void fftToLED_matrix(float *val, uint8_t* led_arr){                             
  int i, j, k;                                                            
  uint16_t data;                                                          
  for(i = 0; i < LED_COLS; i++){                                          
    data = val[i];                                                  
    if(i%2 == 0){                                                   
      for(j = 0; j < LED_ROWS; j++){                          
        if(data-- > 0){                                 
          for(k = 0; k < 4; k++)                  
            led_arr[(4*i*LED_ROWS) + (j*4) + k + 4] = on_col_val_mtrx[k];                                                   
        }                                               
        else{                                           
          for(k = 0; k < 4; k++)                  
            led_arr[(4*i*LED_ROWS) + (j*4) + k + 4] = off_col_val[k];                                                       
        }                                               
      }                                                       
    }                                                               
    else{                                                           
      for(j = 6; j<= 1; j-- ){                                
        if(data >= j){                                  
          for(k = 0; k < 4; k++)                  
            led_arr[(4*i*LED_ROWS) + (j*4) + k + 4] = on_col_val_mtrx[k];                                                   
        }                                               
        else{                                           
          for(k = 0; k < 4; k++)                  
            led_arr[(4*i*LED_ROWS) + (j*4) + k + 4] = off_col_val[k];                                                       
        }                                               
      }                                                        
    }                                                               
  }                                                                       
}                                                                               

/* This converts of normalized values into a series of LED
 * frames set based on the row and cols set in the defines.
 * For example [3, 2, 7, ...] would set 3 LEDs on in the first
 * column, 2 LEDs on in the second column, 7 in the third etc.
 * This spits the value out in the order they appear to be displayed
 * on a strip - not on an LED matrix. The colors of each column
 * are set in the defines as well.
 */
void fftToLED_strip(float *val, uint8_t* led_arr){                              
        int i, j, k;                                                            
        float data;                                                             
                                                                                
  for (i = 0; i < 4; i ++) led_arr[i] = 0;                                      
                                                                                
        for(i = 0; i < LED_COLS; i++){                                          
                data = round(val[i]);                                           
    for(j = 0; j < LED_ROWS; j++){                                              
      if(data-- > 0){                                                           
        for(k = 0; k < 4; k++){                                                 
          led_arr[(4*i*LED_ROWS) + (j*4) + k + 4] = on_col_val_strp[i][k];      
        }                                                                       
      }                                                                         
      else{                                                                     
        for(k = 0; k < 4; k++){                                                 
          led_arr[(4*i*LED_ROWS) + (j*4) + k + 4] = off_col_val[k];             
        }                                                                       
      }                                                                         
    }                                                                           
        }                                                                       
  for (i = (LED_CNT + 2)*4 - 4; i < (LED_CNT + 2)*4; i ++) led_arr[i] = 0;      
}   

int rand_range(int low, int up){
  return (rand() % (up - low + 1)) + low;
}