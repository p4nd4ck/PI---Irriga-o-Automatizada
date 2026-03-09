### 1. Objetivo do Projeto

Desenvolver um sistema de irrigação automatizado utilizando *ESP32* para otimizar o uso de água na cultura do milho, garantindo que a planta receba hidratação apenas quando necessário e em horários de menor evaporação.

---

### 2. Arquitetura de Hardware

O sistema é composto por dispositivos que monitoram o ambiente e executam ações físicas:

* *Cérebro:* Microcontrolador ESP32 DevKit V4.
* 
*Sensores:* * *DHT22:* Mede a temperatura e umidade do ar.


* *Higrômetro (Potenciômetro):* Simula a umidade do solo.
* 
*RTC DS3231:* Relógio de tempo real para precisão de horários.




* *Atuadores e Interface:*
* 
*Relé:* Controla o acionamento da bomba d'água.


* 
*LCD 16x2 I2C:* Exibição local de dados para o operador.





---

### 3. Inteligência e Regras de Negócio

A automação não é aleatória. Ela segue um algoritmo de decisão rigoroso para economia de recursos:

| Critério | Regra Configurada | Motivação Técnica |
| --- | --- | --- |
| *Frequência* | Dias Pares (now.day() % 2 == 0) 

 | Evitar saturação do solo e fungos. |
| *Janela Temporal* | Entre 21h e 11h 

 | Menor taxa de evaporação solar. |
| *Umidade Solo* | Abaixo de 2000 unidades 

 | Irrigar apenas se o solo estiver seco. |
| *Vazão Estimada* | 10 Litros por Minuto 

 | Cálculo preciso de custos e recursos. |

---

### 4. Ecossistema de Dados (IoT & MQTT)

O projeto é "conectado", permitindo que o agricultor monitore tudo pelo celular ou computador via protocolo MQTT:

#### *Telemetria (O que o sistema diz):*

* 
*Status do Sistema:* Informa se a bomba está "LIGADO" ou "PARADO".


* 
*Dados Ambientais:* Envio constante de temperatura e umidade do ar.


* 
*Gestão de Água:* Publicação do volume total consumido em litros.


* 
*Saúde do Dispositivo:* Envio do uptime (tempo ligado) para manutenção.



#### *Comandos Remotos (O que você pode fazer):*

* 
*Manual ON/OFF:* Ligar (1) ou desligar (0) a bomba à distância.


* 
*Modo Auto:* Retornar ao controle inteligente (auto).


* 
*Reset:* Zerar o contador de litros acumulados (reset).



---

### 5. Resumo de Benefícios

1. 
*Economia de Água:* Irrigação baseada em dados reais, não em estimativas.


2. 
*Redução de Custos:* Operação em horários de tarifa de energia possivelmente reduzida e menor perda por evaporação.


3. 
*Escalabilidade:* O uso do protocolo MQTT permite integrar múltiplos pivôs em um único painel de controle.
