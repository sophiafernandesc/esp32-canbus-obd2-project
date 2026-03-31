/*
 * CÓDIGO UNIFICADO PARA ESP32 WROOM
 * Funcionalidades:s
 * - Leitura de 4 potenciômetros (pressão de pneus)
 * - Leitura de 1 potenciômetro (nível de combustível)
 * - Publicação de dados via MQTT
 */

// --- BIBLIOTECAS ---
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>      // Necessário para o MCP2515
#include <mcp2515.h>  // Biblioteca do MCP2515

// --- CONFIGURAÇÕES DE REDE E MQTT ---
#define ID_MQTT "Redes_Veiculares_PUC_sophiaMQTT"
#define BROKER_MQTT "broker.emqx.io"
#define BROKER_PORT 1883

// **ATENÇÃO: as configurações do wifi devem ser modificadas de acordo com a rede que você utilizará
const char* SSID = "vovozona2";           // <-- SSID Wi-Fi
const char* PASSWORD = "vovozona2rocks";  // <-- senha Wi-Fi

// --- PINOS DOS POTENCIÔMETROS (GPIOs ESP32) ---
#define POT_D_ESQ 34        // GPIO 34 (ADC1_CH6)
#define POT_D_DIR 35        // GPIO 35 (ADC1_CH7)
#define POT_T_ESQ 32        // GPIO 32 (ADC1_CH4)
#define POT_T_DIR 33        // GPIO 33 (ADC1_CH5)
#define POT_COMBUSTIVEL 39  // GPIO 39 (ADC1_CH3)

// --- CONFIGURAÇÕES DE SENSORES ---
#define PRESSAO_MIN 28.0        // PSI
#define PRESSAO_MAX 36.0        // PSI
#define TANQUE_CAPACIDADE 47.0  // Litros
float alpha = 0.2;              // Fator de suavização para leitura dos potenciômetros

// --- VARIÁVEIS DE ESTADO E LEITURA ---
WiFiClient espClient;
PubSubClient MQTT(espClient);

float valorFiltradoDEsq = 0;
float valorFiltradoDDir = 0;
float valorFiltradoTEsq = 0;
float valorFiltradoTDir = 0;

unsigned long lastSensorReadTime = 0;
const unsigned long sensorReadInterval = 3000;  // Intervalo para leitura de sensores e MQTT (3 segundos)

// --- PROTÓTIPOS DE FUNÇÕES ---
void initWiFi();
void initMQTT();
void reconnectMQTT();
void VerificaConexoesWiFIEMQTT();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void publicarPressao(const char* topico, float valor);
void publicarNivelCombustivel(float litros, float percentual);
void initCAN();
void handleCANMessages();

// --- FUNÇÃO SETUP ---
void setup() {
  Serial.begin(115200);  // Baud rate mais rápido para o Serial Monitor
  delay(100);
  Serial.println("\n--- INICIANDO SISTEMA ESP32 ---");

  // Inicializa Wi-Fi
  Serial.println("Iniciando conexão Wi-Fi...");
  initWiFi();

  // Inicializa MQTT
  Serial.println("Configurando MQTT...");
  initMQTT();

  // Inicializa CAN
  Serial.println("Configurando CAN (MCP2515)...");
  initCAN();

  Serial.println("--- PRONTO ---");
}

