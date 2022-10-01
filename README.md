# P3---Server-Client


El programa posee 4 archivos (makefile, server y 2 clients), que se ejecutan por medio de los comandos: make t3_server, make t3_client, make t3_client2. Ejecutados en dicho orden, es decir, primero el servidor (para unirse al puerto 8888 y para buscar a quién conectarse) y después los clientes.


Ambos clientes se encargan de establecer una conexión con el puerto del host (8888) para poder conectarse al socket. Seguido a esto tienen 5 opciones disponibles para realizar: crear, listar, detener, borrar contenedores y finalizar programa respectivamente. Cada una de dichas opciones está enumerada de 1-4 para las acciones de los contenedores y 0 para salir. Para crear se necesita el nombre del contenedor e imagen; para listar no se necesita nada y para detener o borrar se necesita o el nombre del contenedor o el ID. Al cliente ingresar cualquiera de estas opciones se envía lo que escribe como una cadena de texto al servidor.


Dentro del servidor, tomará la cadena de texto proveniente de la petición del cliente (previamente conectado) y acorde al número ingresado se hará una u otra cosa. Si el número ingresado es 0, el cliente terminará su programa y el servidor queda esperando que el mismo u otro cliente se conecte al servidor, si es diferente a los números de 0-4 se enviará el mensaje al cliente especificando que la opción no está disponible y si está entre 1-4 hará la creación de hilos. Cada opción que digite el usuario genera un hilo distinto para cumplir la petición.


Todos los hilos tienen incluido su respectivo join, debido a que se requiere que cada hilo complete su trabajo antes de recibir otra petición, por ejemplo, para borrar un contenedor primero se debe de detener, entonces por este tipo de situaciones se programó de dicha manera.


Al finalizar la petición del usuario, se cierra el hilo y se envía un mensaje de confirmación acorde a la petición al cliente. El programa de igual forma cuenta de forma nativa con el comando para poner a correr docker. 


