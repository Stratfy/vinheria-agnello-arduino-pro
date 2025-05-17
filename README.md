# Vinheria Agnello - Sistema de Monitoramento Ambiental com Arduino

![Status](https://img.shields.io/badge/status-finalizado-brightgreen)
![Projeto Acadêmico](https://img.shields.io/badge/tipo-Projeto%20Acadêmico-orange)
![Linguagem](https://img.shields.io/badge/código-C++-blue)
![Plataforma](https://img.shields.io/badge/plataforma-Arduino-blueviolet)
![Licença](https://img.shields.io/badge/licença-MIT-informational)
![Versão](https://img.shields.io/badge/versão-1.0-lightgrey)

Projeto acadêmico desenvolvido na FIAP com foco em monitoramento ambiental aplicado ao armazenamento de vinhos. O sistema realiza leitura contínua de temperatura, umidade e luminosidade, com interface interativa via LCD, menus configuráveis e persistência de dados na EEPROM. Eventos críticos são automaticamente registrados para análise posterior.

## Visão Geral do Circuito

> **Simulação disponível no Wokwi:**  
> [Link da simulação Wokwi](https://wokwi.com/projects/430958871022611457)  


![Esquema do Circuito](https://i.imgur.com/IGAaf9L.png)


## Funcionalidades Técnicas

- Interface gráfica em LCD 16x2 (modo paralelo).
- Navegação por menus via Keypad 4x4.
- Animação de introdução com sprites personalizados.
- Monitoramento contínuo:
  - Sensor DHT22 (Temperatura e Umidade)
  - Sensor LDR (Luminosidade)
- Cálculo de médias com base em múltiplas amostras (20 leituras).
- Registro de eventos críticos (flags) na EEPROM com:
  - Timestamp (RTC DS1307)
  - Luminosidade (% mapeado)
  - Temperatura
  - Umidade
- Configuração dos valores de alerta diretamente no menu.
- Cooldown ajustável para evitar registros repetidos.
- Diagnóstico completo via Serial com função de debug.
- Estrutura de dados persistente para reconfiguração automática.

## Componentes Utilizados

| Componente        | Descrição                        |
|------------------|----------------------------------|
| Arduino Uno       | Microcontrolador principal       |
| LCD 16x2          | Interface de saída paralela      |
| DHT22             | Sensor de temperatura e umidade  |
| LDR               | Sensor de luminosidade analógico |
| DS1307            | RTC para registro de hora        |
| Keypad 4x4        | Interface de entrada             |
| EEPROM interna    | Armazenamento de dados persistente |
| Buzzer            | Emissão sonora de alerta         |
| LEDs              | Sinalização visual               |
| Potenciômetro     | Ajuste de contraste do LCD       |

## Organização da EEPROM

```
[0 – 19]    → Configurações do sistema
[20 – ...]  → Flags com eventos (até 140+ registros)
              - Timestamp (4 bytes)
              - Luminosidade (%) (1 byte)
              - Temperatura (1 byte)
              - Umidade (1 byte)
```

## Mapa de Menus

```
Menu Principal:
  1. Display         → Inicia monitoramento em tempo real
  2. Setup           → Parâmetros do sistema
  3. Logs            → Diagnóstico e depuração

Submenu de Setup:
  - Velocidade do texto (scroll)
  - Unidade de temperatura (°C/°F)
  - Fuso horário (offset)
  - Reset de fábrica
  - Ativação da introdução
  - Setup de luminosidade mínima/máxima
  - Limites de alerta (luz, temperatura, umidade)
  - Cooldown entre registros
```

## Execução

1. Monte o circuito conforme o diagrama disponível na simulação Wokwi ou na imagem.
2. Faça upload do código na Arduino IDE.
3. Utilize o Keypad para navegar pelos menus.
4. Configure os valores mínimos e máximos de luminosidade pelo menu Setup.
5. Ative o modo "Display" para iniciar o monitoramento.
6. Flags serão salvas automaticamente quando algum valor ultrapassar os limites definidos.

Para depuração:
- Use a opção `Logs > Print Log` para visualizar os registros no monitor serial.
- Use a opção `Logs > Limpar Flag` para resetar os dados da EEPROM.

## Equipe do Projeto

| Nome                | Responsabilidade                            |
|---------------------|---------------------------------------------|
| Anthony Sforzin     | Desenvolvimento                             |
| Luigi Cabrini       | Desenvolvimento                             |
| Rogério Arroyo      | Desenvolvimento                             |
| Thayná Simões       | Desenvolvimento                             |
| Bruno Koeke         | Desenvolvimento                             |

## Licença

Este projeto está licenciado sob os termos da **Licença MIT**.  

---

**Versão do Projeto:** 1.0  
**Status:** Finalizado
