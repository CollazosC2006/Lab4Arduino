#include <WiFi.h>
#include <HTTPClient.h>

// Configuración de la red WiFi
const char* ssid = "Tu_SSID";          // Reemplaza con el nombre de tu red WiFi
const char* password = "Tu_Contraseña"; // Reemplaza con la contraseña de tu red WiFi

// URL del servidor FastAPI
const char* serverName = "http://<IP_DEL_SERVIDOR>:8000/verificar_credenciales/"; // Reemplaza con la IP del servidor y puerto

void setup() {
  Serial.begin(115200);
  
  // Conexión a la red WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado a la red WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  
  // Enviar solicitud POST al servidor
  sendPostRequest("Carlos", "clave123");
}

void loop() {
  // Puedes llamar a sendPostRequest dentro de loop si necesitas enviar varias solicitudes
  // sendPostRequest("Carlos", "clave123");
  delay(10000); // Esperar 10 segundos entre solicitudes
}

void sendPostRequest(const String& nombre, const String& clave) {
  if(WiFi.status() == WL_CONNECTED) {  // Verifica que haya conexión WiFi
    HTTPClient http;
    
    // Iniciar conexión con el servidor
    http.begin(serverName);
    
    // Especificar tipo de contenido
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Datos de la solicitud POST
    String postData = "Nombre=" + nombre + "&clave=" + clave;
    
    // Enviar solicitud POST
    int httpResponseCode = http.POST(postData);
    
    // Si la respuesta del servidor es mayor a 0, significa que fue exitosa
    if(httpResponseCode > 0) {
      String response = http.getString(); // Obtener la respuesta del servidor
      Serial.println("Respuesta del servidor: ");
      Serial.println(response);
    } else {
      Serial.print("Error en la solicitud POST. Código de respuesta: ");
      Serial.println(httpResponseCode);
    }
    
    // Finalizar conexión
    http.end();
  } else {
    Serial.println("No hay conexión a WiFi");
  }
}

