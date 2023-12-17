# Preparação de Ambiente e Bibliotecas RapsBerry Pico

[[_TOC_]]


## 1. Preparar ambiente 

Todas as referências para configuração do ambiente pode ser contrada no seguinte link:

https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html

### 1.1 Preparar ambiente linux - Resumido


Para preparar o ambiente de desenvolvimento deve seguir os seguintes passos:

- Criar os diretorio `~/pico`:

```
$ cd ~/
$ mkdir pico
$ cd pico
```

- Clonar o sdk e os exemplos:

```
$ git clone https://github.com/raspberrypi/pico-sdk.git --branch master
$ cd pico-sdk
$ git submodule update --init
$ cd ..
$ git clone https://github.com/raspberrypi/pico-examples.git --branch master
```

- Instalacão do Toolchain

```
$ sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential 
```
## 2. Preparando a placa de desenvolvimento:

Sigua o link para informações detalhadas do procedimento para conectar o botão de reset:
https://embarcados.com.br/programacao-da-raspberry-pi-pico/

- Para conectar o botão de reset, contecte o botão nos pinos PIN30(RUN) e GND.

## 3. Configurando o ambiente de desenvolvimento para este projeto - Resumido:

Para configuando o ambiente de desenvolvimento deve seguir os seguintes passos:

- Clonar esse projeto para dentro do diretorio: 

```
~/pico
```

- Dentro do diretório deste projeto, o diretório  `build` deve ser criado;

```
$ cd picolib
$ mkdir build
$ cd build
```

- A variável de ambiente a seguir de ser executada no terminal dentro do diretório `~/pico/picolib/build`:

[!NOTE] ATENÇÃO: A variável de ambiente de ser setada toda vez que abir o terminal.

```
$ export PICO_SDK_PATH=../../pico-sdk
```

### 3.1 Comando para compilar:
- Compilar com uma thread: 
    ```
    $ make
    ```
    
- Compilar com 4 thread: 
    ```
    $ make -j4
    ```    

- Limpar arquivos compilados:
    ```
    $ make clean
    ```

### 3.2 Carregar códido:

- Deve iniciar as Rasp Pico em modo de boot, para isso bastar conectar ou resetar o pico pressionando o botão de boot que se encontra na placa do pico. 

- O pico será reconhecido como uma mídia externa pelo linux.

-  Depois bastar copiar o arquivo `main.uf2` para dentro do pico:

```
cp ~/pico/picolib/build/main.uf2  /media/<nome_usuario>/RPI-RP2
```

