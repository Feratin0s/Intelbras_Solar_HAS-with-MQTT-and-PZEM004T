#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>  //Armazenar as variaveis totais
//#include <ESP8266HTTPClient.h>  //Acesso ao site de monitoramento
#include <ArduinoOTA.h>  //Atualiza (Over the air)

//Versão --> Importante pra saber qual é a versão que eu usei
//Version = 2.3; By Arthur Ferraz
//date = "03/10/2025";

const char* mqtt_topic_solarpanel = "casa/medidor/solarpanel";
const char* mqtt_topic_solarpanel_total = "casa/medidor/solarpanel_total";

PZEM004Tv30 pzem(4, 5);

const char* ssid = "PAINELSOLAR";
const char* password = "password***";
const char* mqtt_server = "192.168.1.1";
//----

//Configurações do site de monitaramento
//const char* serverURL = "http://solar-monitoramento.intelbras.com.br/";
//String cookie = "";  // Para armazenar o cookie se necessário
//const char* headerkeys[] = { "Set-Cookie" };
//size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
//Credenciais do site de monitoramento
//String account = "Arti";
//String password_solar = "password";
//Tempo que faz a verificação do site
//unsigned long TempoRotativo_site = 30000;  //--> Aumentar posteriormente para 2 minutos
//unsigned long TempoAnterior_site = 0;

WiFiClient espClient;
PubSubClient client(espClient);

//Endereços para salvar floats na EEPROM
//int AdressFloat1 = 18;  // --> Medidor Grid --> Precisa começar apartir do 10 nas placas Wemos D1
//int AdressFloat2 = 22;  // --> Medidor Solar

//Variavel para armazenar o valor do painel solar
float solarPanelValue = 0.0;  // Watts
//
//struct MyStruct {
//  float ValorSolar;
//  unsigned int ID;  // --> ID do valor, pra corrigir o erro do fim de tarde e inicio de manhã
//};

//MyStruct Solar2[] = { 0.0, 0 };
//MyStruct Solar1[] = { 0.0, 0 };  //Zera o medidor do Kwh total --> Assim, o código começa zerado pra calcular energia injetada

//Variavel para armazenar o consumo da casa em EEPROM
//float energy = 0.0;   // Consumo total
//float energy1 = 0.0;  // Zera o medidor de Kwh para medir o intervalo

//const long intervalUpdateGrid = 600000;  //Intervalo que atualiza o valor do consumo geral --> 10 minutos
//unsigned long TempoRotativo = 0;
//unsigned long TempoAnterior = 0;


//Valor acumulativo de Energia Injetada e Energia Consumida
//float MedidorInjetada = 0.0;
//float MedidorGrid = 0.0;

//Variaveis do millis para rodar o código
unsigned long previousMillis = 0;
const long interval = 20000;  //Está em ms, logo, 1000 = 1s
int Delay = 5;                //Demora em ms entre o envio de mensagens delay(Delay);

//Variaveis para rodar a primeira vez o código
//int Y = 0;
//int X = 0;

//Função para salvar o valor na EEPROM a cada 24hrs
//int Tempo = 0;

//bool SendsMqtt = false;  //Just sends MQTT if the difference is greater than 1
//bool ResetSolar = false;

void setup() {
  Serial.begin(115200);
  //EEPROM.begin(512);

  // Configuração Wi-Fi
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());

  //Configuração OTA
  // Configuração básica do OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    // Exemplo: não esqueça de verificar se há espaço suficiente
    Serial.println("Iniciando atualização: " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nAtualização concluída!");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Falha na autenticação");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Erro ao iniciar");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Erro de conexão");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Erro ao receber dados");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Erro ao finalizar");
    }
  });

  // Definir a senha para OTA
  ArduinoOTA.setPassword("SofiaSettMiau");

  // Iniciar o OTA
  ArduinoOTA.begin();
  Serial.println("OTA iniciado");

  // Configuração MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Iniciando...");
  //Ler os valores acumulativos salvos anteriormente
//  Serial.print("Valor do MedidorGrid obtido --> ");
//  EEPROM.get(AdressFloat1, MedidorGrid);
//  Serial.println(MedidorGrid);
//  Serial.print("Valor do MedidorInjetada obtido --> ");
//  EEPROM.get(AdressFloat2, MedidorInjetada);
//  Serial.println(MedidorInjetada);
  delay(10);

  Serial.println("----------------------------->> Código atualizado com sucesso! <<-----------------------------");

}  //--> Pular isso caso queira adicionar o monitoramento abaixo

// ----------------

