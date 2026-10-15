# English

# Localization Research: Hidden Multilingual Structure and Feasibility of a Language Selector in the PC Port

This document details the reverse-engineering findings regarding the text database and the feasibility of implementing a native language selector in the **Ratchet & Clank: Going Commando** PC port, based on the remaining data structure in the NTSC version (USA Black Label v1.01) and previous community solutions.

---

## 1. Summary of Technical Findings

Upon analyzing the decompressed files from the US ISO using the native extractor (`gc-extract`), it was discovered that the global level containers (**`gameplay.bin`**) and cinematic metadata (**`mpeg/*.bin`**) contain a **hidden multilingual localization structure (including full text in Spanish)**.

Unlike other titles that segregate translations by geographic region into separate files, Insomniac Games’ engine for *Ratchet & Clank 2* uses a system of **interleaved string blocks** indexed by dynamic pointers (*offsets*).

### In-Memory Storage Structure:
The file does not replicate separate menus or complete interfaces. Instead, it indexes each text identifier (`Text_ID`) and places its language variants consecutively before moving on to the next entry:

`[Text_ID_001] -> [English String] [00] [French String] [00] [German String] [00] [Spanish String] [00]`

*Note: The null byte (`00`) acts as the structural delimiter that the graphics engine uses to stop reading the string on screen.*

---

## 2. Implementing the “Host Stand-in” Solution (Language Selector)

In the commercial version of the NTSC PlayStation 2, access to these Spanish texts remains unavailable because the original executable strictly (*hardcodes*) the locale ID to `0` (English), overriding the console’s BIOS settings.

Inspired by the technical solution implemented by developer **daino-selvatico** in the sister project *ReRAC* (`fix/language-kept` branch), it is entirely feasible to implement a native language selection patch on PC by manipulating the *Host* environment (`src/host/host_language`):

1. **Configuration Persistence:** Configure the native port to act as an intermediary for system calls from the game engine.
2. **ID Injection:** When the executable launches, the Rust code intercepts the locale read and injects the ID corresponding to the desired language (e.g., `ID 3` for Spanish).
3. **Strings Wake-Up:** This forces the port’s format reader to read the correct offset within `gameplay.bin`, enabling menus, pause screens, and Spanish subtitles immediately without altering a single physical byte of the original game.

---

## 3. Status of Audio Assets (Voices) and Injection Plan

Although the text coexists natively on the North American ISO, the high-density audio files (dialog compressed in 4-bit ADPCM format / `.vag`) corresponding to the European dubs were removed from the NTSC disc to optimize DVD storage space.

To achieve a fully localized experience without altering the graphics processing codebase, the following modding methodology is proposed:

* **External Legal Extraction:** The user must provide their own legally acquired copy of the PAL (European) ISO.
* **Binary Replacement Mapping (File Swapping):** Using community-developed decompression tools (such as Wrench) or the port’s native pipeline, the Spanish audio banks must be extracted and placed in the `audio/` directory of the NTSC version’s development environment, renaming them to match the American sound identifier calls exactly.

---

## ⚠ Legal Disclaimer / Source

This repository and its documentation are intended strictly for educational, research, and software preservation purposes. Disk images (.ISO), proprietary executables, audio binary files (.VAG), or copyrighted materials belonging to Insomniac Games or Sony Interactive Entertainment are not distributed, hosted, or shared. The entire process requires the end user to provide their own legitimately obtained game files.

# Español

# Investigación de Localización: Estructura Multilenguaje Oculta y Viabilidad de Selector en PC Port

Este documento detalla los hallazgos de ingeniería inversa respecto a la base de datos de texto y la viabilidad de implementar un selector de idiomas nativo en el port de **Ratchet & Clank: Going Commando**, basado en la estructura de datos remanente en la versión NTSC (USA Black Label v1.01) y soluciones previas de la comunidad.

---

## 1. Resumen del Hallazgo Técnico

