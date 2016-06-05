/* ===============================================================
   This is a fork of Rays OpenSprinkler code thats amended to use alternative hardware:

   EtherCardW5100.h and EtherCardW5100.cpp implements a minimal set of functions
   as a wrapper to replace the ENC28J60 EtherCard class libraries with the standard
   Arduino Wiznet5100 Ethernet library.

   Version:     Opensprinkler 2.1.6

   Date:        January 2016

   Repository:  https://github.com/Dave1001/OpenSprinkler-Arduino

   License:     Creative Commons Attribution-ShareAlike 3.0 license

   Refer to the README file for more information

   =============================================================== */
#include "Defines.h"
#undef DB_MASK
#define DB_MASK 4
#include <stdarg.h>
#ifndef ESP8266
#include <avr/eeprom.h>
#else
#include <FS.h>
#include <ESP8266mDNS.h>
#ifdef EEPROM_ESP
 #include "Eeprom_ESP.h"
#else
 #include "eeprom_mio.h"
#endif  //EEPROM_ESP
char* SSID = "Vodafone-25873015";
char* PASSWORD = "5ph87cmmjmm8cs9";
#endif

#include "EtherCardW5100.h"

//================================================================
// Utility functions
//================================================================

/// <summary>
/// Copy four bytes to an IPAddress object
/// </summary>
/// <param name="src">Pointer to the 4 byte source</param>
/// <returns></returns>
IPAddress Byte2IP ( const byte *src )
{
    return IPAddress ( src[0], src[1], src[2], src[3] );
}

/// <summary>
/// Copy an IPAddress object to four bytes
/// </summary>
/// <param name="src">Pointer to the 4 byte source</param>
/// <returns></returns>
void IP2Byte ( IPAddress ip, byte *dest )
{
    dest[0] = ip[0];
    dest[1] = ip[1];
    dest[2] = ip[2];
    dest[3] = ip[3];
}

/// <summary>
/// ethercard.cpp - convert a two byte word to ascii
/// </summary>
/// <param name="value"></param>
/// <param name="ptr"></param>
/// <returns></returns>
char* BufferFiller::wtoa ( uint16_t value, char* ptr )
{
    if ( value > 9 )
        ptr = wtoa ( value / 10, ptr );
    *ptr = '0' + value % 10;
    *++ptr = 0;
    return ptr;
}

/// <summary>
/// webutil.cpp - convert a single hex digit character to its integer value
/// </summary>
/// <param name="c">single character</param>
/// <returns>integer value</returns>
unsigned char EtherCardW5100::h2int ( char c )
{
    if ( c >= '0' && c <= '9' )
    {
        return ( ( unsigned char ) c - '0' );
    }
    if ( c >= 'a' && c <= 'f' )
    {
        return ( ( unsigned char ) c - 'a' + 10 );
    }
    if ( c >= 'A' && c <= 'F' )
    {
        return ( ( unsigned char ) c - 'A' + 10 );
    }
    return ( 0 );
}

/// <summary>
/// webutil.cpp - convert a single character to a 2 digit hex str
/// </summary>
/// <param name="c">single char</param>
/// <param name="hstr">2 digit hex string with terminating '\0'</param>
void EtherCardW5100::int2h ( char c, char *hstr )
{
    hstr[1] = ( c & 0xf ) + '0';
    if ( ( c & 0xf ) >9 )
    {
        hstr[1] = ( c & 0xf ) - 10 + 'a';
    }
    c = ( c >> 4 ) & 0xf;
    hstr[0] = c + '0';
    if ( c > 9 )
    {
        hstr[0] = c - 10 + 'a';
    }
    hstr[2] = '\0';
}

//================================================================
// Bufferfiller object
//================================================================

/// <summary>
/// This class populates network send and receive buffers.
/// This class provides formatted printing into memory. Users can use it to write into send buffers.
/// </summary>
/// <param name="fmt">PGM_P : is a pointer to a string in program space(defined in the source code)</param>
void BufferFiller::emit_p ( PGM_P fmt, ... )
{
    va_list ap;
    va_start ( ap, fmt );
    for ( ;; )
    {
        char c = pgm_read_byte ( fmt++ );
        if ( c == 0 )
            break;
        if ( c != '$' )
        {
            *ptr++ = c;
            continue;
        }
        c = pgm_read_byte ( fmt++ );
        switch ( c )
        {
        case 'D':
#ifdef __AVR__
            wtoa ( va_arg ( ap, uint16_t ), ( char* ) ptr );
#else
            wtoa ( va_arg ( ap, int ), ( char* ) ptr );
#endif
            break;
#ifdef FLOATEMIT
        case 'T':
            dtostrf ( va_arg ( ap, double ), 10, 3, ( char* ) ptr );
            break;
#endif
        case 'H':
        {
#ifdef __AVR__
            char p1 = va_arg ( ap, uint16_t );
#else
            char p1 = va_arg ( ap, int );
#endif
            char p2;
            p2 = ( p1 >> 4 ) & 0x0F;
            p1 = p1 & 0x0F;
            if ( p1 > 9 ) p1 += 0x07; // adjust 0x0a-0x0f to come out 'a'-'f'
            p1 += 0x30;             // and complete
            if ( p2 > 9 ) p2 += 0x07; // adjust 0x0a-0x0f to come out 'a'-'f'
            p2 += 0x30;             // and complete
            *ptr++ = p2;
            *ptr++ = p1;
            continue;
        }
        case 'L':
            ltoa ( va_arg ( ap, long ), ( char* ) ptr, 10 );
            break;
        case 'S':
            strcpy ( ( char* ) ptr, va_arg ( ap, const char* ) );
            break;
        case 'F':
        {
            PGM_P s = va_arg ( ap, PGM_P );
            char d;
            while ( ( d = pgm_read_byte ( s++ ) ) != 0 )
                *ptr++ = d;
            continue;
        }
        case 'E':
        {
            byte* s = va_arg ( ap, byte* );
            char d;
            while ( ( d = eeprom_read_byte ( s++ ) ) != 0 )
                *ptr++ = d;
            continue;
        }
        default:
            *ptr++ = c;
            continue;
        }
        ptr += strlen ( ( char* ) ptr );
    }
    va_end ( ap );
}

//================================================================

EtherCardW5100 ether;

