#include <WiFi.h>
#include <PubSubClient.h>

// --- Definições do Sensor ---
const int PIN_SAIDA = 15;

// --- Definições de Rede e MQTT ---
char SSIDName[] = "Wokwi-GUEST"; 
char SSIDPass[] = ""; 

char BrokerURL[] = "broker.hivemq.com"; 
char BrokerUserName[] = ""; 
char BrokerPassword[] = ""; 
char MQTTClientName[] = "mqtt-projeto-iot-umidificador-consumer-controller"; 
int BrokerPort = 1883; 

char Topico_01[] = "umidificador/automatico/dados/estaLigado"; 

//Variaveis globais e objetos
WiFiClient espClient; 
PubSubClient clienteMQTT(espClient); 

// --- Função de Callback (Recepção) ---
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim(); 

  Serial.print("Mensagem recebida: [");
  Serial.print(msg);
  Serial.println("]");

  if (msg.equalsIgnoreCase("true")) { 
    digitalWrite(PIN_SAIDA, HIGH);
    Serial.println("--> LIGADO");
  } else {
    digitalWrite(PIN_SAIDA, LOW);
    Serial.println("--> DESLIGADO");
  }
}

// --- Função de Reconexão MQTT ---
void mqttReconnect() {
  // Loop até reconectar
  while (!clienteMQTT.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (clienteMQTT.connect(MQTTClientName, BrokerUserName, BrokerPassword)) {
      Serial.println("Conectado!");
      clienteMQTT.subscribe(Topico_01);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(" tentando em 5s");
      delay(5000);
    }
  }
}

// --- Função para conectar ao Wi-Fi ---
void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se ao wifi");

  WiFi.begin(SSIDName, SSIDPass);
  while (WiFi.status() != WL_CONNECTED) { //repete enquanto nao estabelece conexao
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP()); //imprime o endereco IP
}

// --- Setup ---
void setup() {
  pinMode(PIN_SAIDA, OUTPUT);
  digitalWrite(PIN_SAIDA, LOW);

  // 1. Inicia Serial mais rápido
  Serial.begin(115200); 
  
  // 2. Dá tempo para você ver o que está acontecendo
  delay(2000); 
  Serial.println("--- INICIANDO ESP32 ---");

  setupWifi(); // Chama a função de Wi-Fi separada

  clienteMQTT.setServer(BrokerURL, BrokerPort);
  clienteMQTT.setCallback(mqtt_callback);
}

// --- Loop ---
void loop() {
  // 3. Garante que o Wi-Fi ainda está conectado antes de tentar MQTT
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }

  if (!clienteMQTT.connected()) {
    mqttReconnect();
  }
  clienteMQTT.loop();

  delay(10); // Pequeno delay para estabilidade
}