// --- FUNÇÃO LOOP PRINCIPAL ---
void loop() {
  // Garante que as conexões Wi-Fi e MQTT estão ativas
  VerificaConexoesWiFIEMQTT();
  MQTT.loop();  // Processa mensagens MQTT pendentes

  unsigned long currentMillis = millis();

  // Lógica de leitura de sensores e publicação MQTT
  if (currentMillis - lastSensorReadTime >= sensorReadInterval) {
    lastSensorReadTime = currentMillis;

    // Leitura e filtragem dos potenciômetros dos pneus
    valorFiltradoDEsq = alpha * analogRead(POT_D_ESQ) + (1 - alpha) * valorFiltradoDEsq;
    valorFiltradoDDir = alpha * analogRead(POT_D_DIR) + (1 - alpha) * valorFiltradoDDir;
    valorFiltradoTEsq = alpha * analogRead(POT_T_ESQ) + (1 - alpha) * valorFiltradoTEsq;
    valorFiltradoTDir = alpha * analogRead(POT_T_DIR) + (1 - alpha) * valorFiltradoTDir;

    // Conversão para PSI
    float pressaoDEsq = PRESSAO_MIN + (valorFiltradoDEsq / 4095.0) * (PRESSAO_MAX - PRESSAO_MIN);
    float pressaoDDir = PRESSAO_MIN + (valorFiltradoDDir / 4095.0) * (PRESSAO_MAX - PRESSAO_MIN);
    float pressaoTEsq = PRESSAO_MIN + (valorFiltradoTEsq / 4095.0) * (PRESSAO_MAX - PRESSAO_MIN);
    float pressaoTDir = PRESSAO_MIN + (valorFiltradoTDir / 4095.0) * (PRESSAO_MAX - PRESSAO_MIN);

    // Publica as pressões dos pneus via MQTT
    publicarPressao("veiculo/pneu/pressao/dianteiro/esq", pressaoDEsq);
    publicarPressao("veiculo/pneu/pressao/dianteiro/dir", pressaoDDir);
    publicarPressao("veiculo/pneu/pressao/traseiro/esq", pressaoTEsq);
    publicarPressao("veiculo/pneu/pressao/traseiro/dir", pressaoTDir);

    // Leitura do potenciômetro de combustível
    int leituraCombustivel = analogRead(POT_COMBUSTIVEL);
    float percentualCombustivel = (leituraCombustivel / 4095.0) * 100.0;
    float litrosCombustivel = (percentualCombustivel / 100.0) * TANQUE_CAPACIDADE;

    // Publica o nível de combustível via MQTT
    publicarNivelCombustivel(litrosCombustivel, percentualCombustivel);
  }
}

// --- IMPLEMENTAÇÃO DAS FUNÇÕES ---

// Conexão Wi-Fi
void initWiFi() {
  WiFi.mode(WIFI_STA);  // Define o ESP32 para o modo Estação (cliente)
  WiFi.begin(SSID, PASSWORD);

  int tentativas = 0;
  const int MAX_TENTATIVAS_CONEXAO = 30;  // 30 segundos de timeout

  while (WiFi.status() != WL_CONNECTED && tentativas < MAX_TENTATIVAS_CONEXAO) {
    delay(1000);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Força do Sinal (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\nFalha ao conectar ao WiFi!");
    Serial.println("Verifique SSID, senha ou rede 2.4 GHz.");
    Serial.print("Último status do Wi-Fi: ");
    Serial.println(WiFi.status());
  }
}

// Callback MQTT (para mensagens recebidas)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem MQTT recebida no tópico: ");
  Serial.print(topic);
  Serial.print(". Conteúdo: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Inicialização MQTT
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

// Reconexão MQTT
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar ao broker MQTT...");
    // Tenta conectar com o ID do cliente
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado!");
      // Opcional: Se houver tópicos para se inscrever, faça-o aqui
      // MQTT.subscribe("topico/para/receber");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(MQTT.state());  // Código de erro MQTT
      Serial.println(". Tentando novamente em 5s");
      delay(5000);  // Espera 5 segundos antes de tentar novamente
    }
  }
}

// Verifica e reconecta Wi-Fi e MQTT
void VerificaConexoesWiFIEMQTT() {
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    WiFi.disconnect();  // Desconecta para tentar novamente do zero
    initWiFi();         // Tenta restabelecer a conexão Wi-Fi
  }
}

// Publica valores de pressão
void publicarPressao(const char* topico, float valor) {
  char msg[20];
  dtostrf(valor, 5, 2, msg);  // Formata float em string (5 largura total, 2 casas decimais)
  if (MQTT.publish(topico, msg)) {
  } else {
    Serial.print("Falha ao publicar ");
    Serial.println(topico);
  }
}

// Publica nível de combustível
void publicarNivelCombustivel(float litros, float percentual) {
  char msgLitros[20];
  char msgPercent[20];
  dtostrf(litros, 5, 2, msgLitros);
  dtostrf(percentual, 5, 2, msgPercent);
  if (MQTT.publish("veiculo/combustivel/litros", msgLitros) && MQTT.publish("veiculo/combustivel/percentual", msgPercent)) {
  } else {
    Serial.println("Falha ao publicar combustível.");
  }
}
}