// Declare static data members (ethercard.cpp)
uint8_t EtherCardW5100::mymac[6];				// MAC address
uint8_t EtherCardW5100::myip[4];				// IP address
uint8_t EtherCardW5100::netmask[4];				// Netmask
uint8_t EtherCardW5100::gwip[4];				// Gateway
uint8_t EtherCardW5100::dhcpip[4];				// DHCP server IP address
uint8_t EtherCardW5100::dnsip[4];				// DNS server IP address
uint8_t EtherCardW5100::hisip[4];				// DNS lookup result
uint16_t EtherCardW5100::hisport = 80;			// TCP port to connect to (default 80)
bool EtherCardW5100::using_dhcp;				// True if using DHCP
// NOT IMPLEMENTED
// uint8_t EtherCardW5100::broadcastip[4];		// Subnet broadcast address
// bool EtherCardW5100::persist_tcp_connection; // False to break connections on first packet received
// uint16_t EtherCardW5100::delaycnt = 0;		// Counts number of cycles of packetLoop when no packet received - used to trigger periodic gateway ARP request

// Declare static data members from enc28j60.h
uint16_t EtherCardW5100::bufferSize;

// Declare static data members for this wrapper class
IPAddress EtherCardW5100::ntpip;
#ifdef MY_PING
SOCKET EtherCardW5100::ping_socket = 0;
ICMPPing EtherCardW5100::ping ( ping_socket, 1 );
ICMPEchoReply EtherCardW5100::ping_result;
#endif

ETHERNES EtherCardW5100::incoming_server ( hisport );
//ETHERNES EtherCardW5100::incoming_server(80);
ETHERNEC EtherCardW5100::incoming_client;
ETHERNEC EtherCardW5100::outgoing_client;
ETHERNEUDP EtherCardW5100::udp_client;
DNSClient EtherCardW5100::dns_client;

// Declare static data (from tcpip.cpp)
static uint8_t www_fd = 0;						// ID of current http request (only one http request at a time - one of the 8 possible concurrent TCP/IP connections)
static const char *client_urlbuf;				// Pointer to c-string path part of HTTP request URL
static const char *client_urlbuf_var;			// Pointer to c-string filename part of HTTP request URL
static const char *client_hoststr;				// Pointer to c-string hostname of current HTTP request
static const char *client_additionalheaderline;	// Pointer to c-string additional http request header info
static const char *client_postval;
static tcpstate_t outgoing_client_state;		//TCP connection state: 1=Send SYN, 2=SYN sent awaiting SYN+ACK, 3=Established, 4=Not used, 5=Closing, 6=Closed
static void ( *client_browser_cb ) ( uint8_t, uint16_t, uint16_t );	// Pointer to callback function to handle result of current HTTP request

// External variables defined in main .ino file
extern BufferFiller bfill;

//=================================================================================
// Ethercard Wrapper Functions
//=================================================================================

/// <summary>
/// Initialise the network interface
/// </summary>
/// <param name="size">Size of data buffer (not used)</param>
/// <param name="macaddr">Hardware address to assign to the network interface (6 bytes) (not used)</param>
/// <param name="csPin">Arduino pin number connected to chip select. Default = 8</param>
/// <returns>Firmware version or zero on failure.</returns>
uint8_t EtherCardW5100::begin ( const uint16_t size, const uint8_t* macaddr, uint8_t csPin )
{
    using_dhcp = false;
    copyMac ( mymac, macaddr );
    return 1; //0 means fail
}
//////////////////////////////////////////////////////////////////scanNetwork()///////////////////////////////////////
String found_SSID[5], found_psw[5];