Al analizar los archivos descompresionados de la ISO americana mediante el extractor nativo (`gc-extract`), se descubrió que los contenedores globales de los niveles (**`gameplay.bin`**) y los metadatos cinematográficos (**`mpeg/*.bin`**) albergan una estructura de localización **multilenguaje oculta (incluyendo texto completo en Español)**. 

A diferencia de otros títulos que segregan las traducciones por región geográfica en archivos independientes, el motor de Insomniac Games para *Ratchet & Clank 2* utiliza un sistema de **bloques de strings intercalados** indexados por punteros dinámicos (*offsets*). 

### Estructura de Almacenamiento en Memoria:
El archivo no replica menús ni interfaces completas por separado. En su lugar, indexa cada identificador de texto (`Text_ID`) y coloca sus variantes lingüísticas de forma consecutiva antes de proceder al siguiente registro:

`[Text_ID_001] -> [String Inglés] [00] [String Francés] [00] [String Alemán] [00] [String Español] [00]`

*Nota: El byte nulo (`00`) actúa como el delimitador estructural que el motor gráfico utiliza para detener la lectura de la cadena en pantalla.*

---

## 2. Aplicación de la Solución "Host Stand-in" (Selector de Idiomas)

En la versión comercial de PlayStation 2 NTSC, el acceso a estos textos en español permanece inaccesible debido a que el ejecutable original fuerza de manera estricta (*hardcoded*) el ID de configuración regional a `0` (Inglés), omitiendo los parámetros de la BIOS de la consola.

Inspirado en la solución técnica implementada por el desarrollador **daino-selvatico** en el proyecto hermano *ReRAC* (rama `fix/language-kept`), es completamente viable implementar un parche de selección nativa en PC mediante la manipulación del entorno del *Host* (`src/host/host_language`):

1. **Persistencia de Configuración:** Configurar el port nativo para que actúe como un intermediario frente a las llamadas del sistema del motor del juego.
2. **Inyección de ID:** Al iniciar el ejecutable, el código en Rust intercepta la lectura regional e inyecta el ID correspondiente al idioma deseado (ej. `ID 3` para Español). 
3. **Despertar de Strings:** Esto fuerza al lector de formatos del port a leer el desplazamiento (*offset*) correcto dentro de `gameplay.bin`, habilitando menús, interfaces de pausa y subtítulos en castellano de forma inmediata y sin alterar un solo byte físico del juego original.

---

## 3. Estado de los Activos de Audio (Voces) y Plan de Inyección

Aunque los textos coexisten de manera nativa en la ISO americana, los archivos de audio de alta densidad (diálogos comprimidos en formato ADPCM de 4 bits / `.vag`) correspondientes a las doblajes europeos fueron removidos del disco NTSC para optimizar el espacio de almacenamiento del DVD.

Para obtener una experiencia completamente localizada sin alterar el código base de procesamiento gráfico, se plantea la siguiente metodología de modding:

* **Extracción Legal Externa:** El usuario debe suministrar su propia copia adquirida legalmente de la ISO PAL (Europea).
* **Mapeo por Reemplazo Binario (File Swapping):** Utilizando herramientas de descompresión comunitarias (como Wrench) o el pipeline nativo del port, se deben extraer los bancos de audio en español e introducirlos en el directorio `audio/` del entorno de desarrollo de la versión NTSC, renombrándolos para que coincidan exactamente con las llamadas de los identificadores de sonido americanos.

---

## ⚠ Descargo de Responsabilidad Legal / Provenance

Este repositorio y su documentación tienen fines estrictamente educativos, de investigación y de preservación de software. No se distribuyen, alojan ni comparten imágenes de disco (.ISO), ejecutables propietarios, archivos binarios de audio (.VAG), ni materiales protegidos por derechos de autor pertenecientes a Insomniac Games o Sony Interactive Entertainment. Todo el proceso requiere que el usuario final provea sus propios archivos de juego obtenidos de manera legítima.
