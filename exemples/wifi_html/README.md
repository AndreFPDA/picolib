# Wi-Fi com Raspberry Pi Pico 2 W

Este exemplo inicializa o CYW43 do Raspberry Pi Pico 2 W, conecta-se a uma rede
Wi-Fi e exibe o endereço IP obtido no monitor serial USB.

## Requisitos

- Raspberry Pi Pico 2 W
- Pico SDK com suporte ao alvo `pico2_w`
- Rede Wi-Fi de 2,4 GHz

## Configuração

Edite `src/blink_simple.c` e informe as credenciais:

```c
#define WIFI_SSID "nome-da-rede"
#define WIFI_PASSWORD "senha-da-rede"
```

O exemplo usa `CYW43_AUTH_WPA2_AES_PSK`. Redes abertas, WPA/WPA3-only,
portais cativos e redes de 5 GHz não são atendidos por esta configuração.

## Compilação

```shell
cd src
mkdir -p build
cd build
export PICO_SDK_PATH=~/pico/pico-sdk
cmake ..
make -j4
```

O arquivo gerado será `pico2w_wifi_exemplo.uf2`. Conecte o Pico 2 W ao
computador mantendo o botão BOOTSEL pressionado e copie esse arquivo para a
unidade `RPI-RP2`.

## Monitor serial

Abra o monitor serial USB a 115200 baud. Após a inicialização, a saída esperada
é semelhante a:

```text
Conectando ao Wi-Fi 'nome-da-rede'...
Conectado com sucesso!
IP obtido: 192.168.1.123
```