char* CharFromString(String uno) {
	
	char *a = new char[uno.length() +1];
		a[uno.length()] = 0;
	memcpy(a, uno.c_str(), uno.length());
	DEBUG_PRINT(a); DEBUG_PRINT(strlen(a));
/*
	int i = 0;
	while (uno[i++] != 0) {
		res[i - 1] = uno[i - 1]; DEBUG_PRINT(res[i - 1]);
	}*/
	return a;
}
byte scanNetwork(byte flag)
{
	byte netCount = 0;

#ifndef WIFIMANAGER
	Serial.println("scan start");

	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0)
	{
		Serial.println("no networks found");
		return 0;
	}
	else
	{
		String Ssid[10];
		String psw[10];
		File file;
		int PaswKnown = -1;
		int Npas = 0;
		if (!SPIFFS.exists("SSID_PASSWORD"))
			file = SPIFFS.open("SSID_PASSWORD", "w");               //new password file
		else
		{    //-----------------read SSID and PASSWORD from file.

			Serial.println("Reading password file....");
			file = SPIFFS.open("SSID_PASSWORD", "r+");               //read passwords from file
			while (file.available())
			{
				char buff[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
				file.readBytesUntil(',', buff, 20);

				Ssid[Npas] = buff;
				char buf[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

				int n=file.readBytesUntil('\n', buf, 20);
				buf[n-1] = 0;
				psw[Npas++] = buf;
				Serial.print(Ssid[Npas - 1]);
				Serial.print('\t'); Serial.print("<>"); Serial.println(psw[Npas - 1]);
			}
		}
		Serial.print(n);
		
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i)
		{
			// Print SSID and RSSI for each network found
			Serial.print(i);
			Serial.print(": ");
			int jpas = Npas - 1;
			while (WiFi.SSID(i) != Ssid[jpas] && jpas >= 0)jpas--;
			if (jpas >= 0)PaswKnown = i;
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.print((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");

			if (jpas >= 0) {
				Serial.println(" passw. available"); netCount++;
				found_SSID[netCount-1] = Ssid[jpas];
				found_psw[netCount-1] = psw[jpas];

			}

			else Serial.println();
			delay(10);
		}

		if (PaswKnown < 0||flag==1)
		{
			Serial.println("Select network n.");
			while (!Serial.available()) delay(10);
			byte ch = Serial.read();
			Ssid[Npas] = WiFi.SSID(ch - '0');
			Serial.print("Enter password for "); Serial.println(Ssid[Npas]);
			while (!Serial.available()) delay(10);
			psw[Npas] = Serial.readString();
			Serial.print(Ssid[Npas]); Serial.print(','); Serial.println(psw[Npas]);
			file.print(Ssid[Npas]); file.print(','); file.println(psw[Npas]);
			file.close();
			netCount++;
		}
	}
#	endif
	return netCount;
}

/// <summary>
/// Configure network interface with static IP
/// </summary>
/// <param name="my_ip">IP address (4 bytes). 0 for no change.</param>
/// <param name="gw_ip">Gateway address (4 bytes). 0 for no change. Default = 0</param>
/// <param name="dns_ip">DNS address (4 bytes). 0 for no change. Default = 0</param>
/// <param name="mask">Subnet mask (4 bytes). 0 for no change. Default = 0</param>
/// <returns>Returns true on success - actually always true</returns>
#ifdef WIFIMANAGER
#include "OpenSprinkler.h"
extern OpenSprinkler os;

void configModeCallback(WiFiManager *myWiFiManager) {
	os.lcd_print_line_clear_pgm(PSTR("Conf WiFi..."), 0);
	os.lcd_print_line_clear_pgm(PSTR("Go 192.168.4.1 "), 1);
	Serial.println("Go 192.168.4.1 ");
}
#endif
bool EtherCardW5100::staticSetup
(const uint8_t* my_ip, const uint8_t* gw_ip, const uint8_t* dns_ip, const uint8_t* mask)
{
	using_dhcp = false;

	
#ifdef ESP8266
#ifndef WIFIMANAGER
	uint8_t n = 0;
	bool result = false;
	uint8_t nNetwork= scanNetwork(0);
	while (n < nNetwork&&!result) {
		SSID =  CharFromString(found_SSID[n]);
		PASSWORD =  CharFromString(found_psw[n]);
		n++;
		if (PASSWORD[strlen(PASSWORD) - 1] < '0')PASSWORD[strlen(PASSWORD) - 1] = 0;

		DEBUG_PRINTLN(strlen(PASSWORD));
		result = WiFiconnect( my_ip, gw_ip,  dns_ip,  mask);
	}
#else
	WiFiManager wifiManager;
	IPAddress ip = IPAddress(my_ip[0],my_ip[1],my_ip[2],my_ip[3]);
	IPAddress gw = IPAddress(gw_ip[0], gw_ip[1], gw_ip[2], gw_ip[3]); 
	IPAddress subnet = IPAddress(255,255,255,0);
	DEBUG_PRINTLN(ip);
	wifiManager.setSTAStaticIPConfig(ip,gw, subnet);
	//wifiManager.setAPCallback(configModeCallback);


	wifiManager.autoConnect("AutoConnectAP");
#endif //WIFIMANAGER
#else
	
	IPAddress ip = Byte2IP ( my_ip );
	IPAddress gw = Byte2IP ( gw_ip );
	IPAddress subnet = Byte2IP ( mask );*/
	// initialize the ethernet device and start listening for clients
	ETHERNE.begin(mymac, ip, gw, subnet);

#endif
	if (result) {
		incoming_server.begin();



		// save the values
		IP2Byte(ETHERNE.localIP(), myip);
		IP2Byte(ETHERNE.gatewayIP(), gwip);
		IP2Byte(ETHERNE.gatewayIP(), dnsip);
		IP2Byte(ETHERNE.subnetMask(), netmask);

		// print debug values
		printIPConfig();
	}
    return result;
}
#ifdef ESP8266
bool EtherCardW5100::WiFiconnect()
{
	//if (WiFi.status() != WL_CONNECTED)
	{
		// convert bytes to IPAddress
//		IPAddress ip = Byte2IP(my_ip);
	//	IPAddress gw = Byte2IP(gw_ip);
		//IPAddress subnet = Byte2IP(mask);
		DEBUG_PRINTLN("Wait WIFI...");
		WiFi.mode(WIFI_STA);
	//	while (millis() < 60000) delay(2);
		WiFi.begin(SSID, PASSWORD);// , my_ip, dns_ip, gw_ip);
								   //		byte netmask[4] = { 255,255,255,0 };
								   //		byte MyIp[4] = { 192,168,1,211 };
								   //		byte MyGate[4] = { 192,168,1,1 };

	//	if (isStatic) WiFi.config(ip, gw, subnet);
		DEBUG_PRINT("\nConnecting to "); DEBUG_PRINTLN(SSID);
		uint8_t i = 0;
		while (WiFi.status() != WL_CONNECTED && i++ <= 201) { yield(); delay(400); DEBUG_PRINT('.'); }
		if (i >= 201) {
			DEBUG_PRINT("Could not connect to"); DEBUG_PRINTLN(SSID);
			return false;
		}
		DEBUG_PRINT("Server started IP="); DEBUG_PRINTLN(WiFi.localIP());
		return true;
	}
}
bool EtherCardW5100::WiFiconnect( const uint8_t* my_ip, const uint8_t* gw_ip, const uint8_t* dns_ip, const uint8_t* mask)
{
	//if (WiFi.status() != WL_CONNECTED)
	{
	// convert bytes to IPAddress
/*	IPAddress ip = Byte2IP ( my_ip );
	IPAddress gw = Byte2IP ( gw_ip );
	IPAddress subnet = Byte2IP ( mask );
	*/
		DEBUG_PRINTLN("Wait WIFI...");
		WiFi.mode(WIFI_STA);
		//while (millis() < 10000) delay(2);
		WiFi.begin(SSID, PASSWORD);// , my_ip, dns_ip, gw_ip);
		byte netmask[4] = { 255,255,255,0 };

		 WiFi.config(my_ip, gw_ip, netmask);
		DEBUG_PRINT("\nConnecting to "); DEBUG_PRINTLN(SSID);
		uint8_t i = 0;
		while (WiFi.status() != WL_CONNECTED && i++ <=201) { yield(); delay(400); DEBUG_PRINT('.'); }
		if (i >= 201) {
			DEBUG_PRINT("Could not connect to"); DEBUG_PRINTLN(SSID);
			return false;
		}

		DEBUG_PRINT("Server started IP="); DEBUG_PRINTLN(WiFi.localIP());
		return true;
	}
}
#endif


/// <summary>
/// Configure network interface with DHCP
/// </summary>
/// <param name="hname">hostname (not implememted)</param>
/// <param name="fromRam">lookup from RAM</param>
/// <returns>True if DHCP successful</returns>
bool EtherCardW5100::dhcpSetup ( const char *hname, bool fromRam )
{
    using_dhcp = true;

    // initialize the ethernet device
#ifndef ESP8266
	if (ETHERNE.begin(mymac) == 0)
#else
#ifndef WIFIMANAGER
	uint8_t n = 0;
	bool result = false;
	uint8_t nNetwork = scanNetwork(0);
	while (n < nNetwork&&!result) {
		SSID = CharFromString(found_SSID[n]);
		
		//   found_SSID[n];
		PASSWORD = CharFromString(found_psw[n]); //found_psw[n];
		//delete special char at the end/////////////////////////////////////
		if (PASSWORD[strlen(PASSWORD)-1] < '0')PASSWORD[strlen(PASSWORD)-1] = 0;
	
		DEBUG_PRINTLN(strlen(PASSWORD));
		n++;
		result = WiFiconnect();
	}
	if (!result )return false;
#else
	WiFiManager wifiManager;
	wifiManager.autoConnect("AutoConnectAP");
#endif

#ifdef HOSTNAM
	
		// Set up mDNS responder:
		// - first argument is the domain name, in this example
		//   the fully-qualified domain name is "esp8266.local"
		// - second argument is the IP address to advertise
		//   we send our IP address on the WiFi network
		char *  hostname = "OS";
#define  DHCP_HOSTNAME_MAX_LEN 10


		//do not  Ignore the hostname - need to extend the standard Arduino ethernet library to implement this
		if (hname != NULL)
		{
			if (fromRam)
			{
				strncpy(hostname, hname, DHCP_HOSTNAME_MAX_LEN);
			}
			else
			{
				strncpy_P(hostname, hname, DHCP_HOSTNAME_MAX_LEN);
			}
		}
		else
		{
			hostname = "OS";
			// Set a unique hostname, use Arduino-?? with last octet of mac address
			// ESP HW n. last 2 digit
			long cod = ESP.getChipId();

			hostname[3] = (cod % 10 + '0');
			hostname[2] = (cod % 100 / 10) + ('0');
			hostname[4] = 0;

		}
		//hostname = "prova";
		DEBUG_PRINT("Hostname:   ");
		DEBUG_PRINTLN(hostname);


		if (!MDNS.begin(hostname)) {
			Serial.println("Error setting up MDNS responder!");
			while (1) {
				delay(1000);
			}
		}
		Serial.println("mDNS responder started");
	
#endif
#endif
    
	// start listening for clients
    incoming_server.begin();
 //   udp_client.begin ( NTP_CLIENT_PORT );

    // save the values
    IP2Byte ( ETHERNE.localIP(), myip );
    IP2Byte (ETHERNE.gatewayIP(), gwip );
    IP2Byte (ETHERNE.gatewayIP(), dnsip );
    IP2Byte (ETHERNE.subnetMask(), netmask );

    // print debug values
    printIPConfig();
	delay(4000);
#ifndef HOSTNAM
    return true;
#else
#ifdef ESP8266	
	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);
	return true;
#else	
	DEBUG_PRINT(F("Hostname:   "));
	DEBUG_PRINT(hostname);
	//DEBUG_PRINTLN(F("(not implemented)"));

	dhcpState = DHCP_STATE_INIT;
	uint16_t start = millis();

	while (dhcpState != DHCP_STATE_BOUND && uint16_t(millis()) - start < 60000) {
	if (isLinkUp()) DhcpStateMachine(packetReceive());
	}
	updateBroadcastAddress();
	delaycnt = 0;
	return dhcpState == DHCP_STATE_BOUND;

#endif //ESP8266
#endif //HOSTNAM
}

/// <summary>
/// Print the current network configuration to the serial port
/// </summary>
void EtherCardW5100::printIPConfig()
{
#ifdef SERIAL_DEBUG
    DEBUG_PRINT ( F ( "Config:     " ) );
    DEBUG_PRINTLN ( using_dhcp ? F ( "DHCP" ) : F ( "Static IP" ) );

    DEBUG_PRINT ( F ( "MAC:        " ) );
    DEBUG_PRINTF ( mymac[0], HEX );
    DEBUG_PRINT ( F ( ":" ) );
    DEBUG_PRINTF ( mymac[1], HEX );
    DEBUG_PRINT ( F ( ":" ) );
    DEBUG_PRINTF ( mymac[2], HEX );
    DEBUG_PRINT ( F ( ":" ) );
    DEBUG_PRINTF ( mymac[3], HEX );
    DEBUG_PRINT ( F ( ":" ) );
    DEBUG_PRINTF ( mymac[4], HEX );
    DEBUG_PRINT ( F ( ":" ) );
    DEBUG_PRINTF ( mymac[5], HEX );
    DEBUG_PRINTLN();

    printIp ( F ( "Local IP:   " ), myip );
    printIp ( F ( "Gateway IP: " ), gwip );
    printIp ( F ( "DNS IP:     " ), dnsip );
    printIp ( F ( "Netmask:    " ), netmask );
#endif
}

/// <summary>
/// Parse received data
/// Note that data buffer is shared by receive and transmit functions
/// Only handles TCP (not UDP)
/// </summary>
/// <param name="plen">Size of data to parse(e.g. return value of packetReceive()).</param>
/// <returns>Offset of TCP payload data in data buffer or zero if packet processed</returns>
uint16_t EtherCardW5100::packetLoop ( uint16_t plen )
{
    uint16_t len = 0;

    // remember that plen passed in from packetReceive is data length plus
    // a dummy TCP header (the rest of the TCP packet is stripped out)
	//DEBUG_PRINTLN(plen);
    // nothing received
    if ( plen == 0 )
    {    
        // Receive incoming TCP data for client
        if ( outgoing_client_state == TCP_ESTABLISHED )
		{
			DEBUG_PRINT('>');
            if ( outgoing_client.available() )
            {
                DEBUG_PRINT ( F ( "Browse URL: client received: " ) );

                // set all bytes in the buffer to 0 - add a
                // byte to simulate space for the TCP header
                memset ( buffer, 0, ETHER_BUFFER_SIZE );
                memset ( buffer, ' ', TCP_OFFSET );
                len = TCP_OFFSET;

                while ( outgoing_client.available() && ( len < ETHER_BUFFER_SIZE ) )
                {
                    buffer[len] = outgoing_client.read();
                    DEBUG_PRINT ( char(buffer[len]&0xFF) );
                    len++;
                }
                DEBUG_PRINTLN ( F ( "" ) );

                // send data to the callback
                if ( len > TCP_OFFSET )
                {
                    ( *client_browser_cb ) ( 0, TCP_OFFSET, len );
                }
                // if the server has disconnected, stop the client
                if ( !outgoing_client.connected() )
                {
                    DEBUG_PRINTLN ( F ( "Browse URL: client disconnected" ) );
                    outgoing_client.stop();
                    outgoing_client_state = TCP_CLOSED;
                }
            }
        }

        // Check for renewal of DHCP lease
        if ( using_dhcp )
#ifndef ESP8266
            ETHERNE.maintain();  
#endif
        return 0;
    }
    else
    {
        //If we are here then this is a TCP/IP packet targetted at us and not
        // related to our client connection so accept treat it as a new request
        return TCP_OFFSET;
    }
}

/// <summary>
/// Send NTP request
/// </summary>
/// <param name="nt_pip">IP address of NTP server</param>
/// <param name="srcport">IP port to send from</param>
void EtherCardW5100::ntpRequest ( uint8_t *ntp_ip, uint8_t srcport )
{
    // set all bytes in the buffer to 0
    memset ( buffer, 0, ETHER_BUFFER_SIZE );

    // Initialize values needed to form NTP request
    buffer[0] = 0b11100011;    // LI, Version, Mode
    buffer[1] = 0;             // Stratum, or type of clock
    buffer[2] = 6;             // Polling Interval
    buffer[3] = 0xEC;          // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    buffer[12] = 49;
    buffer[13] = 0x4E;
    buffer[14] = 49;
    buffer[15] = 52;

    // If a zero IP address is set, use a pool ntp server (this is more reliable)
    if ( ntp_ip[0] == 0 && ntp_ip[1] == 0 && ntp_ip[2] == 0 && ntp_ip[3] == 0 )
    {
        if ( dnsLookup ( "pool.ntp.org", false ) )
            ntpip = IPAddress ( hisip[0], hisip[1], hisip[2], hisip[3] );
        else
        {
            DEBUG_PRINT ( F ( "NTP request failed - could not resolve IP" ) );
            return;
        }
    }
    else
        ntpip = IPAddress ( ntp_ip[0], ntp_ip[1], ntp_ip[2], ntp_ip[3] );

    // all NTP fields have been given values, now you can send a packet requesting a timestamp:
	udp_client.begin(srcport);

    udp_client.beginPacket ( ntpip, NTP_CLIENT_PORT );		// NTP requests are to port 123
    udp_client.write ( buffer, NTP_PACKET_SIZE );
    udp_client.endPacket();

    DEBUG_PRINT ( F ( "NTP request sent to " ) );
    printIp ( ntp_ip );
    DEBUG_PRINT ( F ( ": " ) );

#ifdef SERIAL_DEBUG
    for ( uint8_t c = 0; c < 16; c++ )
    {
        DEBUG_PRINTF ( buffer[c], HEX );
        DEBUG_PRINT ( F ( " " ) );
    }
#endif
    DEBUG_PRINTLN ( "" );
}

/// added 
int EtherCardW5100::Udp_parsePacket() { return udp_client.parsePacket(); }

/// <summary>
/// Ethercard.cpp - Process network time protocol response
/// </summary>
/// <param name="time">Pointer to integer to hold result</param>
/// <param name="dstport_l">Destination port to expect response. Set to zero to accept on any port</param>
/// <returns>True (1) on success</returns>
byte EtherCardW5100::ntpProcessAnswer ( uint32_t *time, byte dstport_l )
{
    int packetSize = udp_client.parsePacket();
    if ( packetSize )
    {
        DEBUG_PRINT ( F ( "NTP response received from " ) );
        DEBUG_PRINT ( udp_client.remoteIP() [0] );
        DEBUG_PRINT ( F ( "." ) );
        DEBUG_PRINT ( udp_client.remoteIP() [1] );
        DEBUG_PRINT ( F ( "." ) );
        DEBUG_PRINT ( udp_client.remoteIP() [2] );
        DEBUG_PRINT ( F ( "." ) );
        DEBUG_PRINT ( udp_client.remoteIP() [3] );
        DEBUG_PRINT ( F ( ":" ) );

        // check the packet is from the correct timeserver IP and port
        if ( udp_client.remotePort() != 123 || udp_client.remoteIP() != ntpip )
        {
            DEBUG_PRINTLN ( F ( " (invalid IP or port)" ) );
            return 0;
        }

        //the timestamp starts at byte 40 of the received packet and is four bytes, or two words, long.
        udp_client.read ( buffer, packetSize );
        ( ( byte* ) time ) [3] = buffer[40];
        ( ( byte* ) time ) [2] = buffer[41];
        ( ( byte* ) time ) [1] = buffer[42];
        ( ( byte* ) time ) [0] = buffer[43];

        DEBUG_PRINTLN ( ( uint32_t ) time );
        return 1;
    }
    return 0;
}

//=================================================================================
// Webutil.cpp
//=================================================================================

/// <summary>
/// webutil.cpp - copies an IP address
/// There is no check of source or destination size. Ensure both are 4 bytes
/// </summary>
/// <param name="dst">dst Pointer to the 4 byte destination</param>
/// <param name="src">src Pointer to the 4 byte source</param>
void EtherCardW5100::copyIp ( byte *dst, const byte *src )
{
    memcpy ( dst, src, 4 );
}

/// <summary>
/// webutil.cpp - copies a hardware address
/// There is no check of source or destination size. Ensure both are 6 bytes
/// </summary>
/// <param name="dst">dst Pointer to the 6 byte destination</param>
/// <param name="src">src Pointer to the 6 byte destination</param>
void EtherCardW5100::copyMac ( byte *dst, const byte *src )
{
    memcpy ( dst, src, 6 );
}

void EtherCardW5100::printIp ( const char* msg, const uint8_t *buf )
{
    Serial.print ( msg );
    EtherCardW5100::printIp ( buf );
    Serial.println();
}

void EtherCardW5100::printIp ( const __FlashStringHelper *ifsh, const uint8_t *buf )
{
    DEBUG_PRINT ( ifsh );
    EtherCardW5100::printIp ( buf );
    DEBUG_PRINTLN();
}

void EtherCardW5100::printIp ( const uint8_t *buf )
{
    for ( uint8_t i = 0; i < 4; ++i )
    {
        DEBUG_PRINTF ( buf[i], DEC );
        if ( i < 3 )
            DEBUG_PRINT ( '.' );
    }
}

/// <summary>
/// webutil.cpp - search for a string of the form key=value in a string that looks like q?xyz=abc&uvw=defgh HTTP/1.1\\r\\n
/// Ensure strbuf has memory allocated of at least maxlen + 1 (to accomodate result plus terminating null)
/// </summary>
/// <param name="str">Pointer to the null terminated string to search</param>
/// <param name="strbuf">Pointer to buffer to hold null terminated result string</param>
/// <param name="maxlen">Maximum length of result</param>
/// <param name="key">Pointer to null terminated string holding the key to search for</param>
/// <returns>Length of the value. 0 if not found</returns>
byte EtherCardW5100::findKeyVal ( const char *str, char *strbuf, byte maxlen, const char *key )
{
    byte found = 0;
    byte i = 0;
    const char *kp;
    kp = key;
    while ( *str &&  *str != ' ' && *str != '\n' && found == 0 )
    {
        if ( *str == *kp )
        {
            kp++;
            if ( *kp == '\0' )
            {
                str++;
                kp = key;
                if ( *str == '=' )
                {
                    found = 1;
                }
            }
        }
        else
        {
            kp = key;
        }
        str++;
    }
    if ( found == 1 )
    {
        // copy the value to a buffer and terminate it with '\0'
        while ( *str &&  *str != ' ' && *str != '\n' && *str != '&' && i<maxlen - 1 )
        {
            *strbuf = *str;
            i++;
            str++;
            strbuf++;
        }
        *strbuf = '\0';
    }
    // return the length of the value
    return ( i );
}

/// <summary>
/// webutil.cpp - decode a URL string e.g "hello%20joe" or "hello+joe" becomes "hello joe"
/// </summary>
/// <param name="urlbuf">Pointer to the null terminated URL (urlbuf is modified)</param>
void EtherCardW5100::urlDecode ( char *urlbuf )
{
    char c;
    char *dst = urlbuf;
    while ( ( c = *urlbuf ) != 0 )
    {
        if ( c == '+' ) c = ' ';
        if ( c == '%' )
        {
            c = *++urlbuf;
            c = ( h2int ( c ) << 4 ) | h2int ( *++urlbuf );
        }
        *dst++ = c;
        urlbuf++;
    }
    *dst = '\0';
}

/// <summary>
/// webutil.cpp - encode a URL, replacing illegal charaters like ' '.
/// There must be enough space in urlbuf. In the worst case that is 3 times the length of str
/// </summary>
/// <param name="str">str Pointer to the null terminated string to encode</param>
/// <param name="urlbuf">urlbuf Pointer to a buffer to contain the null terminated encoded URL</param>
void EtherCardW5100::urlEncode ( char *str, char *urlbuf )
{
    char c;
    while ( ( c = *str ) != 0 )
    {
        if ( c == ' ' || isalnum ( c ) )
        {
            if ( c == ' ' )
            {
                c = '+';
            }
            *urlbuf = c;
            str++;
            urlbuf++;
            continue;
        }
        *urlbuf = '%';
        urlbuf++;
        int2h ( c, urlbuf );
        urlbuf++;
        urlbuf++;
        str++;
    }
    *urlbuf = '\0';
}

/// <summary>
/// webutil.cpp - Convert an IP address from dotted decimal formated string to 4 bytes
/// </summary>
/// <param name="bytestr">Pointer to the 4 byte array to store IP address</param>
/// <param name="str">Pointer to string to parse</param>
/// <returns>0 on success</returns>
byte EtherCardW5100::parseIp ( byte *bytestr, char *str )
{
    char *sptr;
    byte i = 0;
    sptr = NULL;
    while ( i<4 )
    {
        bytestr[i] = 0;
        i++;
    }
    i = 0;
    while ( *str && i<4 )
    {
        // if a number then start
        if ( sptr == NULL && isdigit ( *str ) )
        {
            sptr = str;
        }
        if ( *str == '.' )
        {
            *str = '\0';
            bytestr[i] = ( atoi ( sptr ) & 0xff );
            i++;
            sptr = NULL;
        }
        str++;
    }
    *str = '\0';
    if ( i == 3 )
    {
        bytestr[i] = ( atoi ( sptr ) & 0xff );
        return ( 0 );
    }
    return ( 1 );
}

/// <summary>
/// webutil.cpp - take a byte string and convert it to a human readable display string
/// </summary>
/// <param name="resultstr">Pointer to a buffer to hold the resulting null terminated string</param>
/// <param name="bytestr">Pointer to the byte array containing the address to convert</param>
/// <param name="len">Length of the array (4 for IP address, 6 for hardware (MAC) address)</param>
/// <param name="separator">Delimiter character (typically '.' for IP address and ':' for hardware (MAC) address)</param>
/// <param name="base">Base for numerical representation (typically 10 for IP address and 16 for hardware (MAC) address</param>
void EtherCardW5100::makeNetStr ( char *resultstr, byte *bytestr, byte len, char separator, byte base )
{
    byte i = 0;
    byte j = 0;
    while ( i<len )
    {
        itoa ( ( int ) bytestr[i], &resultstr[j], base );
        // search end of str:
        while ( resultstr[j] )
        {
            j++;
        }
        resultstr[j] = separator;
        j++;
        i++;
    }
    j--;
    resultstr[j] = '\0';
}

//=================================================================================
// enc28j60.cpp
//=================================================================================

/// <summary>
/// enc28j60.cpp - copy recieved packets to data buffer
/// Data buffer is shared by recieve and transmit functions
/// </summary>
/// <returns>Size of recieved data packet</returns>
uint16_t EtherCardW5100::packetReceive()
{
    // listen for incoming clients
    // ** remember this client object is different from the W5100client used for tcp requests **
    incoming_client = incoming_server.available();
    if ( incoming_client )
    {
		DEBUG_PRINTLN(incoming_client);

        // set all bytes in the buffer to 0 - add a
        // byte to simulate space for the TCP header
        memset ( buffer, 0, ETHER_BUFFER_SIZE );
        memset ( buffer, ' ', TCP_OFFSET );
        uint16_t i = TCP_OFFSET; // add a space for TCP offset

        DEBUG_PRINT ( F ( "Server request: " ) );

        while ( incoming_client.connected() && ( i < ETHER_BUFFER_SIZE ) )
        {
            if ( incoming_client.available() )
                buffer[i] = incoming_client.read();

            // print readable ascii characters
            if ( buffer[i] >= 0x08 && buffer[i] <= 0x0D )
            {
                DEBUG_PRINT ( F ( " " ) );		// substitute a space so less rows in serial output
            }
            else if ( buffer[i] > 0x1F )
            {
                DEBUG_PRINT ( ( char ) buffer[i] );
            }

            i++;
        }
        DEBUG_PRINTLN ( F ( "" ) );
        return i;
    }
    else
        return 0;
}

//=================================================================================
// tcpip.cpp
//=================================================================================

/// <summary>
/// tcpip.cpp - send a response to a HTTP request
/// </summary>
/// <param name="dlen">Size of the HTTP (TCP) payload</param>
void EtherCardW5100::httpServerReply ( word dlen )
{
    // ignore dlen - just add a null termination
    // to the buffer and print it out to the client
    buffer[bfill.position() + TCP_OFFSET] = '\0';
    incoming_client.print ( ( char* ) bfill.buffer() );

    // close the connection:
    delay ( 1 ); // give the web browser time to receive the data
    incoming_client.stop();
}

/// <summary>
/// tcpip.cpp - send a response to a HTTP request
/// </summary>
/// <param name="dlen">Size of the HTTP (TCP) payload</param>
/// <param name="flags">TCP flags</param>
void EtherCardW5100::httpServerReply_with_flags ( uint16_t dlen, uint8_t flags )
{
    // Need functionality to handle:
    //     - TCP_FLAGS_ACK_V
    //     - TCP_FLAGS_FIN_V
    //     - TCP_FLAGS_ACK_V|TCP_FLAGS_FIN_V

    // Same as above - ignore dlen & just add a null termination and print it out to the client
    buffer[bfill.position() + TCP_OFFSET] = '\0';
    incoming_client.print ( ( char* ) bfill.buffer() );
    delay ( 1 ); // give the web browser time to receive the data

    if ( flags&TCP_FLAGS_FIN_V != 0 ) // final packet in the stream
    {
        // close the connection:
        incoming_client.stop();
    }

    /* // Original Code from tcpip.cpp
    for(byte i=0;i<=dup;i++)
    {
    set_seq();
    gPB[TCP_FLAGS_P] = flags; // final packet
    make_tcp_ack_with_data_noflags(dlen); // send data
    }
    //if(keepseq) {}
    //else {SEQ=SEQ+dlen;}
    SEQ=SEQ+dlen;
    */
}

/// <summary>
/// tcpip.cpp - acknowledge TCP message
/// </summary>
void EtherCardW5100::httpServerReplyAck()
{
    /*
    make_tcp_ack_from_any(info_data_len,0); // send ack for http get
    get_seq(); //get the sequence number of packets after an ack from GET
    */
}

/// <summary>
/// Populate the bufferfiller with www request header
/// </summary>
/// <param name="fd">File descriptor (always www_fd)</param>
/// <returns>current buffer filler position including TCP_OFFSET</returns>
uint16_t EtherCardW5100::www_client_internal_datafill_cb ( uint8_t fd )
{
    bfill = EtherCardW5100::tcpOffset();

    if ( fd == www_fd )
    {
        if ( client_postval == 0 )
        {
            bfill.emit_p ( PSTR ( "GET $F$S HTTP/1.0\r\n"
                                  "Host: $F\r\n"
                                  "$F\r\n"
                                  "\r\n" ), client_urlbuf,
                           client_urlbuf_var,
                           client_hoststr, client_additionalheaderline );
        }
        else
        {
            const char* ahl = client_additionalheaderline;
            bfill.emit_p ( PSTR ( "POST $F HTTP/1.0\r\n"
                                  "Host: $F\r\n"
                                  "$F$S"
                                  "Accept: */*\r\n"
                                  "Content-Length: $D\r\n"
                                  "Content-Type: application/x-www-form-urlencoded\r\n"
                                  "\r\n"
                                  "$S" ), client_urlbuf,
                           client_hoststr,
                           ahl != 0 ? ahl : PSTR ( "" ),
                           ahl != 0 ? "\r\n" : "",
                           strlen ( client_postval ),
                           client_postval );
        }
    }
    return bfill.position();
}

/// <summary>
/// tcpip.cpp - prepare and send HTTP request
/// </summary>
/// <param name="urlbuf">Pointer to c-string URL folder</param>
/// <param name="urlbuf_varpart">Pointer to c-string URL file</param>
/// <param name="hoststr">Pointer to c-string hostname</param>
/// <param name="callback">Pointer to callback function to handle response</param>
void EtherCardW5100::browseUrl ( const char *urlbuf, const char *urlbuf_varpart, const char *hoststr, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) )
{
    browseUrl ( urlbuf, urlbuf_varpart, hoststr, PSTR ( "Accept: text/html" ), callback );
}