//Tentativa de login no site de monitoramento Intelbras
//  if (loginToSolarAPI()) {
//    Serial.println("Login realizado com sucesso!");
//
//  } else {
//    Serial.println("Falha no login.");
//  }
//}

//bool loginToSolarAPI() {
//  if (WiFi.status() == WL_CONNECTED) {
//
//    WiFiClient client;  // WiFiClient agora é necessário para as requisições HTTP
//    HTTPClient http;
//
// Iniciar a conexão com o cliente WiFi
//    http.begin(client, serverURL + String("login"));  // Agora passando o WiFiClient
//
// Define os parâmetros de login
//    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//    String postData = "account=" + account + "&password=" + password_solar + "&validateCode=&lang=en";

// Envia o pedido de login
//    int httpResponseCode = http.POST(postData);

//   if (httpResponseCode > 0) {
//      String response = http.getString();
//      Serial.println("Resposta do servidor: " + response);

// Aqui você pode capturar o cookie se necessário

//      http.collectHeaders(headerkeys, headerkeyssize);
//      for (int i = 0; i < http.headers(); i++) {
//        Serial.printf("%s = %s\r\n", http.headerName(i).c_str(), http.header(i).c_str());
//      }
//      cookie = http.header("Set-Cookie");  // Armazena o cookie
//      Serial.printf("Cookie capturado: %s\r\n", cookie.c_str());

//getPlantList();
// Verifica se o login foi bem-sucedido
//      if (response.indexOf("\"result\":1") >= 0) {
//        return true;
//      }
//    } else {
//      Serial.println("Erro ao fazer o login: " + String(httpResponseCode));
//    }

//    http.end();
//  }
//  return false;
//}

// -------------------------------------

void loop() {

  ArduinoOTA.handle();  // Verifica se há uma nova atualização OTA

  // Obtendo o tempo atual em milissegundos
  unsigned long currentMillis = millis();

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  delay(10);

  if (currentMillis - previousMillis >= interval) {

    // Salvando o último tempo em que enviou as informações
    previousMillis = currentMillis;

    // Leitura dos valores
    float voltage = readVoltage();
    float current = readCurrent();
    float power = readPower();
    float energy = readEnergy();  // Consumo da Casa no geral --> fora do escopo de void loop()
    float frequency = readFrequency();
    float pf = readPowerFactor();
    float InjectedEnergy = readEnergyInjected();  // Valor injetado pelo painel solar, só pode ser positivo
    float energyGrid = readEnergyGrid();          // Valor consumido pela rede, só pode ser positivo

    // Exibição dos valores no console
    //printValues(voltage, current, power, energy, frequency, pf, InjectedEnergy, energyGrid);

    //Envia os valores para o MQTT
    sendToHomeAssistant("casa/medidor/InjectedEnergy", String(InjectedEnergy).c_str());
    //Serial.println("");
    //Serial.print("--------------> Energia injetada na rede ------> ");
    Serial.println(InjectedEnergy);
    delay(Delay);
    sendToHomeAssistant("casa/medidor/energy", String(energy).c_str());
    delay(Delay);
    sendToHomeAssistant("casa/medidor/energyGrid", String(energyGrid).c_str());  //EnergyGrid
    delay(Delay);
    sendToHomeAssistant("casa/medidor/voltage", String(voltage).c_str());
    delay(Delay);
    sendToHomeAssistant("casa/medidor/current", String(current).c_str());
    delay(Delay);
    sendToHomeAssistant("casa/medidor/power", String(power).c_str());
    delay(Delay);
    sendToHomeAssistant("casa/medidor/frequency", String(frequency, 1).c_str());
    delay(Delay);
    sendToHomeAssistant("casa/medidor/power_factor", String(pf).c_str());
    delay(Delay);
  }

  //bool ContinuarExec = true;

  //if (currentMillis - TempoAnterior_site >= TempoRotativo_site) {  // --> Atualiza o site do monitoramento solar a cada 2 minutos para pegar as variaveis
  //  TempoAnterior_site = currentMillis;
  //  //Adicionar aqui o código de verificação ------------
  //}

  //if (currentMillis - TempoAnterior >= TempoRotativo) {  // A cada 10 minutos, exceto na primeira vez do código
  //  TempoAnterior = currentMillis;

  // if (Y == 0 && !isnan(energy)) {
  //  TempoRotativo = intervalUpdateGrid;  //Atualiza o tempo
  //  energy1 = energy;
  //  ContinuarExec = false;
  //  Y++;
  //}

  //if (ContinuarExec) {
  //  Calcular();  //Chama as funções para calcular
  //  delay(30);

  // Adicione Serial.print para monitorar os valores
  //Serial.print("EnergyGridKwh: ");
  //Serial.println(MedidorGrid);
  //Serial.print("EnergySolarKwh: ");
  //Serial.println(MedidorInjetada);

  //if (SendsMqtt) {
  // SendsMqtt = false;
  //Envia o valores de consumo e injetado para o mqtt
  //Envia para mqtt
  //sendToHomeAssistant("casa/medidor/InjectedEnergy_total", String(MedidorInjetada).c_str());
  //delay(Delay);
  //sendToHomeAssistant("casa/medidor/energyGrid_total", String(MedidorGrid).c_str());  //EnergyGrid
  //delay(Delay);
}
//    }
//  }
//}




