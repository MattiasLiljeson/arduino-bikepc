#include <U8glib.h>
//**************************************************
// Change this constructor to match your display!!!
U8GLIB_SSD1306_128X64 u8g(12, 11, 10, 9, 8);  // D0=13, D1=11, CS=10, DC=9, Reset=8
//**************************************************


#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

static NMEAGPS  gps;
static gps_fix  fix;

//----------------------------------------------------------------
//  This function gets called about once per second, during the GPS
//  quiet time.  It's the best place to do anything that might take
//  a while: print a bunch of things, write to SD, send an SMS, etc.
//
//  By doing the "hard" work during the quiet time, the CPU can get back to
//  reading the GPS chars as they come in, so that no chars are lost.

char floatBuf[8];
char buf[16];
float dist_in_km;
uint32_t ms;

void calc(){
  if (ms != 0){
    double delta = millis() - ms;
    double factor = delta/3600000;
    dist_in_km += factor*fix.speed_kph();
    //dist_in_km += 1;

    Serial.print("millis: ");
    Serial.print(millis());
    Serial.print("\n");

    Serial.print("ms: ");
    Serial.print(ms);
    Serial.print("\n");
    
    Serial.print("delta: ");
    Serial.print(delta);
    Serial.print("\n");

    Serial.print("factor: ");
    Serial.print(factor);
    Serial.print("\n");
  
    Serial.print(dist_in_km);
    Serial.print("\n");
    
    Serial.print(delta);
    Serial.print("\n");
  }
  ms = millis();
}

void draw() {
  u8g.firstPage();  
  do {
    // graphic commands to redraw the complete screen should be placed here  
    u8g.setFont(u8g_font_fur17r);
    {
      dtostrf(fix.speed_kph(), 6, 0, floatBuf);
      sprintf(buf, "Spd:%s", floatBuf);
      u8g.drawStr( 0, 32, buf);
    }
    {
      dtostrf(dist_in_km, 6, 3, floatBuf);
      sprintf(buf, "Dst:%s", floatBuf);
      u8g.drawStr( 0, 64, buf);
    }
  } while( u8g.nextPage() );
}

void doSomeWork()
{
  trace_all( DEBUG_PORT, gps, fix );
  calc();
  draw();
}

void setupSw(){
  dist_in_km = 0;
  ms = 0;
}

void setupDisplay() {  
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 
}

void setupGps(){
  DEBUG_PORT.begin(9600);
  while (!DEBUG_PORT)
    ;

  DEBUG_PORT.print( F("NMEA.INO: started\n") );
  DEBUG_PORT.print( F("  fix object size = ") );
  DEBUG_PORT.println( sizeof(gps.fix()) );
  DEBUG_PORT.print( F("  gps object size = ") );
  DEBUG_PORT.println( sizeof(gps) );
  DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );

  #ifndef NMEAGPS_RECOGNIZE_ALL
    #error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
  #endif

  #ifdef NMEAGPS_INTERRUPT_PROCESSING
    #error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
  #endif

  #if !defined( NMEAGPS_PARSE_GGA ) & !defined( NMEAGPS_PARSE_GLL ) & \
      !defined( NMEAGPS_PARSE_GSA ) & !defined( NMEAGPS_PARSE_GSV ) & \
      !defined( NMEAGPS_PARSE_RMC ) & !defined( NMEAGPS_PARSE_VTG ) & \
      !defined( NMEAGPS_PARSE_ZDA ) & !defined( NMEAGPS_PARSE_GST )

    DEBUG_PORT.println( F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed.") );

  #else
    if (gps.merging == NMEAGPS::NO_MERGING) {
      DEBUG_PORT.print  ( F("\nWARNING: displaying data from ") );
      DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      DEBUG_PORT.print  ( F(" sentences ONLY, and only if ") );
      DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      DEBUG_PORT.println( F(" is enabled.\n"
                            "  Other sentences may be parsed, but their data will not be displayed.") );
    }
  #endif

  DEBUG_PORT.print  ( F("\nGPS quiet time is assumed to begin after a ") );
  DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
  DEBUG_PORT.println( F(" sentence is received.\n"
                        "  You should confirm this with NMEAorder.ino\n") );

  trace_header( DEBUG_PORT );
  DEBUG_PORT.flush();

  gpsPort.begin( 9600 );
}

void setup()
{
  setupSw();
  setupDisplay();
  setupGps();
}

void loop()
{
  while (gps.available( gpsPort )) {
    fix = gps.read();
    doSomeWork();
  }
}
