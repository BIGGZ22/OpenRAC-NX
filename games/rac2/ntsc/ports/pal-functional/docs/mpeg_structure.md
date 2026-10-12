# English

## Ratchet & Clank 2 (PAL) - MPEG & Multi-language Cinematic Structure

This document details the reverse engineering findings regarding the cinematic asset structure for Ratchet & Clank 2 (Going Commando / Locked and Loaded) PAL version. This information is intended for educational, research, and future modding development (such as custom localization and voice-over injection).

---

## ⚠️ Legal Disclaimer
 
This repository does not host, distribute, or share copyrighted assets, proprietary code, or media files from Insomniac Games or Sony Interactive Entertainment. All information provided here is derived from clean-room reverse engineering and file analysis. No `.pss` or `.asset` binaries are included.

---

## Technical Overview

### Directory Layout
In the PAL version, pre-rendered cutscenes are stored in a dedicated folder called `mpeg/`. Inside, the assets are organized into numbered subdirectories:

```text
mpeg/
  ├── 001/
  │    ├── video_pal.pss
  │    └── [cinematic_name].asset
  ├── 002/
  │    ├── video_pal.pss
  │    └── [cinematic_name].asset
  └── ...
```

---

## File Specifications

### 1. `video_pal.pss`
* **Type / Tipo:** PlayStation Stream Format (MPEG-2 Video + Audio container).
* **Role / Rol:** Contains the raw synchronized video track formatted for PAL regions (50Hz / 25fps or standard resolution scaling).

### 2. `.asset` Metadata Container
This is the core mapping file that enables multi-language support in the European (PAL) release. Instead of hardcoding text or baking multiple audio tracks directly into the `.pss` stream, the engine references this dynamic metadata.

#### Component Mappings:
* **Subtitles:** Contains text strings or pointers for multiple languages (English, Spanish, French, German, Italian). The game checks the PS2 BIOS/system language and displays the matching subtitle stream.
* **Duration:** Timecodes, frame counts, or millisecond values indicating how long the scene lasts and when specific subtitles should trigger.
* **Dynamic Voice-Over Routing:** Maps the correct external localization audio file corresponding to the chosen language. If the selected language folder contains the dialogue track, it plays natively; otherwise, it falls back to the default language track.

---

## Modding Potential for PC Ports

Understanding this layout allows modern native ports (like Bevy/Rust engine rewrites) or external loaders to cleanly hook custom language packs. By creating or modifying the `.asset` structure, developers can inject fan-made translations, high-quality audio files, or custom subtitles without decompiling or breaking the original game structure.

# Español

## Estructura de Cinemáticas MPEG y Multilenguaje en Ratchet & Clank 2 (PAL)

Este documento detalla los hallazgos de ingeniería inversa respecto a la estructura de cinemáticas para la versión PAL de Ratchet & Clank 2. Esta información tiene fines educativos, de investigación y para el desarrollo de futuros mods (como localización personalizada e inyección de voces).

---

## ⚠️ Descargo de Responsabilidad Legal

Este repositorio no aloja, distribuye ni comparte archivos protegidos por derechos de autor, código propietario o archivos multimedia de Insomniac Games o Sony Interactive Entertainment. Toda la información aquí provista se deriva de ingeniería inversa y análisis de archivos. No se incluyen binarios `.pss` ni `.asset`.

---

## Resumen Técnico

### Estructura de Directorios

En la versión PAL, las cinemáticas pre-renderizadas se almacenan en una carpeta dedicada llamada `mpeg/`. Dentro, los recursos están organizados en subdirectorios numerados:

```text
mpeg/
  ├── 001/
  │    ├── video_pal.pss
  │    └── [cinematic_name].asset
  ├── 002/
  │    ├── video_pal.pss
  │    └── [cinematic_name].asset
  └── ...
```

---

## Especificaciones de Archivos

### 1. `video_pal.pss`
* **Type / Tipo:** PlayStation Stream Format (MPEG-2 Video + Audio container).
* **Rol (ES):** Contiene la pista de video cruda sincronizada y formateada para la región PAL (50Hz / 25fps o escalado de resolución estándar).

### 2. Contenedor de Metadatos `.asset`
Este es el archivo de mapeo central que permite el soporte multilenguaje en la versión europea (PAL). En lugar de integrar texto fijo o incrustar múltiples pistas de audio directamente en el flujo del `.pss`, el motor hace referencia a estos metadatos dinámicos.

#### Mapeo de Componentes:
* **Subtítulos:** Contiene las cadenas de texto o punteros para múltiples idiomas (inglés, español, francés, alemán, italiano). El juego verifica el idioma del sistema/BIOS de la PS2 y muestra el flujo de subtítulos correspondiente.
* **Duración:** Códigos de tiempo, conteo de fotogramas o valores en milisegundos que indican cuánto dura la escena y cuándo deben activarse subtítulos específicos.
* **Enrutamiento Dinámico de Voz:** Mapea el archivo de audio de localización externo correcto correspondiente al idioma elegido. Si la carpeta del idioma seleccionado contiene la pista de diálogo, se reproduce de forma nativa; de lo contrario, recurre a la pista por defecto.

---

## Potencial para Mods en Ports de PC

Comprender esta estructura permite que los ports nativos modernos (como las reconstrucciones en motores Bevy/Rust) o cargadores externos conecten limpiamente paquetes de idiomas personalizados. Al crear o modificar la estructura `.asset`, los desarrolladores pueden inyectar traducciones hechas por fans, archivos de audio de alta calidad o subtítulos personalizados sin necesidad de descompilar o romper la estructura original del juego.