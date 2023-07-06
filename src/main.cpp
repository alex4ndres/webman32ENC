#include <Arduino.h>
#include <SPI.h>
#include <EthernetENC.h>
#include <EthernetClient.h>
#include <manager_html.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <config.h>


// -------------------------------
// ----- NETWORK DATA STRUCT -----
// -------------------------------
struct NetworkData {
  char ip_address[64];
  char http_port[64];
  char gateway[64];
  char dns[64];
  char mask[64];
  char firmwareVersion[32] = "V0.1.0";
};
NetworkData myNetworkData;


// ---------------------------------
// ----- FUNCTION DECLARATIONS -----
// ---------------------------------
void Check_Ethernet_Cable();
void Setup_Ethernet();
void replacePlaceholder(String& html, const char* placeholder, const char* value);
void saveNetworkSettings(EthernetClient client, String requestData);
void handleRequests();
void ReadNetworkDataFromSPIFFS();
void SaveNetworkDataSPIFFS();
void restartDevice(EthernetClient client);


// ---------------------------------
// ----- VARIABLE DECLARATIONS -----
// ---------------------------------
uint8_t macAddress[6]; 
char macHex[100];
EthernetServer server(80);
IPAddress ip (192,168,2,18);


/*
  @brief Setup Function.
  @param none.
  @return none.
*/
void setup() 
{
  Serial.begin(115200);
  Serial.println("Initializing Device");
  SPIFFS.begin(true);
  if(!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS mount failed!");
    return;
  }
  ReadNetworkDataFromSPIFFS();

  Check_Ethernet_Cable();
  Ethernet.init(5);
  Setup_Ethernet();
  server.begin();
}


/*
  @brief Loop Function.
  @param none.
  @return none.
*/
void loop() 
{
  handleRequests();
}


/*
  @brief Configures the ethernet module and shows the IP, mask, gateway and DNS.
  @param none.
  @return none.
*/
void Setup_Ethernet()
{
    WiFi.macAddress(macAddress);
    sprintf(macHex, "%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    Serial.print("Mac Adress: ");
    Serial.println(macHex);

    Ethernet.begin(macAddress, ip); //, ip, _dns, _gateway, _subnetMask
    delay(4000);
    Serial.print("Local IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP : ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : ");
    Serial.println(Ethernet.dnsServerIP());
}


/*
  @brief Function to check if ethernet cable is connected.
  @param none.
  @return none.
*/
void Check_Ethernet_Cable()
{
    while (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("Ethernet Cable Disconnected...");
        delay(1000);               
    }
}


/*
  @brief Function to replace the values in the HTML page.
  @param Value String, const char* and const char*.
  @return none.
*/
void replacePlaceholder(String& html, const char* placeholder, const char* value) {
  int startIndex = html.indexOf(placeholder);
  if (startIndex != -1) {
    int placeholderLen = strlen(placeholder);
    String before = html.substring(0, startIndex);
    String after = html.substring(startIndex + placeholderLen);
    html = before + value + after;
  }
}


/*
  @brief Function to handle the GET request to restart the device
  @param Value EthernetClient.
  @return none.
*/
void restartDevice(EthernetClient client) 
{
  Serial.println("Restarting Device...");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<h2>Device restarting...</h2>");
  delay(5000);
  ESP.restart();
}


/*
  @brief Function to handle incoming requests.
  @param none.
  @return none.
*/
void handleRequests() 
{
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        //String requestData = client.readStringUntil('\r');
        String requestData = client.readString();
        client.flush();

        if (requestData.startsWith("GET / ")) 
        {
          String html = String(manager_html);
          replacePlaceholder(html, "%IPADDRESS%", myNetworkData.ip_address);
          replacePlaceholder(html, "%PORT%", myNetworkData.http_port);
          replacePlaceholder(html, "%GATEWAY%", myNetworkData.gateway);
          replacePlaceholder(html, "%DNS%", myNetworkData.dns);
          replacePlaceholder(html, "%MASK%", myNetworkData.mask);
          replacePlaceholder(html, "%FIRMWAREVERSION%", myNetworkData.firmwareVersion);
          replacePlaceholder(html, "%MACADDRESS%", macHex); 

          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();
          client.println(html);
        } 
        else if (requestData.startsWith("POST /networksettings")) 
        {
          saveNetworkSettings(client, requestData);
        } 
        else if (requestData.startsWith("GET /restartdevice")) 
        {
          restartDevice(client);
        } 
        else 
        {
          client.println("HTTP/1.1 404 Not Found");
          client.println("Content-type:text/html");
          client.println();
          client.println("<h2>Page not found</h2>");
        }
        break;
      }
    }
    delay(10);
    client.stop();
  }
}


