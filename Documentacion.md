# Tarea Corta #2 de Redes
Tarea programada #2 de Redes.  
**Estudiante**:
  - Jonder Hernández Gutiérrez

**Carnet**:
  - 2018203660

**Objetivo general**:  
Implementar una calculadora de subredes IPv4 utilizando lenguaje de programación C y Docker compose.

## <font size="6">Cómo correr el proyecto</font>

### <font size="5">**En Windows**</font>

1. Instalar Docker y Docker Compose, para esto se puede apoyar en la siguiente página: <https://docs.docker.com/desktop/windows/install/>
    - Puede descargar el ejecutable que incluye Docker y Docker Compose en: <https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe>
2. Entrar a la carpeta del proyecto, utilizando la consola de windows y el comando 'cd' desplazarse en los directorios del sistema hasta encontrar la carpeta donde se encuentra el proyecto, también se puede utilizar la dirección completa del archivo.
3. Una vez se tiene la consola ubicada dentro de la dirección del proyecto utilizar el comando:  

    - ```bash
      docker-compose up --build
      ```

    - Algunas veces en Windows es necesario añadir manualmente la carpeta de \Docker\resources\bin a la
      variable de entorno PATH para que reconozca el comando ***docker-compose***.

### <font size="5">**En linux**</font>

#### <font size="4">**Configurar repositorio:**</font>

1. Actualizar la lista de paquetes con:

    - ```bash
      sudo apt-get update
      ```

2. Permitir apt usar un repositorio por medio de HTTPS

    - ```bash
      sudo apt-get install \
      ca-certificates \
      curl \
      gnupg \
      lsb-release
      ```

3. Añadir la llave oficial GPG de Docker
   - ```curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg```
4. Configurar la rama stable del Repositorio:

    - ```bash
      echo \
      "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
      $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
      ```

#### <font size="4">**Instalar Docker Engine:**</font>

1. Instalamos la última versión de Docker Engine, containerd y Docker Compose:

    - ```bash
      sudo apt-get update && \
      sudo apt-get install docker-ce docker-ce-cli containerd.io docker-compose-plugin
      ```

#### <font size="4">**Instalar Docker Compose:**</font>

1. Usamos este comando para descargar la última versión estable de Docker Compose:

    - ```bash
      DOCKER_CONFIG=${DOCKER_CONFIG:-$HOME/.docker} && \
      mkdir -p $DOCKER_CONFIG/cli-plugins && \
      curl -SL https://github.com/docker/compose/releases/download/v2.4.1/docker-compose-linux-x86_64 -o && \
      $DOCKER_CONFIG/cli-plugins/docker-compose
      ```

2. Damos permisos para ejecutar el instalador:

    - ```bash
      chmod +x $DOCKER_CONFIG/cli-plugins/docker-compose
      ```

3. Probamos la instalación con:

    - ```bash
      docker compose version
      ```

#### <font size="4">**Iniciar Docker Compose:**</font>

  1. Entramos en la carpeta con el archivo *docker-compose.yaml* y ejecutamos el comando:

      - ```bash
        docker-compose up --build
        ```


## <font size="6">Como conectarse al servidor</font>
La conección al servidor se puede hacer por medio de sockets con alguna implementación, en este caso usaremos "***telnet***" ya que nos facilitara las cosas.

Para conectarse al cliente es necesario primero instalar telnet si no se encuentra instalado, seguidamente se ejecuta el siguiente comando:
```bash
telnet 127.0.0.1 9801
```
Si no se cuenta localmente con el servidor se usa el el ip especifico en lugar de "127.0.0.1", ejemplo:
```bash
telnet 186.177.110.87 9801
```

Para **cerrar** la conexión una vez establecida la conexión se puede enviar lo siguiente para pedir al servidor cerrar la conexión:
```
exit
```
o sino tambien:
```
EXIT
```
## <font size="6">Pruebas</font>
Aqui se describen unas pruebas para probar cada funcionalidad. Los mensajes de prueba se ingresan una vez conectado con el servidor, para esto ver la sección anterior.
### <font size="5">Broadcast</font>
Pruebas para la funcionalidad para la obtención del Broadcast Ip:
1.  ```
    GET BROADCAST IP 10.8.2.5 MASK /29
    ```
    Resultado esperado: 
    ```
    10.8.2.7
    ```

2.  ```
    GET BROADCAST IP 172.16.0.56 MASK 255.255.255.128
    ```
    Resultado esperado: 
    ```
    172.16.0.127
    ```

### <font size="5">Número de Red</font>
Pruebas para la funcionalidad para la obtención del número de red:
1.  ```
    GET NETWORK NUMBER IP 10.8.2.5 MASK /29
    ```
    Resultado esperado: 
    ```
    10.8.2.0
    ```

2.  ```
    GET NETWORK NUMBER IP 172.16.0.56 MASK 255.255.255.128
    ```
    Resultado esperado: 
    ```
    172.16.0.0
    ```
    
### <font size="5">Número de subred aleatorio</font>
Pruebas para la funcionalidad para la obtención del número de subred aleatorio:
1.  ```
    GET RANDOM SUBNETS NETWORK NUMBER 10.0.0.0 MASK /8 NUMBER 3 SIZE /24
    ```
    Ejemplo de posibles resultados: 
    ```
    10.20.10.0/24
    10.33.11.0/24
    10.42.13.0/24
    ```

# Referencias
- [1] Ibrahim, M. (2019). Echo server and client using sockets in c. Mohsensy. Recuperado de https://mohsensy.github.io/programming/2019/09/25/echo-server-and-client-using-sockets-in-c.html.

- [2] Regular expressions in C - GeeksforGeeks. GeeksforGeeks. Recuperado de https://www.geeksforgeeks.org/regular-expressions-in-c/

- [3] Cordes, P. (2021). How to count the number of set bits in a 32-bit integer. Stack Overflow. Recuperado de https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer.
