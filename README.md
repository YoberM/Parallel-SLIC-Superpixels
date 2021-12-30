# SLIC superpixels

# Setup del proyecto

## Notas
   * ### Setup Local
      * Para pruebas locales, solo es necesario clonar el repositorio y tener mpich.
  
   * ### Setup Cluster
      * Todos los nodos deben tener el mismo usuario (mpitest por ejemplo)
      * Todos los nodos deben usar la misma version de OS Ubuntu (20.04)
      * Es necesario instalar todos los requerimientos

## Requerimientos:
* Hamachi : Es una aplicacion para crear la red local virtual (VLAN)
* OpenSSH : Es una aplicacion para la comunicacion entre los nodos.
* NFS     : Se usa para compartir una carpeta entre todos los nodos.
* Mpich   : API para implementar programan que se ejecuten en paralelo.

## Hamachi
* Instalarlo desde https://vpn.net/linux
* Iniciar el servicio de hamachi
   * ### Host
   * El host debe crear una red privada, sudo hamachi create <nombre-red> <contraseña>
   * Ejemplo: sudo hamachi create SLICnet 1234
   * ### Worker
   * Cada worker debe unirse a la red, sudo hamachi join <nombre-red> <contraseña>
   * Ejemplo: sudo hamachi join SLICnet 1234

* Para comprobar que se han unido correctamente y mostrar todas las computadoras conectadas.
   * sudo hamachi list


## Openssh
* ### Instalacion
   * sudo apt-get install openssh-server
   * sudo systemctl enable ssh
   * sudo systemctl start ssh
* ### Configuracion de /cat/hosts 
   * Agregar un identificador a cada ip
   * cat /etc/hosts , editar con: sudo nano /etc/hosts
     ```
     <ip-master> <identificador>
     <ip-worker> <identificador>
     <ip-worker2 <identificador>
     ```
   * Ejemplo : 
     ```
     23.93.20.1 master
     23.93.20.2 worker1
     23.93.20.3 workerLocal
     ```
* ### Conceder acceso con claves autorizadas - Master
   * ssh-keygen -t rsa
   * ssh-copy-id <ip-workerx>
   * Ejemplo:
        * ssh-copy-id worker1 o ssh-copy-id workerLocal

## NFS
* ### Instalacion
   * Master: sudo apt install nfs-kernel-server
   * Worker: sudo apt install nfs-common
* ### Configuracion de master
   * crear directorio
     ```
        mkdir /path
        Ejemplo:
        mkdir /home/user/SharedFolder
     ```
   * Configurar exports /etc/exports
      * Visualizar: cat /etc/exports , Editar : sudo nano /etc/exports
        ```
        /path <ip>(rw,sync,no_subtree_check)
        Ejemplo:
        /home/user/ShareFolder 23.90.20.1/24(rw,sync,no_subtree_check)
        Aclaracion: /24 es la subnet, por lo que cualquier ip dentro del rango sera aceptada
        Nota: Es posible usar * para permitir a cualquier ip, es solo recomendable usar * para fines academicos
        ```
   * Finalmente reinicie nfs para aplicar los cambios 
     ```
     sudo systemctl restart nfs-kernel-server
     ```
* ### Configuracion de worker
   * Tener un usuario y path iguales para que pueda usar la carpeta compartida:
     * Path de master:  /home/user/SharedFolder
     * Path de worker:  /home/user/SharedFolder
   * Montar la carpeta
        ```
        sudo mount <ip o identificador>:/path /workerpath
        Ejemplo ip 
        sudo mount 23.93.20.1:/home/user/SharedFolder /home/user/SharedFolder
        Ejemplo identificador 
        sudo mount 23.93.20.1:/home/user/SharedFolder /home/user/SharedFolder
        ```
* ### MPICH
   * Compilacion:
        ```
        mpic++ archivo.cpp -o salida archivo1.cpp 
        ```
   * Ejecucion:
        * Especificar nodos mediante el identificador /cat/hosts:
        ```
        mpirun -host master,worker1 salida
        ```
        * Archivo con arugmentos
        ```
        mpirun -host master,worker1 salida arg1 arg2 arg3
        ```
        * Especificar cantidad de nucleos por nodo
        ```diff
        mpirun -host master:2 ,worker1:2 salida arg1 arg2 arg3
        ```
       

  
# Referencia: SLIC Superpixel Implementation
This repository contains an implementation of the SLIC Superpixel algorithm by Achanta et al. (PAMI'12, vol. 34, num. 11, pp. 2274-2282). The C++ implementation is created to work with the strutures of OpenCV.

## Exemplary result
The images below shows an example of an over-segmentation using 400 superpixels and a weight factor of 40.
<p align="center">
  <img src="https://github.com/PSMM/SLIC-Superpixels/blob/master/dog.png?raw=true" alt="Dog"/>
  <img src="https://github.com/PSMM/SLIC-Superpixels/blob/master/dog_segmentation.png?raw=true" alt="Dog Segmentation"/>
</p>
