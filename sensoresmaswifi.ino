#include <NewPing.h> // Librería: https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
#include <ESP8266HTTPClient.h> //Librería para transformar la placa en cliente
#include <ESP8266WiFi.h> // Librería para manipular el chip WIFI

#define SONAR_NUM 2      // Número de sensores
#define MAX_DISTANCE 500 // Distancia máxima que medirá el sensor.

NewPing sonar[SONAR_NUM] = {   // Array para identificar a los sensores.
  NewPing(5, 16, MAX_DISTANCE), // trigger pin, echo pin, sensor 0.
  NewPing(0, 4, MAX_DISTANCE) // trigger pin, echo pin, sensor 0.
};

String plaza[SONAR_NUM] = { // Array que guarda las id de las plazas del estacionamiento
  String("1"), // ID de cada plaza
  String("2")  // ID de cada plaza
};

bool estado[SONAR_NUM]{ // Array que guarda el estado de las plazas.
  false, // False para estado inicial false = desocupada.
  false // False para estado inicial false = desocupada.
};

void setup() {
  Serial.begin(115200); // Abrimos el terminal serial a 115200 baudios
  conectarWifi(); // Llamamos al método conectarWIFI que está más abajo.
}

void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop para tomar las medidas que entrega cada sensor.
    delay(50); // Esperamos 50 ms para no interferir las ondas, mínimo 29 ms.
    Serial.print(i); // Muestra el id del array que ocupa cada sensor dentro del arreglo.
    Serial.print("="); // Imprime es = por el terminal Serial
    Serial.print(sonar[i].ping_cm()); // ping_cm() método de la librería NewPing que entrega medida en Centímetros.
    Serial.print("cm "); // Imprime cm por terminal Serial.

    //Preguntamos si Los estacionamientos están siendo ocupados
    // Preguntamos si la plaza con ID 1 está ocupada para cambiar su estado.
    if(sonar[i].ping_cm() <= 6 && plaza[i] == "1" && estado[i] == false){
      // Condición 1: que sea menor o igual a 6 cm.
      // Condicion 2: que la plaza sea la plaza 1 y
      // Condición 3: que la plaza esté desocupada para que pueda ser ocupada.
      //Si está ocupado, se envía POST al servidor para notificarlo.
      Serial.println("Estacionamiento 1 está ocupado"); // Imprime mensaje comunicación por monitor Serial.
      cambiarEstado(plaza[i],"true"); // Método que envía el post al servidor para cambiar el estado de la plaza.
      estado[i] = true; // Se cambia el estado de la plaza de forma local.
    }
    // Se consultan condiciones contrarias para identificar cuando la plaza de desocupa.
    if(sonar[i].ping_cm() > 6 && plaza[i] == "1" && estado[i] == true){
      cambiarEstado(plaza[i],"false");
      estado[i] = false;
      Serial.println("Estacionamiento 1 se desocupó.");
    }

    // Se repite el proceso para la plaza número 2.
    if(sonar[i].ping_cm() <= 6 && plaza[i] == "2" && estado[i] == false){
      //Si está ocupado, se envía POST al servidor para notificarlo.
      Serial.println("Estacionamiento 2 está ocupado");
      cambiarEstado(plaza[i],"true");
      estado[i] = true;
    }

    if(sonar[i].ping_cm() > 6 && plaza[i] == "2" && estado[i] == true){
      cambiarEstado(plaza[i],"false");
      estado[i] = false;
      Serial.println("Estacionamiento 2 se desocupó.");
    }
        
  }
  Serial.println(); // Salto de línea en monitor Serial.
  Serial.println("------"); // Delimitador en el monitor Serial.
  delay(3000); // Repite cada 3 segundos.
}

void conectarWifi(){
  WiFi.begin("spaceparking", "123123123");   // Credenciales para conectarse a una red wifi.

  while (WiFi.status() != WL_CONNECTED) {  // Esperando que el estatos de sea Conectado para continuar y salir del loop while.
    delay(500);
    Serial.println("Esperando establecer conexión a la red...");
  }
}

//Método que cambia el estado de la plaza en el servidor.
// Parámetro uno ID de la plaza, segundo parametro boolean true = ocupado, false = desocupado.
void cambiarEstado(String plaza_id,String ocupada){

if(WiFi.status()== WL_CONNECTED){   // Pregunta para saber si tenemos conexión a Wifi e Internet

  HTTPClient http;    // Declaración del objeto HTTPClient para enviar peticiones HTTP
    
  http.begin("http://192.168.0.100:8000/api/ocupacion"); // URL del endpoint
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Cabecera de la petición
  int httpCode = http.POST("ocupada="+ocupada+"&plaza_id="+plaza_id+"&nodemcu_id=1"); // Datos de la petición mediante url
  http.writeToStream(&Serial); // Envío de la petición.

  //int httpCode = http.POST(); // Obtenemos el código de respuesta desde el servidor.
  
  if(httpCode > 0){ // Si hay un código de respuesta mayor a 0
    String payload = http.getString(); // Obtenemos la respuesta en texto plano
    Serial.println("HTTP Code response: " + httpCode); // Mostramos por monitor Serial código http de respuesta
    Serial.println(payload); // Imprime el texto plano de la respuesta.
  }
   
  http.end();  // cierra la conexión con el servidor.

}else{
    Serial.println("Error al conectar con Wifi."); // Si no existe conexión, se muestra este error en monitor Serial.
}
 
}

