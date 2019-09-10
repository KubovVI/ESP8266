// 4LF, 128HF Filtering 
void getPPG(int16_t *Red, int16_t *IR); // Filtered data
void getPPGraw(int32_t *Red, int32_t *IR); //Raw data
//time interval >(16+2)ms
char setupPPG(void); //1000sps, 16 samples average, Red+IR 
void powerPPG(char power); //Sensor LEDs On/Off
void setLeds(uint8_t Red, uint8_t IR); 