/// <summary>
/// tcpip.cpp - prepare and send HTTP request / the reply is received in the main packetloop
/// </summary>
/// <param name="urlbuf">Pointer to c-string URL folder</param>
/// <param name="urlbuf_varpart">Pointer to c-string URL file</param>
/// <param name="hoststr">Pointer to c-string hostname</param>
/// <param name="additionalheaderline">additionalheaderline Pointer to c-string with additional HTTP header info</param>
/// <param name="callback">callback Pointer to callback function to handle response</param>
void EtherCardW5100::browseUrl ( const char *urlbuf, const char *urlbuf_varpart, const char *hoststr, const char *additionalheaderline, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) )
{
    client_urlbuf = urlbuf;
    client_urlbuf_var = urlbuf_varpart;
    client_hoststr = hoststr;
    client_additionalheaderline = additionalheaderline;
    client_postval = 0;
    client_browser_cb = callback;
//    client_tcp_datafill_cb = &www_client_internal_datafill_cb;

    // check cb pointer is 'real' (non zero)
    if ( !client_browser_cb )
        return;
	DEBUG_PRINT("hoststring=");
	for (int i = 0; i < strlen(hoststr); i++)DEBUG_PRINT(hoststr[i]);

    // fill the buffer  host_name_should be loaded here
    uint16_t len = ( *www_client_internal_datafill_cb ) ( www_fd );
    buffer[bfill.position() + TCP_OFFSET] = '\0';

    DEBUG_PRINT ( F ( "Browse URL: " ) );

    for ( uint16_t c = TCP_OFFSET; c < bfill.position() + TCP_OFFSET; c++ )
    {
        // print readable ascii characters
        if ( buffer[c] >= 0x08 && buffer[c] <= 0x0D )
        {
            DEBUG_PRINT ( ( char ) buffer[c] ); // DEBUG_PRINT ( F ( " " ) );		// substitute a space so less rows in serial output
        }
        else if ( buffer[c] > 0x1F )
        {
            DEBUG_PRINT ( ( char ) buffer[c] );
        }
    }
    DEBUG_PRINTLN ( F ( "" ) );

    // close any connection before send a new request, to free the socket
    outgoing_client.stop();
	delay(1000);
    // send the request
    if ( outgoing_client.connect (hoststr, hisport ) )
		//-------------modified *hoststr now=weather.opensprinkler.com was "*"
    {
        // send the HTTP GET request:
        outgoing_client.print ( ( char* ) bfill.buffer() );
        outgoing_client.println();
        DEBUG_PRINT ( F ( "Browse URL: sent to " ) );
        DEBUG_PRINT ( hoststr );
        DEBUG_PRINT ( F ( " port " ) );
        DEBUG_PRINT ( hisport );
        DEBUG_PRINTLN ( F ( "(OK)" ) );
        outgoing_client_state = TCP_ESTABLISHED;
    }
    else
    {
        DEBUG_PRINTLN ( F ( "Browse URL: failed (could not connect)" ) );
        outgoing_client_state = TCP_CLOSED;
    }
}

