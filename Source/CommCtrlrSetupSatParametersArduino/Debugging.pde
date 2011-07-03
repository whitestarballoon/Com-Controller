// Thist prints strings out the Ground Support Board by using i2c! Yay
int lprintf(char *str, ...)
{
  unsigned long time;
  unsigned long timestart;
  char lstr[30];
  uint8_t i2cSend[31];
  int chars;
  va_list args;

  va_start(args, str);

  chars = vsnprintf(lstr, 30, str, args);

  if(chars > 30)
  {
    va_end(args);
    return 1;
  } 
  else {
    I2CXmit(i2cGroundSupport, 0x05, (byte*)lstr, chars);
    va_end(args);
    return 0;
  }
  //Delay a bit for the GSP to catch up
  time = millis();
  timestart = time;
  while (timestart+100>time){
    time = millis();
  }
}



int lprintf_P(const char *str, ...)
{
  unsigned long time;
  unsigned long timestart;
	char lstr[30];
	uint8_t i2cSend[31];
	int chars;
	va_list args;

	va_start(args, str);

	chars = vsnprintf_P(lstr, 30, str, args);

	if(chars > 30)
	{
		va_end(args);
		return 1;
	} else {
		I2CXmit(i2cGroundSupport, 0x05, (byte*)lstr, chars);
		va_end(args);
		return 0;
	}
  //Delay a bit for the GSP to catch up
  time = millis();
  timestart = time;
  while (timestart+100>time){
	time = millis();
  }
}
