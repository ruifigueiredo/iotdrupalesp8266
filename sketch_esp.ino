/*
 *  Source switch um relay ligado à porta 5 (GPIO5) no ESP8266.
 *
 *  Vai ficar ligado ao MQTT e escuta o topico estado/porta/relay_state
 *  e os comandos 'on' e 'off'. De 60 em 60 segundos, vai publicar um estado actual no 
 *  estado/porta/relay_state
 *
 *  Rui Figueiredo
 *  rui.figueiredo@gmail.com
 *  
 *  Filipe Pereira
 *  fmfpereira@gmail.com 
 *
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int RelayPin = 4;    // RELAY ligado ao pin digital 5

const char* ssid     = "SSI NETWORK";
const char* password = "SSID PASSWORD";

//ip do MQTT Server que está no raspberry
const char* mqtt_server = "192.168.0.103";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print(" => ");

char* payload_str;
  payload_str = (char*) malloc(length + 1);
  memcpy(payload_str, payload, length);
  payload_str[length] = '\0';
Serial.println(String(payload_str));
  
  if ( String(topic) == "estado/porta/relay" ) {
    if (String(payload_str) == "1" ) {
      digitalWrite(RelayPin, HIGH);   // Liga o RELAY
      client.publish("estado/porta/relay_state","1");
    } else if ( String(payload_str) == "0" ) {
      digitalWrite(RelayPin, LOW);    // Desliga o RELAY
      client.publish("estado/porta/relay_state","0");
    } else {
      Serial.print("I do not know what to do with ");
      Serial.print(String(payload_str));
      Serial.print(" on topic ");
      Serial.println( String(topic));
    }
  }
}

void connect_to_MQTT() {
 client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (client.connect("estado_porta_relay")) {
    Serial.println("(re)-connected to MQTT");
    client.subscribe("estado/porta/relay");
  } else {
    Serial.println("Could not connect to MQTT");
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // A ligar ao WIFI
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  connect_to_MQTT();

  // Inicializar PIN 5, onde o relay está ligado.
  pinMode(RelayPin, OUTPUT);
}

int tellstate = 0;

void loop() {
  client.loop();

  if (! client.connected()) {
    Serial.println("Not connected to MQTT....");
    connect_to_MQTT();
    delay(5000);
  }

  // Verifica o estado do relay de 60 em 60 segundos
  if ( (millis() - tellstate) > 6000000 ) {
    if ( digitalRead(RelayPin) ) {
      client.publish("estado/porta/relay_state","1");
    } else {
      client.publish("estado/porta/relay_state","0");
    }
    tellstate = millis();
  }
}
