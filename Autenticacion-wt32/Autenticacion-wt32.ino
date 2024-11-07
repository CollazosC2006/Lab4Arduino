#include <Adafruit_Fingerprint.h>
#include <ETH.h>   // Biblioteca para Ethernet en ESP32
#include <WiFi.h>  // Biblioteca para WiFi en ESP32
#include <HTTPClient.h>  // Biblioteca para hacer solicitudes HTTP
#include <ArduinoJson.h> // Biblioteca para manejar JSON

#include <Base64.h>  // Biblioteca para codificar en Base64



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
// Configuramos Serial2 para el sensor dactilar y Serial0 para la comunicación con la PC
HardwareSerial mySerial(2); // Serial2 en ESP32 (pines IO17 para TX y IO16 para RX)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id=1;
uint8_t fingerTemplate[512]; // El template real

void setup()
{
  // Comunicación con la PC
  Serial.begin(9600); // Serial0 para la comunicación por USB con el PC
  while (!Serial);  
  delay(100);

  ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE);

  // Registrar el evento para el estado de Ethernet
  WiFi.onEvent(WiFiEvent);
  

  // Configuración del puerto serial para el sensor dactilar
  mySerial.begin(57600, SERIAL_8N1, 5, 17); // RX = IO16, TX = IO17
  //Aseguramos la conexion del sensor  
  while (!finger.verifyPassword()) { 
    Serial.println("Did not find fingerprint sensor :("); 
    delay(500); 
  }
  
  finger.getParameters();

}



void loop()                     
{
  
  while (! getFingerprintEnroll() );

  if(downloadFingerprintTemplate(id)){
    //send fingerprint
    sendFingerprintTemplate(fingerTemplate);
  }



  delay(1000);


}

String fingerprintToHexString(uint8_t *data, size_t length) {
  String hexString = "";
  for (size_t i = 0; i < length; i++) {
    if (data[i] < 16) hexString += "0";
    hexString += String(data[i], HEX);
  }
  return hexString;
}


// Función para enviar el template de la huella al servidor
void sendFingerprintTemplate(uint8_t* fingerprintData) {
  if (ETH.linkUp()) {
    HTTPClient http;
    //String serverUrl = "http://192.168.10.14/api/fingerprint";  // Reemplaza con la URL de tu servidor
    
    
    Serial.println("Convirtiendo template de huella a cadena hexadecimal...");

    //String fingerprintHex = "020036BA8BC3130084E010E0AC85175B120526861231F14941CC80F0523088FD1914712C9E0B0518CC08F0D145C3FB20E051410A24C9148243468B0517D1B312514654B21BE4515138AD191447500D2E451F95476D514795AC641851E57A7C0814888E8BCB330084E411E41BB617DC138524861231F14841CC88F851EC88FD1933F1EF01FFFFFFFF02008214642C9D8A0518CC08F0B145C3FB20E051410A24C9148243C70B051791D3327146348D9CD051D92C86F914554EAB46C51214034B9147F551DB5451E56B186014735E9F02051F3F8E0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001D41EF01FFFFFFFF02008200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000084EF01FFFFFFFF02008200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    String fingerprintHex = fingerprintToHexString(fingerTemplate, 512);

    // Preparar el payload JSON
    String payload = "fingerprint=" + fingerprintHex ;

    //http.begin(serverUrl);
    http.begin("http://192.168.128.32:8000/verificar_credenciales/");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    Serial.println("Enviando solicitud POST al servidor...");


    int httpCode = http.POST(payload);


    if (httpCode > 0) {
      Serial.printf("Código de respuesta HTTP: %d\n", httpCode);
      String response = http.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      bool isValid = doc["valid"];  // Obtener el valor de "valid" en la respuesta JSON

      // Realizar acción basada en la respuesta booleana
      if (isValid) {
        Serial.println("La huella es válida.");
        finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
        delay(2000);
        // Acción cuando la huella es válida
        // Por ejemplo: abrir una puerta, encender un LED, etc.
      } else {
        Serial.println("La huella es inválida.");
        finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
        delay(2000);

        
        // Acción cuando la huella es inválida
        // Por ejemplo: emitir una alerta, mantener la puerta cerrada, etc.
      }
    } else {
      Serial.printf("Error al enviar POST: %s\n", http.errorToString(httpCode).c_str());
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
    }

    http.end();

    

    
    /*Serial.print("Dirección IP: ");
    Serial.println(ETH.localIP());
    // URL del servidor FastAPI, actualiza la IP si es necesario
    http.begin("http://192.168.10.14:8000/verificar_credenciales/");

    // Cabecera para enviar la solicitud como formulario
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Datos que se van a enviar en la solicitud POST
    String postData = "Nombre=Carlos&clave=clave123";

    // Enviar la solicitud POST
    int httpResponseCode = http.POST(postData);

    // Verificar si se recibió una respuesta válida
    if (httpResponseCode > 0) {
      String response = http.getString();  // Obtener la respuesta
      Serial.println("Respuesta del servidor:");
      Serial.println(response);            // Imprimir la respuesta en el Serial
    } else {
      Serial.print("Error en la solicitud POST. Código: ");
      Serial.println(httpResponseCode);
    }

    // Finalizar la conexión
    http.end();*/
  } else {

    Serial.println("Ethernet no está conectado.");
    
  }
}


bool downloadFingerprintTemplate(uint16_t id)
{
  Serial.println("------------------------------------");
  Serial.print("Attempting to load #"); Serial.println(id);
  uint8_t p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" loaded");
      break;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      return false;
  }

  Serial.print("Attempting to get #"); Serial.println(id);
  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" transferring:");
      break;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      return false;
  }

  uint8_t bytesReceived[534]; // 2 paquetes de datos
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
    if (mySerial.available()) {
      bytesReceived[i++] = mySerial.read();
    }
  }
  Serial.print(i); Serial.println(" bytes read.");
  Serial.println("Decoding packet...");

  
  memset(fingerTemplate, 0xff, 512);

  // Filtrando solo los paquetes de datos
  int uindx = 9, index = 0;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);   // Primeros 256 bytes
  uindx += 256;
  uindx += 2;    // Saltar el checksum
  uindx += 9;    // Saltar el siguiente header
  index += 256;
  memcpy(fingerTemplate + index, bytesReceived + uindx, 256);   // Siguientes 256 bytes





  Serial.println("\ndone.");

  return true;
}

/*
void printHex(uint8_t num) {
  if (num < 0x10) { // Si el valor es menor a 16, agregar un 0 al frente
    Serial.print("0");
  }
  Serial.print(num, HEX); // Imprimir en formato hexadecimal
}*/

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    default:
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Unknown error");
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      return p;
  }

  Serial.println("Remove finger");
  delay(10);
  
  
  
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);
      break;
    default:
      Serial.println("Unknown error");
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      break;
    }
  }

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Unknown error");
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      delay(2000);
      return p;
  }

  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else {
    Serial.println("Unknown error");
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    delay(2000);
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  }else {
    Serial.println("Unknown error");
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    delay(2000);
    return p;
  }

  return true;
}
