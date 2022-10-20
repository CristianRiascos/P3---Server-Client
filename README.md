# Mini ECS


El programa posee 4 archivos (makefile, server y 2 clients), que se ejecutan por medio de los comandos: make t3_server, make t3_client, make t3_client2. Ejecutados en dicho orden, es decir, primero el servidor (para unirse al puerto 8888 y para buscar a quién conectarse) y después los clientes.


Ambos clientes se encargan de establecer una conexión con el puerto del host (8888) para poder conectarse al socket. Seguido a esto tienen 5 opciones disponibles para realizar: crear, listar, detener, borrar (contenedores) y finalizar el programa respectivamente. Cada una de dichas opciones está enumerada de 1-4 para las acciones de los contenedores y 0 para salir. Para crear, detener o borrar se necesita el nombre del contenedor e imagen y para listar no se necesita nada. Cuando el cliente ingresa cualquiera de estas opciones se envía lo que escribe como una cadena de texto al servidor.


Dentro del servidor, tomará la cadena de texto proveniente de la petición del cliente (previamente conectado) y acorde al número ingresado se hará una u otra cosa. Si el número ingresado es 0, el cliente terminará su programa y el servidor queda esperando que el mismo u otro cliente se conecte al servidor, si es diferente a los números de 0-4 se enviará el mensaje al cliente especificando que la opción no está disponible y si está entre 1-4 hará la creación de hilos. Cada opción que digite el usuario genera un hilo distinto para cumplir la petición.


Todos los hilos tienen incluido su respectivo join, debido a que se requiere que cada hilo complete su trabajo antes de recibir otra petición, por ejemplo, para borrar un contenedor primero se debe de detener, entonces por este tipo de situaciones se programó de dicha manera.


Las opciones disponibles son las siguientes: 1 - Crear contenedor, 2 - Listar contenedores, 3 - Detener contenedor, 4 - Borrar contenedor y las respectivas validaciones son hechas en archivos txt que contienen la información de contenedor (status: estado name: nombreContenedor image:nombreImagen).


Crear Contenedor: Para esta opción se hace uso de la función searchWord para validar que el nombre del contenedor no exista, en caso de no existir la función devolverá -1 y entrará a la función addContainerWord para añadir la información del contenedor al txt (su estado será "running"), posteriormente ejecuta el comando de creación del contenedor por medio del execlp. En caso de que el nombre del contenedor exista saldrá de la función y enviará un mensaje de error al cliente junto con el motivo del error.

Listar Contenedor: Para esta opción se verifica que el archivo txt no esté vacío, si está vacío envía un mensaje de error al cliente junto con el motivo y finaliza la opción. En caso de no estar vacío imprime el contenido del txt.

Detener Contenedor: Para esta opción se hace uso de la función changeStatus, esta función hará uso del txt que almacena la información de los contenedores (containers.txt) y un archivo temporal. Para cambiar el estado del contenedor se itera desde el inicio hasta el final del archivo containers.txt y cada línea del mismo se irá imprimiendo en el temporal, cuando se encuentré la línea que contenga el nombre del contenedor a cambiar su estado, se cambia el nombre del estado a stopped y pondrá dicha línea en el temporal. Al finalizar, elimina el archivo containers.txt y cambia el nombre del temporal a containers.txt, posteriormente se ejecuta el comando por medio de execlp. Si dentro de la función no se encuentra el nombre del contenedor, entonces envía mensaje de error al cliente y su motivo.

Borrar Contenedor: Para esta función se hacen 2 validaciones: si el nombre del contenedor existe y si está detenido (en dicho orden), cuando alguna de esas 2 no se cumple se envía mensaje de error al cliente y su motivo. Si ambas opciones se cumplen entonces se usará la función deleteContainerFile que funciona de igual forma que changeStatus, con el cambio de que al encontrar la línea a eliminar solo sigue derecho sin imprimir nada respecto a esa línea en el temporal. Al terminar ese proceso se ejecuta el comando mediante execlp.


Cada una de estas funciones posee un mutex ubicado en el manejo de archivo, bien sea lectura o escritura. Esto para que solo haya un hilo que pueda leer y/o modificar el archivo. 


Al finalizar la petición del usuario, se cierra el hilo y se envía un mensaje de confirmación (o error) acorde a la petición al cliente. El programa de igual forma cuenta de forma nativa con el comando para inicializar docker. 




