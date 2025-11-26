//Incluir bibliotecas
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <Wire.h>

// --- Definições do Sensor ---
const int DHT_PIN = 15; //terminal do sensor de temperatura e umidade

// --- Definições de Rede e MQTT ---
char SSIDName[] = "Wokwi-GUEST"; //nome da rede WiFi
char SSIDPass[] = ""; //senha da rede WiFI
char BrokerURL[] = "broker.hivemq.com"; //URL do broker MQTT
char BrokerUserName[] = ""; //nome do usuario para autenticar no broker MQTT
char BrokerPassword[] = ""; //senha para autenticar no broker MQTT
char MQTTClientName[] = "mqtt-projeto-iot-umidificador-producer"; //nome do cliente MQTT
int BrokerPort = 1883; //porta do broker MQTT

char Topico_01[] = "umidificador/automatico/dados/umidade"; //nome do topico 01

//Variaveis globais e objetos
WiFiClient espClient; //instancia o objeto espClient que pode se conectar a um endereço IP e porta de Internet especificados, conforme definido em clienteMQTT.connect()
PubSubClient clienteMQTT(espClient); //instancia o objeto clienteMQTT parcialmente inicializado cujo argumento eh o objeto para conectar na Internet
DHTesp dhtSensor; //instancia o objeto dhtSensor a partir da classa DHTesp

//Funcoes definidas pelo usuário

//funcao para reconectar ao broker
void mqttReconnect() {
  while (!clienteMQTT.connected()) {
    Serial.println("Conectando-se ao broker MQTT...");
    Serial.println(MQTTClientName);
    if (clienteMQTT.connect(MQTTClientName, BrokerUserName, BrokerPassword)) {
      Serial.print(MQTTClientName);
      Serial.println(" conectado!");
      clienteMQTT.subscribe(Topico_01);
    } else {
      Serial.print("failed, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(" tente novamente em 5 segundos.");
      delay(5000);
    }
  }
}

//Setup
void setup() {
  clienteMQTT.setServer(BrokerURL, BrokerPort); //configura os detalhes do broker MQTT passando URL e porta
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22); //inicializa o sensor de temperatura e umidade

  pinMode(LED_BUILTIN, OUTPUT); //configura o LED embutido da placa como saida

  Serial.begin(9600);
  Serial.print("Conectando-se ao Wi-Fi");
  WiFi.begin(SSIDName, SSIDPass); //inicializa Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { //repete enquanto nao estabelece conexao
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP()); //imprime o endereco IP

  delay(1000);
}

//Loop
void loop() {
  if (!clienteMQTT.connected()) { //verifica se o cliente esta conectado ao broker
    mqttReconnect(); //se o cliente nao estiver conectado, tenta reconectar
  }
  clienteMQTT.loop(); //funcao necessaria para manter a conexao com o broker MQTT ativa e coletar as mensagens recebidas

  TempAndHumidity temp_umid = dhtSensor.getTempAndHumidity(); //instancia o objeto temp_umid a partir da classe TempAndHumidity
  float umidade = temp_umid.humidity;

  // Verifica se a leitura foi bem sucedida antes de continuar
  if (isnan(umidade)) {
    Serial.println("Falha na leitura do sensor DHT!");
    delay(2000);
    return;
  }

  // --- Publicação no MQTT ---
  char _u[8];
  dtostrf(umidade, 1, 2, _u); //converte um float em string
  clienteMQTT.publish(Topico_01, _u); //publica um topico no broker
  Serial.print("Umidade publicada: ");
  Serial.println(_u);

  delay(60000);
}