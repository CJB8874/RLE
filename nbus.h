#define MW_ADR    0x70
#define MW_CONF   0x72
#define MW_DAT1   0x74
#define MW_DAT2   0x76
#define _GNU_SOURCE

void nbuslock(void);
void nbusunlock(void);
void nbuspreempt(void); 

void nbus_poke16(unsigned char, unsigned short);
unsigned short nbus_peek16(unsigned char);

void winpoke16(unsigned int, unsigned short);
unsigned short winpeek16(unsigned int);
void winpoke32(unsigned int, unsigned int);
unsigned int winpeek32(unsigned int);
void winpoke8(unsigned int, unsigned char);
unsigned char winpeek8(unsigned int);


