// Agregamos las bibliotecas
#include <WiFi.h> // conectar a wifi
#include <PubSubClient.h> // Publicar y suscribir en mqtt
#include <DHT.h> // configuracion del DHT

#define DHTPIN 14 // Definimos pin del Sensor
#define DHTTYPE DHT11 // Definimos el modelo del Sensor
DHT dht(DHTPIN, DHTTYPE); // Configuramos el sensor con los datos de las dos lineas anteriores y le damos el nombre "dht"

// Escribimos nuestras credenciales de red
const char* ssid = "ElNombreDeTuRed";
const char* password = "Clave";

// Escribimos la direccion del broker que vamos a usar
// En este caso una direccion publica
const char* mqtt_server = "test.mosquitto.org";

// Definimos el pin del LED
const int LED = 2;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
String Estado;


void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Conectado a  ");
  Serial.println(ssid);

// La ESP32 se conecta a wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Estado += (char)payload[i];
  }
  Serial.println();

  Serial.print("Payload String: ");
  Serial.println(Estado);
  
  if (Estado == "Off") {
    digitalWrite(LED, LOW);
    Estado = "";

  } if (Estado == "On") {
    digitalWrite(LED, HIGH);
    Estado = "";

  }
}

// Bloque para la reconeccion a MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conexion MQTT perdida...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      client.subscribe("esp32/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT); // Pin Led como Salida
  digitalWrite(LED, HIGH); // El led inicia encendido
  dht.begin(); // Iniciamos el DHT
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Se conecta al servidor MQTT
  client.setCallback(callback); // Llamamos la funcion Callback
  client.subscribe("esp32/led"); // Se suscribe al topic "esp32/led"
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

// Variables para leer los datos de temperatura y humedad
 float t = dht.readTemperature(); // Lee la temperatura
 float h = dht.readHumidity(); // Lee la humedad
 char temp[8]; // Variable auxiliar para guardar la temperatura
 dtostrf(t, 6, 2, temp);
 char hum[8]; // Variable auxiliar para guardar la humedad
 dtostrf(h, 6, 2, hum);


  Serial.print("Temperatura: ");
  Serial.println(t); 
  Serial.print("Humedad: ");
  Serial.println(h); 
   delay(5000);
   
   if (!client.connected()) {
     reconnect();
   }   
   if (client.connected()){
     client.publish("esp32/temp", temp); // Publica el valor de la temperatura en el topic "esp32/temp"
     client.publish("esp32/hum", hum); // Publica el valor de la humedad en el topic "esp32/hum"
     Serial.println("se envio los datos al broker ");    
   }
  client.loop();
}
 
