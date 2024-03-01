#include <HTTPClient.h>
#include <WiFi.h>

const char* ssid = "AniketKhandelwal"; // your network SSID (name of wifi network)
const char* password = "k8nrz6it";           // your network password
const int pirPin = 2;                         // PIR sensor output connected to 
const int flowDigitalPin = 34; 
const char* host = "20.70.178.193";

unsigned long motionDetectedTime = 0;
unsigned long waterFlowStartTime = 0;
// unsigned long sendMessageTime = 0;
bool motionDetected = false;
bool waterFlowDetected = false;
bool messageSent = false; // Flag to track if message has been sent

const int timeToDetectWaterFlow = 8000;// check after 8 sec 

void sendMessage(WiFiClient& client);
void checkWaterFlow(WiFiClient& client);

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(flowDigitalPin, INPUT);
  delay(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client;

  // const int httpPort = 80;

  if (digitalRead(2) == HIGH) {
    motionDetected = true;
    motionDetectedTime = millis();
    Serial.println("Motion detected");
  } else {
    motionDetected = false;
  }

  // Check water flow and send message if detected for 15 seconds continuously
  checkWaterFlow(client);

  delay(1000);
}

void checkWaterFlow(WiFiClient& client) {
  if (digitalRead(4) == HIGH) {
    waterFlowDetected = true;
    if (waterFlowStartTime == 0) {
      waterFlowStartTime = millis();
      Serial.println("Water flow detected"); // Print message when water flow is detected
    }
  } else {
    waterFlowDetected = false;
    waterFlowStartTime = 0;
    messageSent = false; // Reset messageSent flag when water flow stops
  }

  // Send message if water flow is detected for 15 seconds continuously
  if (!motionDetected && waterFlowDetected && (millis() - waterFlowStartTime >= timeToDetectWaterFlow) && !messageSent) {
    sendMessage(client);
    messageSent = true; // Set messageSent flag to true after sending message
  }
}

void sendMessage(WiFiClient& client) {
  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/api/NERDWHATS_AMERICA/send-message";
  String message = "{\"phone\": \"916377711389\",\"isGroup\": false,\"isNewsletter\": false,\"message\": \"There is unusual flow of water is detected from tap , go and close it\"}";

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Accept: */*\r\n" +
               "Authorization: Bearer $2b$10$dFl7dycN9Mz5SGcSgoe1dOeZG8EtB_wo1Jh77kOcY_qN6XBCOC_i6\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + message.length() + "\r\n\r\n" +
               message);
  unsigned long timeout = millis();
  while (!client.available()) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}