float readVoltage() {
  float voltage = pzem.voltage();
  if (!isnan(voltage)) {
    //Serial.print("Tensão: ");
    //Serial.print(voltage);
    //Serial.println("V");
  } else {
    //Serial.println("Erro ao ler a tensão");
  }
  return voltage;
}

float readCurrent() {
  float current = pzem.current();
  if (!isnan(current)) {
    //Serial.print("Corrente: ");
    //Serial.print(current);
    //Serial.println("A");
  } else {
    Serial.println("Erro ao ler a corrente");
  }
  return current;
}

float readPower() {
  float power = pzem.power();
  if (!isnan(power)) {
    //Serial.print("Potência: ");
    //Serial.print(power);
    //Serial.println("W");
  } else {
    Serial.println("Erro ao ler a potência");
  }
  return power;
}

float readEnergy() {
  float energy = pzem.energy();
  if (!isnan(energy)) {
    //Serial.print("Energia: ");
    //Serial.print(energy, 1);
    //Serial.println("kWh");
  } else {
    Serial.println("Erro ao ler a energia");
  }
  return energy;
}

float readFrequency() {
  float frequency = pzem.frequency();
  if (!isnan(frequency)) {
    //Serial.print("Frequência: ");
    //Serial.print(frequency, 1);
    //Serial.println("Hz");
  } else {
    Serial.println("Erro ao ler a frequência");
  }
  return frequency;
}

float readPowerFactor() {
  float pf = pzem.pf();
  if (!isnan(pf)) {
    //Serial.print("Fator de Potência: ");
    //Serial.println(pf);
  } else {
    Serial.println("Erro ao ler o fator de potência");
  }
  return pf;
}

void printValues(float voltage, float current, float power, float energy, float frequency, float pf, float InjectedEnergy, float energyGrid) {
  Serial.println();
  Serial.println("Valores:");
  Serial.print("  Tensão: ");
  Serial.print(voltage);
  Serial.println("V");
  Serial.print("  Corrente: ");
  Serial.print(current);
  Serial.println("A");
  Serial.print("  Potência: ");
  Serial.print(power);
  Serial.println("W");
  Serial.print("  Energia: ");
  Serial.print(energy, 3);
  Serial.println("kWh");
  Serial.print("  Frequência: ");
  Serial.print(frequency, 1);
  Serial.println("Hz");
  Serial.print("  Fator de Potência: ");
  Serial.println(pf);
}


void sendToHomeAssistant(const char* topic, const char* payload) {
  if (client.connected()) {
    //Serial.println("");
    //Serial.println("Enviando mensagem para o broker MQTT...");
    //Serial.print("Tópico: ");
    //Serial.println(topic);
    //Serial.print("Payload: ");
    //Serial.println(payload);

    // Tentar publicar a mensagem
    if (client.publish(topic, payload)) {
      //Serial.println("Mensagem publicada com sucesso");
    } else {
      Serial.println("Erro ao publicar a mensagem");
    }
  } else {
    Serial.println("Erro: não conectado ao broker MQTT");
    // Tentar reconectar automaticamente
    reconnect();
  }
  delay(30);
}