/*
  @brief Function to process the POST request and save the data in the struct.
  @param none.
  @return none.
*/
void saveNetworkSettings(EthernetClient client, String requestData) 
{
  String ip = requestData.substring(requestData.indexOf("ipInput=") + 8);
  ip = ip.substring(0, ip.indexOf("&"));

  String port = requestData.substring(requestData.indexOf("portInput=") + 10);
  port = port.substring(0, port.indexOf("&"));

  String gateway = requestData.substring(requestData.indexOf("gatewayInput=") + 13);
  gateway = gateway.substring(0, gateway.indexOf("&"));

  String dns = requestData.substring(requestData.indexOf("dnsInput=") + 9);
  dns = dns.substring(0, dns.indexOf("&"));

  String mask = requestData.substring(requestData.indexOf("maskInput=") + 10);
  mask = mask.substring(0, mask.indexOf("&"));

  ip.toCharArray(myNetworkData.ip_address, sizeof(myNetworkData.ip_address));
  port.toCharArray(myNetworkData.http_port, sizeof(myNetworkData.http_port));
  gateway.toCharArray(myNetworkData.gateway, sizeof(myNetworkData.gateway));
  dns.toCharArray(myNetworkData.dns, sizeof(myNetworkData.dns));
  mask.toCharArray(myNetworkData.mask, sizeof(myNetworkData.mask));

  SaveNetworkDataSPIFFS();
  Serial.println("IP Address: " + String(myNetworkData.ip_address));
  Serial.println("HTTP Port: " + String(myNetworkData.http_port));
  Serial.println("Gateway: " + String(myNetworkData.gateway));
  Serial.println("DNS: " + String(myNetworkData.dns));
  Serial.println("Subnet Mask: " + String(myNetworkData.mask));

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<fieldset>");
  client.println("<center>");
  client.println("<h2>Settings saved successfully!</h2>");
  client.println("<a href=\"/\">Go back</a>");
  client.println("</fieldset>");
  client.println("</center>");
}


/*
  @brief Function to save struct value on SPIFFS.
  @param none.
  @return none.
*/
void SaveNetworkDataSPIFFS() 
{
  File file = SPIFFS.open("/network_data.json", "w");
  if (!file) 
  {
    Serial.println("Failed to open network_data.json file for writing");
    return;
  }
  StaticJsonDocument<512> doc;

  doc["ip_address"] = myNetworkData.ip_address;
  doc["http_port"] = myNetworkData.http_port;
  doc["gateway"] = myNetworkData.gateway;
  doc["dns"] = myNetworkData.dns;
  doc["mask"] = myNetworkData.mask;

  if (serializeJson(doc, file) == 0) { Serial.println("Failed to serialize JSON object to file"); }

  file.close();
  Serial.println("Data saved to network_data.json");
}


/*
  @brief Function to read network data from SPIFFS.
  @param none.
  @return none.
*/
void ReadNetworkDataFromSPIFFS()
{
  if (SPIFFS.exists("/network_data.json")) 
  {
    File file = SPIFFS.open("/network_data.json", "r");
    if (!file) { Serial.println("Failed to open network_data.json file"); }
    
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error) { Serial.println("Failed to parse the JSON file"); }

    strlcpy(myNetworkData.ip_address, doc["ip_address"], sizeof(myNetworkData.ip_address));
    strlcpy(myNetworkData.http_port, doc["http_port"], sizeof(myNetworkData.http_port));
    strlcpy(myNetworkData.gateway, doc["gateway"], sizeof(myNetworkData.gateway));
    strlcpy(myNetworkData.dns, doc["dns"], sizeof(myNetworkData.dns));
    strlcpy(myNetworkData.mask, doc["mask"], sizeof(myNetworkData.mask));

    file.close();

    Serial.println("-------- NETWORKDATA READ FROM SPIFFS ---------");
    Serial.println("ip_address: " + String(myNetworkData.ip_address));
    Serial.println("http_port: " + String(myNetworkData.http_port));
    Serial.println("gateway: " + String(myNetworkData.gateway));
    Serial.println("dns: " + String(myNetworkData.dns));
    Serial.println("mask: " + String(myNetworkData.mask));
    Serial.println("------------------------------------------------");
  }
  else
  {
    //TO-DO: create an .h file to handle the default network settings.
    Serial.println("Creating network_data.json file on SPIFFS");
    strncpy(myNetworkData.ip_address, defautlNetworkData.ipAddess, sizeof(myNetworkData.ip_address));
    strncpy(myNetworkData.http_port, defautlNetworkData.httpPort, sizeof(myNetworkData.http_port));
    strncpy(myNetworkData.gateway, defautlNetworkData.gateway, sizeof(myNetworkData.gateway));
    strncpy(myNetworkData.dns, defautlNetworkData.dsn, sizeof(myNetworkData.dns));
    strncpy(myNetworkData.mask, defautlNetworkData.mask, sizeof(myNetworkData.mask));

    SaveNetworkDataSPIFFS();
  }
}


/*
  @brief Function reset the device to factory settings.
  @param none.
  @return none.
*/
void ResetFactorySettings()
{
  strncpy(myNetworkData.ip_address, defautlNetworkData.ipAddess, sizeof(myNetworkData.ip_address));
  strncpy(myNetworkData.http_port, defautlNetworkData.httpPort, sizeof(myNetworkData.http_port));
  strncpy(myNetworkData.gateway, defautlNetworkData.gateway, sizeof(myNetworkData.gateway));
  strncpy(myNetworkData.dns, defautlNetworkData.dsn, sizeof(myNetworkData.dns));
  strncpy(myNetworkData.mask, defautlNetworkData.mask, sizeof(myNetworkData.mask));
  SaveNetworkDataSPIFFS();
  ESP.restart();
}