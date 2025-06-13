# Problematica
En este proyecto vamos a resolver la problematica de un banco que recibe múltiples transacciones simlutáneas de diferentes fuentes:

Se desarrollara codigos en C los cuales permitan mantener la constancia del saldo cuando múltiples operaciones ocurren al mismo tiempo.

# Comparación de Procesos vs Hilos en C

Este proyecto compara el rendimiento de dos enfoques de concurrencia en lenguaje C: uno utilizando procesos (`fork`) y otro utilizando hilos (`pthread`). Ambos programas realizan transacciones simuladas utilizando un conjunto común de datos para asegurar condiciones equivalentes de ejecución.

## Archivos incluidos

- `procesos.c` — Implementación basada en procesos.
- `hilos.c` — Implementación basada en hilos.
- `Transacciones.txt` — Archivo de entrada que contiene las transacciones a ejecutar.

## Requisitos del sistema

- Ubuntu Server
- GCC (`gcc`)
- Biblioteca de desarrollo pthread
- build-essential

Instalación recomendada:

```bash
sudo apt update
sudo apt install build-essential
```

## Compilación

Ejecutar los siguientes comandos desde el mismo directorio donde se encuentran los archivos:

```bash
# Compilar la versión con procesos
gcc -o procesos procesos.c -pthread -lrt

# Compilar la versión con hilos
gcc -o hilos hilos.c -pthread
```

## Preparación de archivos

Asegúrese de que los siguientes archivos estén presentes en el mismo directorio de trabajo:

```
procesos.c
hilos.c
Transacciones.txt
```

Si el archivo `Transacciones.txt` fue descargado como `Transacciones.txt.txt`, renómbralo correctamente:

```bash
mv Transacciones.txt.txt Transacciones.txt
```

## Ejecución

Ejecutar los programas desde la terminal:

```bash
./procesos
./hilos
```

## Resultados

Cada programa generará un archivo de salida:

- `TiempoProcesos.tx` — Contiene la salida del programa basado en procesos.
- `TiempoHilos.tx` — Contiene la salida del programa basado en hilos.

Estos archivos incluyen:

- El saldo final tras todas las transacciones
- El tiempo total de ejecución

## Verificación

- Ambos métodos deben producir el mismo saldo final, dado que usan los mismos datos de entrada.
- La diferencia de tiempos de ejecución puede variar dependiendo de la carga del sistema y recursos disponibles.

Este entorno permite analizar las diferencias de comportamiento y eficiencia entre la creación de procesos y el uso de hilos en tareas concurrentes.