/// <summary>
/// tcp.cpp - send ping
/// </summary>
/// <param name="destip">Pointer to 4 byte destination IP address</param>
void EtherCardW5100::clientIcmpRequest ( const uint8_t *destip )
{
#ifdef MY_PING
    IPAddress pingAddr ( destip[0], destip[1], destip[2], destip[3] ); // ip address to ping

    DEBUG_PRINT ( F ( "Ping: to " ) );
    DEBUG_PRINT ( destip[0] );
    DEBUG_PRINT ( F ( "." ) );
    DEBUG_PRINT ( destip[1] );
    DEBUG_PRINT ( F ( "." ) );
    DEBUG_PRINT ( destip[2] );
    DEBUG_PRINT ( F ( "." ) );
    DEBUG_PRINT ( destip[3] );

    // note - asynchStart will return false if we couldn't even send a ping
    if ( !ping.asyncStart ( pingAddr, 3, ping_result ) )
    {
        DEBUG_PRINT ( F ( " send failed (status=" ) );
        DEBUG_PRINT ( ( int ) ping_result.status );
        DEBUG_PRINTLN ( F ( ")" ) );
    }
    else
    {
        DEBUG_PRINTLN ( F ( " sent (OK)" ) );
    }
#endif

}

/// <summary>
/// tcp.cpp - check for ping response
/// </summary>
/// <param name="ip_monitoredhost">Pointer to 4 byte IP address of host to check</param>
/// <returns>True (1) if ping response from specified host</returns>
uint8_t EtherCardW5100::packetLoopIcmpCheckReply ( const uint8_t *ip_monitoredhost )
{
#ifndef MY_PING

	if (WiFi.status() != WL_CONNECTED) return true; else return false;
#else
    if ( ping.asyncComplete ( ping_result ) )
    {
        DEBUG_PRINT ( F ( "Ping: " ) );

        if ( ping_result.status != SUCCESS )
        {
            // failure... but whyyyy?
            DEBUG_PRINT ( F ( " failed (status=" ) );
            DEBUG_PRINT ( ping_result.status );
            DEBUG_PRINTLN ( F ( ")" ) );
            return 0;
        }
        else
        {
            // huzzah
            DEBUG_PRINT ( F ( " reply " ) );
            DEBUG_PRINT ( ping_result.data.seq );
            DEBUG_PRINT ( F ( " from " ) );
            DEBUG_PRINT ( ping_result.addr[0] );
            DEBUG_PRINT ( F ( "." ) );
            DEBUG_PRINT ( ping_result.addr[1] );
            DEBUG_PRINT ( F ( "." ) );
            DEBUG_PRINT ( ping_result.addr[2] );
            DEBUG_PRINT ( F ( "." ) );
            DEBUG_PRINT ( ping_result.addr[3] );
            DEBUG_PRINT ( F ( " bytes=" ) );
            DEBUG_PRINT ( REQ_DATASIZE );
            DEBUG_PRINT ( F ( " time=" ) );
            DEBUG_PRINT ( millis() - ping_result.data.time );
            DEBUG_PRINT ( F ( " TTL=" ) );
            DEBUG_PRINT ( ping_result.ttl );

            // check the address
            if ( ping_result.addr[0] == ip_monitoredhost[0] &&
                    ping_result.addr[1] == ip_monitoredhost[1] &&
                    ping_result.addr[2] == ip_monitoredhost[2] &&
                    ping_result.addr[3] == ip_monitoredhost[3] )
            {
                DEBUG_PRINTLN ( F ( " (OK)" ) );
                return 1;
            }
            else
            {
                DEBUG_PRINTLN ( F ( " (received from wrong host)" ) );
                return 0;
            }
        }
    }
#endif
}

