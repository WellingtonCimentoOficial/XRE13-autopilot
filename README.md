# 🏍️ Autopilot — Circuito de Controle

Circuito baseado em **ESP32** para leitura dos sinais da motocicleta e controle do sistema de acionamento do motor.

O circuito possui:

- Condicionamento para os sinais digitais de **velocidade**, **freio** e **embreagem**
- Leitura analógica do **acelerador**
- Isolamento do **botão** através de **B05005S-1W + PC817**
- **Display OLED**
- **LED de status**
- Controle **PWM** do driver do motor

---

## 📑 Índice

- [Visão geral](#-visão-geral)
- [Entradas](#-entradas)
  - [Velocidade](#-velocidade)
  - [Freio](#-freio)
  - [Acelerador](#️-acelerador)
  - [Embreagem](#-embreagem)
- [74HC14](#-74hc14)
- [Botão](#-botão)
- [Display OLED](#-display-oled)
- [LED de status](#-led-de-status)
- [Driver do motor](#️-driver-do-motor)
- [Pinout completo do ESP32](#-pinout-completo-do-esp32)
- [Alimentação](#-alimentação)
- [Componentes principais](#-componentes-principais)
- [Resumo das conexões](#-resumo-das-conexões)

---

## 🔌 Visão geral

O circuito é dividido nos seguintes blocos:

- 🏍️ Leitura de velocidade
- 🛑 Leitura do freio
- 🎚️ Leitura do acelerador
- 🦾 Leitura da embreagem
- 🔘 Botão com isolamento óptico
- 📺 Display OLED
- 💡 LED de indicação
- ⚙️ Controle do driver do motor
- 🔄 Condicionamento de sinais através do 74HC14

---

## 📡 Entradas

### 🏍️ Velocidade

O sinal de velocidade passa por um circuito de condicionamento antes de entrar no 74HC14.

**Ligações**

```text
Sinal de velocidade
        │
       R1
      10K
        │
        ├──────────> 1A — 74HC14
        │
       R2
       1K
        │
       R3
      2,2K
        │
       GND
```

**Componentes**

| Componente | Valor  |
|------------|--------|
| R1         | 10 KΩ  |
| R2         | 1 KΩ   |
| R3         | 2,2 KΩ |

A saída **1Y** do 74HC14 é conectada ao **GPIO 25** do ESP32.

---

### 🛑 Freio

O sinal de freio é condicionado através de resistor e capacitor antes de entrar no 74HC14.

**Ligações**

```text
Sinal de freio
      │
     R1
    39K
      │
      ├──────────> 2A — 74HC14
      │
     100nF
      │
     R2
    10K
      │
     GND
```

**Componentes**

| Componente | Valor  |
|------------|--------|
| R1         | 39 KΩ  |
| R2         | 10 KΩ  |
| C1         | 100 nF |

A saída **2Y** do 74HC14 é conectada ao **GPIO 4**.

---

### 🎚️ Acelerador

O acelerador possui leitura analógica, portanto o sinal é conectado ao **GPIO 32** do ESP32, que possui entrada ADC.

> ⚠️ O sinal **não** passa pelo 74HC14.

**Ligações**

```text
Sinal do acelerador
        │
       R1
      20K
        │
        ├──────────> GPIO 32 (ADC)
        │
       R2
      39K
        │
       GND
```

**Componentes**

| Componente | Valor |
|------------|-------|
| R1         | 20 KΩ |
| R2         | 39 KΩ |

O GPIO 32 é utilizado como entrada analógica para realizar a leitura da tensão do acelerador.

---

### 🦾 Embreagem

O sinal da embreagem passa por um circuito de condicionamento antes de entrar no 74HC14.

**Ligações**

```text
Sinal da embreagem
        │
       R1
      4,7K
        │
        ├──────────> 4A — 74HC14
        │
       R2
       1K
        │
      100nF
        │
       R3
       10K
        │
       GND
```

**Componentes**

| Componente | Valor  |
|------------|--------|
| R1         | 4,7 KΩ |
| R2         | 1 KΩ   |
| R3         | 10 KΩ  |
| C1         | 100 nF |

A saída **4Y** do 74HC14 é conectada ao **GPIO 19**.

---

## 🔄 74HC14

O 74HC14 é utilizado para **condicionamento e inversão** dos sinais digitais de velocidade, freio e embreagem.

**Alimentação**

```text
VCC → 3.3V
GND → GND
```

Adicionar um capacitor de **100 nF** entre VCC e GND, preferencialmente próximo ao CI.

**Pinout utilizado**

| 74HC14 | Função             | ESP32   |
|--------|--------------------|---------|
| 1A     | Entrada velocidade | —       |
| 1Y     | Saída velocidade   | GPIO 25 |
| 2A     | Entrada freio      | —       |
| 2Y     | Saída freio        | GPIO 4  |
| 4A     | Entrada embreagem  | —       |
| 4Y     | Saída embreagem    | GPIO 19 |
| VCC    | Alimentação        | 3.3V    |
| GND    | Alimentação        | GND     |

---

## 🔘 Botão

O botão utiliza isolamento óptico através do conjunto **B05005S-1W** e **PC817**.

**B05005S-1W**

| Pino | Conexão          |
|------|------------------|
| 1    | GND              |
| 2    | 5V               |
| 3    | Cátodo do PC817  |
| 4    | Botão            |

**PC817**

```text
Anodo
  │
 R 1K
  │
Botão

Coletor → GPIO 18
Emissor → GND
```

O PC817 fornece isolamento entre o circuito do botão e a entrada do ESP32.

---

## 📺 Display OLED

O display OLED utiliza comunicação **I²C**.

| OLED | ESP32   |
|------|---------|
| GND  | GND     |
| VDD  | 3.3V    |
| SCK  | GPIO 22 |
| SDA  | GPIO 21 |

**Pinout I²C**

```text
SCK → GPIO 22
SDA → GPIO 21
```

---

## 💡 LED de status

O LED é acionado diretamente pelo **GPIO 5** através de um resistor limitador de corrente.

```text
GPIO 5
  │
220Ω
  │
Anodo do LED
  │
LED
  │
Catodo
  │
GND
```

**Componentes**

| Componente | Valor |
|------------|-------|
| Resistor   | 220 Ω |

---

## ⚙️ Driver do motor

O ESP32 controla o sentido e o acionamento do motor através de sinais **PWM**.

| Driver | Conexão |
|--------|---------|
| GND    | GND     |
| VCC    | 5V      |
| RPWM   | GPIO 27 |
| LPWM   | GPIO 26 |

**Sinais PWM**

```text
GPIO 27 → RPWM
GPIO 26 → LPWM
```

Os dois GPIOs são utilizados para controle PWM do driver.

---

## 📌 Pinout completo do ESP32

| GPIO    | Função     | Tipo    |
|---------|------------|---------|
| GPIO 4  | Freio      | Digital |
| GPIO 5  | LED        | Saída   |
| GPIO 18 | Botão      | Digital |
| GPIO 19 | Embreagem  | Digital |
| GPIO 21 | OLED SDA   | I²C     |
| GPIO 22 | OLED SCK   | I²C     |
| GPIO 25 | Velocidade | Digital |
| GPIO 26 | Motor LPWM | PWM     |
| GPIO 27 | Motor RPWM | PWM     |
| GPIO 32 | Acelerador | ADC     |

---

## 🔋 Alimentação

O circuito utiliza duas tensões principais:

| Tensão | Utilização                          |
|--------|-------------------------------------|
| 3.3V   | ESP32, 74HC14 e OLED                |
| 5V     | B05005S-1W e lógica do driver do motor |
| GND    | Referência comum do circuito        |

> ⚠️ **Importante:** o sinal do acelerador é uma entrada analógica e deve permanecer conectado ao GPIO 32, sem passar pelo 74HC14.

---

## 🧩 Componentes principais

- ESP32
- 74HC14
- B05005S-1W
- PC817
- Display OLED I²C
- Driver de motor
- LED
- Resistores
- Capacitores de 100 nF

---

## 📋 Resumo das conexões

| Sistema    | GPIO / Conexão  |
|------------|-----------------|
| Velocidade | GPIO 25         |
| Freio      | GPIO 4          |
| Acelerador | GPIO 32 — ADC   |
| Embreagem  | GPIO 19         |
| Botão      | GPIO 18         |
| LED        | GPIO 5          |
| OLED SDA   | GPIO 21         |
| OLED SCK   | GPIO 22         |
| Motor RPWM | GPIO 27         |
| Motor LPWM | GPIO 26         |
