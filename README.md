# Preparação de Ambiente Para RaspBerry Pico

## 1. Preparar ambiente 

Todas as referências para configuração do ambiente podem ser encontradas no seguinte link:

https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html

### 1.1 Preparar ambiente linux - Resumido


Para preparar o ambiente de desenvolvimento deve seguir os seguintes passos:

- Criar o diretorio `~/pico`:

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

- Instalação do Toolchain

```
$ sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential 
```
## 2. Preparando a placa de desenvolvimento:

Sigua o link para informações detalhadas do procedimento para conectar o botão de reset:
https://embarcados.com.br/programacao-da-raspberry-pi-pico/

- Para conectar o botão de reset, conecte os terminais do botão nos pinos PIN30(RUN) e GND.

## 3. Configurando o ambiente de desenvolvimento para este projeto - Resumido:

Para configurando o ambiente de desenvolvimento deve seguir os seguintes passos:

- Clonar esse projeto para dentro do diretorio: 

```
cd ~/pico
git clone https://github.com/AndreFPDA/picolib.git
```

- Dentro do diretório deste projeto, o diretório  `build` deve ser criado;

```
$ cd picolib
$ mkdir build
$ cd build
```

- A variável de ambiente a seguir deve ser executada no terminal dentro do diretório `~/pico/picolib/src/build`:

[!NOTE] ATENÇÃO: A variável de ambiente e o CMAKE devem ser executados toda vez que abir o terminal.

```
$ export PICO_SDK_PATH=../../../pico-sdk
```


```
cmake ..
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

### 3.2 Carregar código:

- Deve iniciar a Rasp Pico em modo de boot, para isso bastar conectar ou resetar o Rasp Pico pressionando o botão de boot que se encontra na placa. 

- O Rasp Pico será reconhecido como uma mídia externa pelo linux.

-  Depois bastar copiar o arquivo `main.uf2`, que sencontra em `~/pico/picolib/build`, para dentro do pico:

```
cp ~/pico/picolib/build/main.uf2  /media/<nome_usuario>/RPI-RP2
```

### 3.3 Monitor serial:

- Putty - Recomendado:

O Putty é uma monitor usados para teste de varios tipos de comunicação. Ele permite ser usado como monitor serial.

Está disponível para Linux e Windows.

Pode ser baixado pelo seguinte link: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html


## 4. Edição do código

### 4.2  Clang Format - Para manter o código com organização padronizada - OBRIGATÓRIO.

- Instalação do Clang Format 11:
  
```
sudo apt install clang-format-11
```

- Para formatar os arquivos *.hpp e *.cpp, exemplo:
  
```
clang-format-11 -i nome_arquivo_formatar.cpp
```

### 4.3 Ambiente VSCode - Recomendado

- Pacotes linux que devem ser instalados previamente:

- Aplicativos VSCode:
    - C/C++ v1.16.3 -> Microsoft
    - C/C++ Extension Pack-> Microsoft
    - Clang-Format -> Xaver Hellauer
    - GitLens -> GitLens

Para evitar problemas com acentos e formatos:
- No VSCode, Configurações, Settings, Text Editor, Code Actions On Save, clicar em “Edit in settings.json”. 
- Para acessar o settings.json, use o comando de atalho <Ctrl+Shift+P> e pesquise pelo nome do arquivo.
- Copie e cole o as seguintes configurações:
```
{
    "files.encoding": "UTF-8",
    "terminal.integrated.persistentSessionScrollback": 100000000,
    "terminal.integrated.scrollback": 100000000,
    "codegnuglobal.autoupdate": false,
    "editor.defaultFormatter": "xaver.clang-format",
    "clang-format.assumeFilename": ".clang-format",
    "editor.formatOnSave": true,
    "clang-format.executable": "clang-format-11",
    "window.zoomLevel": -1
}
```


