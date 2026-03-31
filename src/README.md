# Desenvolvimento e Firmware

Nesta pasta, você encontrará os componentes de software que podem ser compartilhados publicamente.

### Disponível:
* **Gateway ESP32:** Código-fonte para conexão Wi-Fi e publicação de tópicos MQTT (Sensores simulados de PSI via potenciômetro). Demonstra a lógica de transmissão para a nuvem.

### Restrito (Sigilo Industrial):
* **Lógica CAN (Arduino Nano):** O firmware que realiza o *sniffing* e a decodificação dos PIDs não está disponível para proteger a propriedade intelectual da montadora.
* **Mobile App (React Native):** O código fonte contém chaves de API e endpoints privados.

> **Nota:** A comunicação entre o Arduino (CAN) e a ESP32 (MQTT) é feita via Serial. O código da ESP32 aqui presente foca na recepção desses dados e no envio dos sensores de pressão de pneus (TPMS) simulados.