CC = gcc                                                                        
CFLAGS = -O2 -Wall                                                              
OBJS =  nbus.o led_spi.o                                                        
LIBS = -lm -lasound #-mcpu=arm9                                                 
                                                                                
all: led_spectrum                                                               
                                                                                
led_spectrum: $(OBJS)                                                           
	$(CC) $(LFLAGS) led_spectrum.c $(OBJS) $(LIBS) -o $@                    
                                                                                
nbus.o: nbus.c                                                                  
	$(CC) $(CFLAGS) -c $^                                                   
                                                                                
led_spi.o: led_spi.c                                                            
	$(CC) $(CFLAGS) -c $^                                                   
        
run:
	su -c "./led_spectrum"

clean:
	rm -f *.o led_spectrum                                                  
                                                                                
restore:
	su -c "cp -a /root/. /home/ece/ && chmod 666 *"                         

root:
	su

ece:
	su ece

cron: 
	(crontab -l ; echo "@reboot cd /home/ece && ./led_spectrum")| crontab - 