// Callback para tratar mensagens recebidas
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("");
  Serial.println("----------- Mensagem recebida -------------");
  Serial.print("Tópico recebido: ");
  Serial.println(topic);

  // Verifica se o tópico é o desejado
  if (strcmp(topic, mqtt_topic_solarpanel) == 0) {
    // Converte o payload em uma string e depois para long
    payload[length] = '\0';  // Garante que a string é terminada corretamente
    solarPanelValue = atof((char*)payload);

    // Agora solarPanelValue contém o valor recebido do tópico "casa/medidor/solarpanel"
    Serial.print("Valor recebido do tópico 'casa/medidor/solarpanel': ");
    Serial.println(solarPanelValue);
    //if (solarPanelValue == 0) {
      //Corrige erro do último MQTT
    //  Solar1[0].ValorSolar = Solar2[0].ValorSolar;
    //}
  }  // else if (strcmp(topic, mqtt_topic_solarpanel_total) == 0) {

  // Converte o payload em uma string e depois para long
  //   payload[length] = '\0';  // Garante que a string é terminada corretamente

  //   if (ResetSolar) {  // Só atualiza quando a energia for maior que 0.1 para corrigir erro de precisão no medidor solar
  //     Solar1[0].ValorSolar = Solar2[0].ValorSolar;
  //     ResetSolar = false;
  //   }  //Só pode atualizar quando passar do tempo da variavel --> intervalUpdateGrid
  //
  //   Solar2[0].ValorSolar = atof((char*)payload);
  //   Solar2[0].ID++;  //Atualiza o valor da ID também
  //
  //    if (X == 0) {
  //      Solar1[0].ValorSolar = Solar2[0].ValorSolar;
  //      Solar1[0].ID = Solar2[0].ID;  //Atualiza o valor da ID quando iniciado pela primeira vez
  //      X++;
  //    }
  //  }
}

// Adicione esta função para tentar reconectar automaticamente
void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando reconectar ao servidor MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Reconectado ao servidor MQTT");
      // Inscreva-se novamente no tópico após a reconexão
      client.subscribe(mqtt_topic_solarpanel);
      client.subscribe(mqtt_topic_solarpanel_total);
    } else {
      Serial.print("Falha na reconexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(10000);  // Espera 10 segundos antes de tentar novamente
    }
  }
}

//----------------------------------------------------------------------/----------------------------------------------------

float readEnergyInjected() {
  float energiaInjetada = solarPanelValue - readPower();
  //Zera a energia injetada quaso seja menor do que zero, ou seja só conta a energia positiva que foi injetada
  if (energiaInjetada < 0) {
    energiaInjetada = 0;
  }
  return energiaInjetada;
}

float readEnergyGrid() {
  float energyGrid = readPower() - solarPanelValue;
  //Zera a energiaGrid quaso seja menor do que zero, ou seja só conta a energia positiva que foi consumida.
  if (energyGrid < 0) {
    energyGrid = 0;
  }
  return energyGrid;
}

//----------------------------------------------------------------------/----------------------------------------------------

//Função pai
//void Calcular() {
//  verificarID();

// if (energy - energy1 >= 0.1) {
//   IntervaloInjetadaGrid();
//
//    //Atualiza o valor de energy
//    energy1 = energy;
//Pra enviar o mqtt
//    SendsMqtt = true;
//    ResetSolar = true;
//  }
//Toda vez que for executado, ou seja, a cada 10 minutos atualiza o valor de Tempo
//  Tempo++;

// if (Tempo >= 2) {
//   //Salva na EEPROM - A cada 24hrs
//   EEPROM.put(AdressFloat2, MedidorInjetada);
//  delay(300);
//    EEPROM.put(AdressFloat1, MedidorGrid);
//   delay(300);
//    EEPROM.commit();
//    //Zera o tempo
//    Tempo = 0;
//  }
//}

//void IntervaloInjetadaGrid() {
//  float IntervaloEnergia;
//  //Calcula a variação de energia entre 10 minutos
//  Serial.println(Solar2[0].ValorSolar - Solar1[0].ValorSolar);
//  IntervaloEnergia = (Solar2[0].ValorSolar - Solar1[0].ValorSolar) - (energy - energy1);
//  Serial.print("Resultado da equação: ");
//  Serial.println(IntervaloEnergia);
//  //Atualiza o valor de ID pra mostrar que já foi computado
//  Solar1[0].ID = Solar2[0].ID;
//  if (IntervaloEnergia < 0) {
//    Serial.print("Valor negativo INTERVALO ENERGIA --> ");
//    IntervaloEnergia = IntervaloEnergia * (-1);
//    MedidorGrid += IntervaloEnergia;
//    Serial.println(MedidorGrid);
//  } else {
//    MedidorInjetada += IntervaloEnergia;
//    Serial.print("Valor POSITIVO -- INTERVALO ENERGIA --> ");
//    Serial.println(IntervaloEnergia);
//  }
//}
//
//void verificarID() {
//  if (Solar1[0].ID == Solar2[0].ID) {
//    Serial.println("ID igual, logo, valor SOLAR resetado");
// Atualiza o valor de Solar caso o ID seja igual, dessa forma evita que um valor seja computado 2 vezes seguidas.
//    Solar1[0].ValorSolar = Solar2[0].ValorSolar;
//  }
//}
