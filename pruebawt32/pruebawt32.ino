#include <ETH.h>   // Biblioteca para Ethernet en ESP32
#include <WiFi.h>  // Biblioteca para WiFi en ESP32
#include <HTTPClient.h>  // Biblioteca para hacer solicitudes HTTP


// Evento de red para monitorear cambios en la conexión
void WiFiEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("Ethernet iniciado");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet conectado");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("Dirección IP: ");
      Serial.println(ETH.localIP());
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet desconectado");
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Inicializar Ethernet
  ETH.begin();

  // Registrar el evento para el estado de Ethernet
  WiFi.onEvent(WiFiEvent);

  // Probar si se está recibiendo correctamente el mensaje en el serial
  Serial.println("Hola mundo! Desde WT32-ETH01");
}

void loop() {
  // Imprimir "Hola mundo" cada segundo
  Serial.println("Hola mundo!");
  /*
  // Si está conectado, prueba la conectividad con google.com
  if (eth_connected) {
    testClient("google.com", 80);
  }
  */

  HTTPClient http;
  
  Serial.println("Enviando solicitud a Google...");

  // Iniciar conexión al servidor de Google
  http.begin("http://www.google.com");

  // Realizar solicitud GET
  int httpCode = http.GET();

  // Verificar si la solicitud fue exitosa
  if (httpCode > 0) {
    Serial.println("Conexión exitosa con Google!");
  } else {
    Serial.print("Error en la conexión: ");
    Serial.println(httpCode);
  }

  // Finalizar la conexión
  http.end();

  delay(5000); // Esperar 10 segundos antes de la siguiente prueba

  
}
