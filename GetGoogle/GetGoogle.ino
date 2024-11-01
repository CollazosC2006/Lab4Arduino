#include <ETH.h>   // Biblioteca para Ethernet en ESP32
#include <WiFi.h>  // Biblioteca para WiFi en ESP32
#include <HTTPClient.h>  // Biblioteca para hacer solicitudes HTTP

#define ETH_ADDR        1
#define ETH_POWER_PIN   16
#define ETH_POWER_PIN_ALTERNATIVE 16 
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN 

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
  ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);

  // Registrar el evento para el estado de Ethernet
  WiFi.onEvent(WiFiEvent);

  // Probar si se está recibiendo correctamente el mensaje en el serial
  Serial.println("Hola mundo! Desde WT32-ETH01");
}

void loop() {
  // Enviar una solicitud HTTP cada 3 segundos
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 3000) {  // Cada 3 segundos
    lastMillis = millis();
    sendPing();
  }

  // Imprimir "Hola mundo" cada segundo
  Serial.println("Hola mundo!");
  delay(1000);
}

// Función para enviar un ping (GET request) al servidor de Google
void sendPing() {
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
}
