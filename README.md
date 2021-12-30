# SLIC superpixels


## Requerimientos:
Hamachi, para crear una red local virtual
OpenSSH, Para la comunicacion entre los nodos
NFS, Para tener una carpeta compartida entre todoso los nodos.
Mpich, API para implementaciones paralelas

## Hamachi
Tener instalado desde la SnapStore
Iniciar el servicio de hamachi
Sempai123
sudo systemctl start logmein-hamachi
sudo systemctl status logmein-hamachi

## Crear la red
sudo hamachi login
sudo hamachi create <nombre> <contrase;a>

## Unirse a la red
sudo hamachi join <nombre> <contrase;a>

## Listar las redes y comprobar que se unio correctamente
sudo hamachi list


## Openssh

sudo pacman -S openssh
sudo systemctl enable sshd.service
sudo systemctl start sshd.service


## OPENSSH config

### /etc/hosts
Este archivo puede identificar a un host mediante un tag, que facilita el uso de los clusters.
    
  
### sudo nano /etc/hosts
  
ssh-keygen -t rsa

cd .ssh

sudo cp id_rsa.pub authorized_keys

ssh-copy-id user@<ip> o user@<tag>


#NFS
De parte del servidor editar esta entrada:
/etc/exports
Ejemplo:
  ~/Escritorio/Parallel-SLIC-Superpixels *(rw,sync,crossmnt,fsid=0)
  |      Directorio a ser compartido  | |direccion ip| (|argumentos|)
 
## Setup del proyecto

### Local
  Para pruebas locales, solo es necesario clonar el repositorio.
  
### Cluster
  Es necesario instalar todos los requerimientos y ejecutar con mpirun
  


  
# Referencia: SLIC Superpixel Implementation
This repository contains an implementation of the SLIC Superpixel algorithm by Achanta et al. (PAMI'12, vol. 34, num. 11, pp. 2274-2282). The C++ implementation is created to work with the strutures of OpenCV.

## Exemplary result
The images below shows an example of an over-segmentation using 400 superpixels and a weight factor of 40.
<p align="center">
  <img src="https://github.com/PSMM/SLIC-Superpixels/blob/master/dog.png?raw=true" alt="Dog"/>
  <img src="https://github.com/PSMM/SLIC-Superpixels/blob/master/dog_segmentation.png?raw=true" alt="Dog Segmentation"/>
</p>
