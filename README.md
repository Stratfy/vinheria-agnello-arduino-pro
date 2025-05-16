
# Sistema de Monitoramento Ambiental – Stratfy Embedded

![Status](https://img.shields.io/badge/Status-Em%20Operação-brightgreen)
![Plataforma](https://img.shields.io/badge/Plataforma-Arduino%20UNO-blue)
![Linguagem](https://img.shields.io/badge/Linguagem-C/C++-lightgrey)
![Licença](https://img.shields.io/badge/Licen%C3%A7a-MIT-yellow)

Projeto de sistema embarcado para monitoramento de luminosidade, temperatura e umidade, com registro de eventos na EEPROM, animações gráficas no display LCD e interação via teclado matricial. Desenvolvido para aplicações em ambientes inteligentes, IoT e processos educativos.

---

## Funcionalidades

- Animação de inicialização com sprites no LCD
- Menu interativo com navegação por teclas (A, B, C, D)
- Leitura de:
  - Luminosidade (LDR)
  - Temperatura (DHT22)
  - Umidade (DHT22)
- Visualização dos dados no LCD com indicadores gráficos (sol, gota, termômetro, etc.)
- Registro de eventos na EEPROM com:
  - Data e hora (via RTC DS1307)
  - Leitura de luz, temperatura e umidade
- Flags de alerta personalizáveis:
  - Luminosidade acima do limite
  - Temperatura acima do limite
  - Umidade acima do limite
- Cooldown configurável entre registros
- Aviso visual (LED) quando a EEPROM estiver cheia
- Reset para configurações de fábrica

---

## Componentes Utilizados

| Componente         | Quantidade | Descrição                                     |
|--------------------|------------|-----------------------------------------------|
| Arduino Uno        | 1x         | Microcontrolador principal                   |
| Sensor LDR         | 1x         | Sensor de luminosidade                       |
| Sensor DHT22       | 1x         | Sensor de temperatura e umidade              |
| Display LCD 16x2 I2C | 1x       | Interface de exibição                        |
| RTC DS1307 + CR2032| 1x         | Relógio de tempo real com backup de bateria  |
| Teclado Matricial 4x4 | 1x      | Entrada de comandos                          |
| LED                | 1x         | Indicador de EEPROM cheia                    |
| Resistores diversos| Diversos   | Para LDR e outros componentes                |
| Protoboard         | 1x         | Montagem do circuito                         |
| Jumpers            | Diversos   | Conexões entre os componentes                |

---

## Estrutura do Projeto

```
stratfy_monitoramento_embarcado/
├── main.ino                        # Código completo do sistema
├── README.md                        # Documentação do projeto
├── LICENSE                           # Licença do projeto (MIT)
```

---

## Como Operar

1. **Na primeira execução:**
   - O sistema solicita configuração de luz mínima e máxima.
   - Use o menu `Setup → Setup LDR`.

2. **Configurações adicionais:**
   - Flags de luminosidade, temperatura, umidade e cooldown podem ser definidas no menu `Setup`.

3. **Funcionamento:**
   - O sistema faz leituras contínuas dos sensores.
   - As leituras são exibidas no LCD com indicadores gráficos:
     - Sol para luminosidade
     - Gota para umidade
     - Termômetro ou floco de neve para temperatura
   - Quando qualquer valor ultrapassa seu limite configurado, o sistema:
     - Salva um evento na EEPROM com timestamp e valores
     - Exibe “FLAG SALVO” no LCD
   - Um LED acende quando a EEPROM atinge sua capacidade máxima (~140 flags).

4. **Logs e Debug:**
   - Acessar pelo menu `Logs`:
     - Print das flags na serial
     - Limpeza das flags na EEPROM

---

## Lógica do Sistema

- **EEPROM:**  
Cada flag ocupa 7 bytes:
- Timestamp (4 bytes)
- Luminosidade (1 byte)
- Temperatura (1 byte)
- Umidade (1 byte)

Capacidade: ~140 registros.

- **CoolDown:**  
Permite definir um tempo mínimo entre registros consecutivos.

- **Navegação:**  
Utiliza teclado matricial:
- `A`: Subir
- `B`: Descer
- `C`: Voltar
- `D`: Enter/Selecionar

- **Animações:**  
Ao ligar, exibe uma aranha animada puxando o nome “Stratfy”.

---

## Requisitos

- **IDE:** Arduino IDE 1.8+ ou plataforma WokWI (simulação)
- **Bibliotecas:**  
  - `LiquidCrystal_I2C.h`  
  - `Keypad.h`  
  - `DHT.h`  
  - `RTClib.h`  
  - `EEPROM.h` (nativa)

---

## Equipe

| Nome               | Função            |
|--------------------|-------------------|
| Fábio Cabrini      | Desenvolvimento e Projeto Eletrônico |

---

## Licença

Este projeto está licenciado sob os termos da licença MIT. Consulte o arquivo [`LICENSE`](LICENSE) para mais detalhes.

---

<p align="center"><b>Desenvolvido com dedicação pela equipe Stratfy</b></p>