/*
/// <summary>
/// get the sequence number of packets after an ack from GET
/// </summary>
static void get_seq()
{
    SEQ = ( ( ( unsigned long ) gPB[TCP_SEQ_H_P] * 256 + gPB[TCP_SEQ_H_P + 1] ) * 256 + gPB[TCP_SEQ_H_P + 2] ) * 256 + gPB[TCP_SEQ_H_P + 3];
} //thanks to mstuetz for the missing (unsigned long)

/// <summary>
/// Set the correct sequence number and calculate the next with the length of current packet
/// </summary>
static void set_seq()
{
    gPB[TCP_SEQ_H_P] = ( SEQ & 0xff000000 ) >> 24;
    gPB[TCP_SEQ_H_P + 1] = ( SEQ & 0xff0000 ) >> 16;
    gPB[TCP_SEQ_H_P + 2] = ( SEQ & 0xff00 ) >> 8;
    gPB[TCP_SEQ_H_P + 3] = ( SEQ & 0xff );
}
*/
//=================================================================================
// dns.cpp
//=================================================================================

/// /// <summary>
/// dns.cpp - perform DNS lookup. Result is stored in hisip member.
/// </summary>
/// <param name="name">Host name to lookup</param>
/// <param name="fromRam">NOT IMPLEMENTED (Look up cached name. Default = false)</param>
/// <returns>True on success. </returns>
bool EtherCardW5100::dnsLookup ( const char* name, bool fromRam )
{

    IPAddress serverIP ( 0, 0, 0, 0 );
	DEBUG_PRINT(name);
	DEBUG_PRINTLN(" Starting LOOKUP");
#ifndef ESP8266  //use WiFi.hostByName
    
	dns_client.begin(Ethernet.dnsServerIP());
    int result = dns_client.getHostByName ( name, serverIP );
#else
	
	dns_client.begin ( ETHERNE.gatewayIP());
	int result = ETHERNE.hostByName(name, serverIP);
#endif

    DEBUG_PRINT ( F ( "DNS lookup " ) );
    DEBUG_PRINT ( name );
    DEBUG_PRINT ( F ( " is " ) );
    DEBUG_PRINT ( serverIP[0] );
    DEBUG_PRINT ( F ( "." ) );
    DEBUG_PRINT ( serverIP[1] );
    DEBUG_PRINT ( F ( "." ) );
    DEBUG_PRINT ( serverIP[2] );
    DEBUG_PRINT ( F ( "." ) );
    DEBUG_PRINT ( serverIP[3] );

    for ( uint8_t i = 0; i < 4; i++ )
        hisip[i] = serverIP[i];

    if ( result == 1 )
    {
        DEBUG_PRINTLN ( F ( " (OK)" ) );
        return true;
    }
    else
    {
        DEBUG_PRINTLN ( F ( " (failed)" ) );
        return false;
    }
}









