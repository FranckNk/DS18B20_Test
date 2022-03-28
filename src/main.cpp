#include <OneWire.h>
#include <DallasTemperature.h>
#include "WIFIConnector_MKR1000.h"
#include "MQTTConnector.h"
#include <Wire.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9 // Lower resolution

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
void printAddress(DeviceAddress deviceAddress);
String AdresseTemp;

byte AdresseMac[6];
float Temperature = 0;

void setup(void)
{
	// start serial port
	Serial.begin(9600);
	Serial.println("Dallas Temperature IC Control Library Demo");
	// delay(3000);
	// Configuration du wifi et du serveur MQTT.
	wifiConnect();
	MQTTConnect();

	// Récupération de l'adresse MAC du uC.
	WiFi.macAddress(AdresseMac);
	

	// Start up the library
	sensors.begin();
	
	// Grab a count of devices on the wire
	numberOfDevices = sensors.getDeviceCount();
	
	// locate devices on the bus
	Serial.print("Locating devices...");
	
	Serial.print("Found ");
	Serial.print(numberOfDevices, DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: "); 
	if (sensors.isParasitePowerMode()) Serial.println("ON");
	else Serial.println("OFF");
	
	// Loop through each device, print out address
	for(int i=0;i<numberOfDevices; i++)
	{
		// Search the wire for address
		if(sensors.getAddress(tempDeviceAddress, i))
		{
			Serial.print("Found device ");
			Serial.print(i, DEC);
			Serial.print(" with address: ");
			printAddress(tempDeviceAddress);
			Serial.println();
			
			Serial.print("Setting resolution to ");
			Serial.println(TEMPERATURE_PRECISION, DEC);
			
			// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
			sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
			
			Serial.print("Resolution actually set to: ");
			Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
			Serial.println();
		}else{
			Serial.print("Found ghost device at ");
			Serial.print(i, DEC);
			Serial.print(" but could not detect address. Check power and cabling");
		}
	}
	AdresseTemp = "";
	// Création de la chaine et envoi de l'adresse MAC sur T.B.
	for (int i = 5; i >= 0; i--)
	{
		AdresseTemp = AdresseTemp + String(AdresseMac[i], HEX);
		// AdresseTemp = AdresseTemp + ":";
	}
	AdresseTemp.toUpperCase();
	Serial.println(AdresseTemp);
	
	appendPayloadMac("AdresseMac" ,AdresseTemp);

}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
	// method 1 - slower
	//Serial.print("Temp C: ");
	//Serial.print(sensors.getTempC(deviceAddress));
	//Serial.print(" Temp F: ");
	//Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

	// method 2 - faster
	float tempC = sensors.getTempC(deviceAddress);
	if(tempC == DEVICE_DISCONNECTED_C) 
	{
		Serial.println("Error: Could not read temperature data");
		return;
	}
	Serial.print("Temp C: ");
	Serial.print(tempC);
	Serial.print(" Temp F: ");
	Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

void loop(void)
{ 
	// call sensors.requestTemperatures() to issue a global temperature 
	// request to all devices on the bus
	Serial.print("Requesting temperatures...");
	sensors.requestTemperatures(); // Send the command to get temperatures
	Serial.println("DONE");
	
	
	// Loop through each device, print out temperature data
	for(int i=0;i<numberOfDevices; i++)
	{
		// Search the wire for address
		if(sensors.getAddress(tempDeviceAddress, i))
		{
			// Output the device ID
			Serial.print("Temperature for device: ");
			Serial.println(i,DEC);
			
			// It responds almost immediately. Let's print out the data
			printTemperature(tempDeviceAddress); // Use a simple function to print out the data
		} 
		//else ghost device! Check your power requirements and cabling
		
		printAddress(tempDeviceAddress);
		Serial.println(AdresseTemp);
		AdresseTemp.toUpperCase();
		appendPayload(AdresseTemp, sensors.getTempC(tempDeviceAddress));
		sendPayload();
	}
		delay(10000);
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	AdresseTemp = "";
	for (uint8_t i = 0; i < 8; i++)
	{
		//if (deviceAddress[i] < 16) Serial.print("0");
		AdresseTemp = AdresseTemp + String(deviceAddress[i], HEX);
	}
	Serial.println(AdresseTemp);	
}