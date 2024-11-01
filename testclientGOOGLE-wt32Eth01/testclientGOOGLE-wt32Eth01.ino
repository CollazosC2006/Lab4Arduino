#include <ETH.h>
#include <WiFi.h>

#define ETH_ADDR        1
#define ETH_POWER_PIN   16
#define ETH_POWER_PIN_ALTERNATIVE 16 
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN 

// Variables para el estado de la conexión
static bool eth_connected = false;

// Callback para eventos de Ethernet utilizando arduino_event_id_t
void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("Ethernet iniciado");
      // Configurar el nombre del dispositivo en la red
      ETH.setHostname("ESP32-ETH-Device");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet conectado");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("Conexión establecida con DHCP");
      Serial.println("---- Detalles de la conexión ----");

      // Mostrar la descripción de la interfaz de red
      Serial.printf("Interfaz de red: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
      
      // Imprimir dirección IP asignada
      Serial.print("Dirección IP: ");
      Serial.println(ETH.localIP());
      
      // Imprimir máscara de subred
      Serial.print("Máscara de subred: ");
      Serial.println(ETH.subnetMask());
      
      // Imprimir puerta de enlace (gateway)
      Serial.print("Puerta de enlace: ");
      Serial.println(ETH.gatewayIP());
      
      // Imprimir servidor DNS primario
      Serial.print("DNS Primario: ");
      Serial.println(ETH.dnsIP(0));
      
      // Imprimir servidor DNS secundario
      Serial.print("DNS Secundario: ");
      Serial.println(ETH.dnsIP(1));
      
      Serial.println("-------------------------------");
      
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("Se perdió la dirección IP");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet desconectado");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("Ethernet detenido");
      eth_connected = false;
      break;
    default:
      break;
  }
}

// Función para probar la conectividad con un servidor
void testClient(const char *host, uint16_t port) {
  Serial.print("\nConectando a ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("Error al conectar");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("Conexión cerrada\n");
  client.stop();
}

void setup() {
  // Iniciar la comunicación serial
  Serial.begin(115200);
  
  // Registrar el evento de WiFi para manejar eventos de Ethernet
  WiFi.onEvent(onEvent);

  // Iniciar la conexión Ethernet (DHCP)
  ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);
}

void loop() {
  // Si está conectado, prueba la conectividad con google.com
  if (eth_connected) {
    testClient("google.com", 80);
  }
  
  delay(10000); // Esperar 10 segundos antes de la siguiente prueba
}
