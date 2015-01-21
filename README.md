# PixelShirtV2
Arduino based collection of video games for a shirt

Remember to tweak the lib to use a 128 byte buffer and high speed for GPU receive (Wire, TWI)

in Wire.h:

\#define BUFFER_LENGTH 128

in twi.h:

\#ifndef TWI_FREQ

\#define TWI_FREQ 400000L

\#endif

\#ifndef TWI\_BUFFER_LENGTH

\#define TWI\_BUFFER_LENGTH 128

\#endif
