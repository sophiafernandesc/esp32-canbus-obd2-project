#  Monitoramento Veicular IoT com CAN Bus, ESP32 e React Native

**PUC MINAS | Projeto de Redes Veiculares**  
**Integrantes:** 

---

##  Visão Geral

Este projeto implementa um ecossistema de telemetria veicular utilizando o protocolo **CAN (Controller Area Network)**. O sistema captura dados de uma arquitetura elétrica real, processa via hardware híbrido (Arduino + ESP32) e disponibiliza as informações em nuvem (MQTT) para visualização em um aplicativo mobile.

---

##  Arquitetura do Sistema

O projeto foi estruturado em quatro camadas principais:

1.  **Aquisição (Hardware Automotivo):** Um **Arduino Nano** integrado ao controlador **MCP2515** realiza a leitura dos PIDs via porta **OBD-II**.
2.  **Simulação de Sensores:** 5 potenciômetros foram acoplados para simular o sistema **TPMS (Tyre Pressure Monitoring System)**, enviando dados de PSI de cada pneu.
3.  **Gateway IoT:** Uma **ESP32** recebe os dados do Arduino e atua como ponte para a nuvem via protocolo **MQTT**.
4.  **Interface Mobile:** Aplicativo em **React Native** que consome os tópicos MQTT para exibição em tempo real.

---

## Ambiente de Testes 

A validação do projeto ocorreu na bancada de arquitetura eletroeletrônica (conhecida como **Plywood**) do **Fiat Cronos**. 

![Bancada Plywood Fiat Cronos](./public/assets/plywood.jpeg)

> ###  Agradecimentos
> Agradecemos à **Stellantis** pela doação deste equipamento ao laboratório de engenharia da **PUC Minas**. A utilização de um chicote elétrico e módulos (ECUs) originais foi fundamental para a análise real do comportamento do barramento CAN e sucesso deste projeto.

---

##  Hardware e Interface

![img hardware](./public/assets/imghardware2.jpeg)

###  Esquema do Protótipo

![img esquema](./public/assets/esquemaHardware.jpeg)

---

##  Aplicativo e Design

O aplicativo Android foi desenvolvido para oferecer uma experiência de diagnose automotiva, focando em baixa latência na atualização dos dados.

###  WInterface

![interfaces app](./public/assets/wireframesAPP.jpeg)

---

## Nota sobre Propriedade Intelectual e Segurança

Este repositório tem como objetivo documentar a arquitetura de sistemas e a integração de hardware/software desenvolvida. No entanto, em conformidade com as diretrizes de segurança e sigilo industrial, o código-fonte completo do Arduino e do Aplicativo não estão disponíveis publicamente pelos seguintes motivos:

Embora a extração de dados tenha sido realizada via *sniffing* independente, optamos por **não publicar o dicionário de mensagens (IDs e offsets)** nem o firmware completo do Arduino neste repositório. A divulgação do dicionário de mensagens resultante desta análise poderia comprometer a integridade da arquitetura eletroeletrônica da montadora.

Segurança de API e Credenciais: O aplicativo utiliza endpoints e chaves de acesso privadas.

