#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

// Configuración de la red WiFi
const char *ssid = "GONZALES";
const char *password = "123felipao";

// Configuración del servidor MQTT
const char *mqtt_server = "34.23.25.139";
const char *mqtt_topic_control = "brazo/control";
const char *mqtt_topic_status = "brazo/status";

#define SERVO_PIN 2
#define SERIAL_BAUD 9600

Servo myServo;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);

  Serial.println("\nConectando a la red WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println("\nConectado a la red WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  myServo.attach(SERVO_PIN);
  setupMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Verifica comandos desde el puerto serial
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == '1') {
      abrirBrazo();
    } else if (command == '0') {
      cerrarBrazo();
    }
  }
}

void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado al servidor MQTT");
      // Suscribirse al topic de control
      client.subscribe(mqtt_topic_control);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentar de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void reconnect() {
  setupMQTT();
}

void abrirBrazo() {
  myServo.write(90);
  delay(1000);
  Serial.println("Brazo abierto");
  // Publicar mensaje en el topic de estado
  client.publish(mqtt_topic_status, "Brazo abierto");
}

void cerrarBrazo() {
  myServo.write(0);
  delay(1000);
  Serial.println("Brazo cerrado");
  // Publicar mensaje en el topic de estado
  client.publish(mqtt_topic_status, "Brazo cerrado");
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensaje recibido en el topic [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  // Verificar el contenido del mensaje y realizar la acción correspondiente
  if (strcmp(topic, mqtt_topic_control) == 0) {
    char command = (char)payload[0];
    if (command == '1') {
      abrirBrazo();
    } else if (command == '0') {
      cerrarBrazo();
    }
  }
}
