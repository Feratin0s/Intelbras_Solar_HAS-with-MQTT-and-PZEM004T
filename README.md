# Intelbras_Solar_HAS-with-MQTT-and-PZEM004T com ESP8266 com OTA

Este projeto permite monitorar o consumo e a injeção de energia em instalações elétricas monofásicas, especialmente útil para sistemas com geração solar INTELBRAS.

## 🛠 Componentes Necessários

- **Medidor de Energia PZEM-004T com Amperímetro**
- **Módulo ESP8266** (NodeMCU, Wemos D1 Mini, etc.)

## ⚡ Instalação Física

### Posicionamento dos Sensores
1. **PZEM-004T**: Deve ser instalado no **cabos principal** da entrada de energia
2. **Amperímetro**: Conectado ao PZEM004-T e na fase principal
3. **ESP8266**: Responsável pela comunicação e envio dos dados e leitura serial do medidor

## 🔌 Configuração do Firmware

O código para o ESP8266 está disponível na pasta `medidor_energia_v2`:

1. Configure as credenciais Wi-Fi
2. Defina o endereço do Broker MQTT
3. Ajuste os pinos de comunicação com o PZEM-004T (Está no 4 e 5)
4. Configure uma senha OTA
5. Carregue o sketch no ESP8266

## 📊 Node-RED Flow

### ⚠️ Importante: Broker MQTT Requerido
Este projeto **requer um Broker MQTT** funcionando para comunicação entre os dispositivos.

### Configuração do Flow
No repositório você encontra o arquivo `Intelbras_Solar.json` que contém:

1. **Conexão com Broker MQTT**: Configure o endereço do seu broker no NODEs MQTT
2. **Integração com Intelbras**: 
   - Faz requisições automáticas ao site da Intelbras
   - Captura dados de produção de energia solar (Watts e total)
3. **Processamento de Dados**:
   - Combina dados do PZEM-004T (via ESP8266)
   - Calcula energia injetada e consumida da rede
   - Funciona **mesmo sem flow direction** do medidor

### Funcionalidades do Flow
- 📈 **Monitoramento em Tempo Real**: Consumo e produção solar
- 🔄 **Cálculo de Energia Líquida**: Diferença entre consumo e geração
- 📊 **Dashboard Visual(HAS)**: Interface para acompanhamento
- 💾 **Armazenamento de Dados**: Histórico de medições

## 🎯 Como Funciona o Cálculo

O sistema calcula automaticamente:
- **Energia Consumida da Rede**: Quando o consumo é maior que a geração
- **Energia Injetada na Rede**: Quando a geração solar excede o consumo

## ⚙️ Configuração NodeRed

1. Importe o flow `Intelbras_Solar.json` no Node-RED
2. Configure o Broker MQTT nos nós correspondentes
3. Ajuste as credenciais do sistema Intelbras no NODE Function "envia login"
4. Verifique a comunicação com o ESP8266

## ⚙️ Configuração HomeAssistant

1. Importe o arquivo .yaml ou copie o que a dentro para as configurações do seu HomeAssistant
2. Configure o Broker MQTT

