
# 🚦 Semáforo Inteligente com Acessibilidade

Este projeto implementa um sistema de semáforo inteligente com foco em acessibilidade para pessoas com deficiência visual, utilizando a placa **BitDogLab com o microcontrolador RP2040** e o sistema operacional em tempo real **FreeRTOS**.

---

## 🎥 Demonstração
O vídeo com a execução da simulação pode ser acessado em:
[🔗 Link para o vídeo]()

---

## 📋 Funcionalidades

- 🟢 **Modo Normal**: alternância entre os estados Verde → Amarelo → Vermelho.
- 🌙 **Modo Noturno**: LEDs piscando lentamente com alerta sonoro suave.
- 🧠 **Acessibilidade**: sinalização sonora distinta para cada estado do semáforo.
- 📟 **Display OLED**: exibe o estado atual com mensagens visuais claras.
- 💡 **Matriz WS2812**: representa os estados com cores e piscas visuais no centro da matriz.

---

## 🎮 Controles

- **Botão A (GPIO 5)**: alterna entre o modo normal e o modo noturno com debounce.

---

## 🛠️ Periféricos utilizados

| Componente      | Função no projeto |
|-----------------|-------------------|
| LED Verde       | Indica “Pode atravessar” |
| LED Amarelo     | Indica “Atenção" |
| LED Vermelho    | Indica “Pare” |
| Buzzer          | Feedback sonoro para cada estado |
| Botão A         | Alterna o modo de operação |
| Display OLED    | Exibe mensagens visuais com o estado atual |
| Matriz WS2812   | Representação visual colorida do semáforo |

---

## 🧱 Estrutura do Projeto

```
📁 semaforo_freertos/
├── lib/
│   ├── matriz.c / matriz.h         # Controle da matriz de LEDs WS2812
│   ├── ssd1306.c / ssd1306.h       # Driver do display OLED
│   ├── estado.h                    # Enumeração dos estados do semáforo
│   ├── FreeRTOSConfig.h            # Configuração do FreeRTOS
│   ├── font.h                      # Fonte usada no display
│   └── ws2812.pio.h                # Programa PIO do WS2812
├── semaforo_freertos.c             # Código principal com as tasks
├── ws2812.pio                      # Programa PIO bruto
├── CMakeLists.txt                  # Build com CMake
└── README.md                       # Este arquivo
```

---

## ⚙️ Como compilar e rodar

1. Clone este repositório:
```bash
git clone https://github.com/seu-usuario/semaforo_freertos.git
cd semaforo_freertos
```

2. Compile usando o **CMake** e **pico-sdk**:

```bash
mkdir build && cd build
cmake ..
make
```

3. Conecte sua placa RP2040 no modo BOOTSEL e arraste o `.uf2` gerado.

---

## 📝 Licença
Este programa foi desenvolvido como um exemplo educacional e pode ser usado livremente para fins de estudo e aprendizado.

## 📌 Autor
LORENZO GIUSEPPE OLIVEIRA BARONI
