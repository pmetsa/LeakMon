/* (c) 2022 Pekko Metsä

   Licence: GPLv3 or any later.

   Vuotovahti is written for Teensy 4.1, with several Rain Sensors
   FL-37 (or other versions like YL-83) attachet in ADC pins.
   Software uses DHCP to get an IP, and syncs RTC to NTP every few
   hours.  If RTC battery is attached, the initial time is read from
   RTC as Teensy boots.

   Whenever UDP port 8888 gets a connection, a byte string containing
   - 8bit int: data format version (0)
   - 16bit int: id of the installation
   - 32bit int: running sequence number since last boot 
   - 32bit int: Unix timestamp of the data 
   - N x 16bit int: the values of N sensors
   is sent as a reply.  All values are unsigned little-endians.  This
   is the data format version 0.

   In some future version the software may use e.g. SD card for
   logging the values, and more expressive query language for reading
   it.  But not yet.  Also a checksum for checking the data validity
   might be nice.
 */

#include <NativeEthernet.h>
#include <NTPClient.h>
#include <DS1307RTC.h>
#include <Chrono.h>

#define DEBUG 1
#define EQUIP_ID 1 // Zero means 'unidentified'.  Any nonzero should
                   // be unique inside the organization.
#define NTP_Interval 6*3600000 // Interval (in ms) for checking NTP
#define DATA_FORMAT 0

byte mac[] = {
	      // Enter the MAC address of the controller below
	      0x04, 0xE9, 0xE5, 0x0C, 0x70, 0xA4
};
unsigned int localPort = 8888; // local port to listen on
// The ADC pins to use
int analogPin1 = A16;
int analogPin2 = A17;
// EthernetUDP instance for NTP connection
EthernetUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, NTP_Interval);
Chrono NTPChrono;
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet
// The EthernetUDP instance for data communication
EthernetUDP Udp;
uint32_t count = 0; // Runninq seq. number for queries.

void setup() {
  time_t NTPnow; // for querying the time from NTP servers
  analogReadResolution(12); // Teensy 4.1 allows 12 bits, although
			    // only 10 bits is usually usable due to
			    // noise.

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // The function to get the time from the RTC
  setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");
  // start the Ethernet connection
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Setting the time..");
  timeClient.begin();
  timeClient.update();
  NTPnow=timeClient.getEpochTime();
  RTC.set(NTPnow);
  setTime(NTPnow);
  Serial.print("NTP updated. Time: ");
  Serial.println(NTPnow, DEC);
  if (DEBUG) {
    time_t RTCnow = now();
    Serial.print("RTC: ");
    Serial.println(RTCnow, DEC);
  }
  digitalClockDisplay();
  
  // start UDP
  Udp.begin(localPort);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  // The unions for sending integers as byte arrays
  union {
    uint32_t integer;
    byte bytes[4];
  } reply32;
  union {
    uint16_t integer;
    byte bytes[2];
  } reply16;
  uint16_t val1, val2; // For storing the Rain Sensor values
  time_t RTCnow;
  
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // Read the data and record the time stamp
    val1 = analogRead(analogPin1);
    val2 = analogRead(analogPin2);
    RTCnow = now();

    // Build and send th UDP datagram
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write((uint8_t) DATA_FORMAT);
    reply16.integer = (uint16_t) EQUIP_ID;
    Udp.write(reply16.bytes, sizeof reply16.bytes);
    reply32.integer = count++;
    Udp.write(reply32.bytes, sizeof reply32.bytes);
    reply32.integer = (uint32_t) RTCnow;
    Udp.write(reply32.bytes, sizeof reply32.bytes);
    reply16.integer = val1;
    Udp.write(reply16.bytes, sizeof reply16.bytes);
    reply16.integer = val2;
    Udp.write(reply16.bytes, sizeof reply16.bytes);
    Udp.endPacket();
    
    digitalClockDisplay();
    Serial.print("Pin1: ");
    Serial.println(val1);
    Serial.print("Pin2: ");
    Serial.println(val2);
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(year());
  Serial.print("-");
  printDigits(month());
  Serial.print("-");
  printDigits(day());
  Serial.print(" UTC ");
  printDigits(hour());
  Serial.print(":");
  printDigits(minute());
  Serial.print(":");
  printDigits(second());
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
