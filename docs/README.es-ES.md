# Accessible OBS Studio 1.0

[![Logotipo de Tiflo.Info: ondas azules sobre el texto Tiflo.info y el lema ruso «Cierra los ojos y mira»](../assets/tiflo-info-logo.jpg)](https://tiflo.info)

Accessible OBS Studio es un complemento de accesibilidad para OBS Studio en Windows. Está pensado para personas ciegas que trabajan con el teclado y un lector de pantalla, especialmente JAWS o NVDA.

## Requisitos e instalación

Requiere Windows 10 u 11 de 64 bits y una versión compatible de OBS Studio 32 de 64 bits. Solo las funciones de OpenAI necesitan una clave API propia y conexión a Internet.

Cierre OBS y ejecute `AccessibleOBSStudio-1.0-Setup.exe`. El instalador coloca el complemento en `C:\ProgramData\obs-studio\plugins\accessible-obs-studio` y comprueba Microsoft WebView2 y Visual C++. Solo descarga de Microsoft los componentes que falten; para ello necesita conexión a Internet. No sustituye archivos de OBS ni de Qt. Para desinstalarlo, use Aplicaciones instaladas de Windows. La configuración se conserva salvo que elija expresamente eliminarla.

## Teclas predeterminadas

- F3: capturar y analizar el lienzo mediante OpenAI.
- Ctrl+M: enfocar los Controles multimedia nativos visibles.
- F5: iniciar o detener la transmisión.
- F6 / Mayús+F6: área principal siguiente / anterior.
- F7: iniciar o detener la grabación.
- F8: activar o desactivar la cámara virtual.
- Ctrl+acento grave: abrir la Consola de volumen accesible. Se refiere a la tecla física situada justo debajo de Escape; su carácter depende de la distribución del teclado.
- Ctrl+0 a Ctrl+5: enfocar Vista previa, Escenas, Fuentes, Mezclador de audio, Transiciones o Controles.

El comando **Accessible OBS Studio: Open Accessible OBS Studio** abre el editor de atajos. Se le puede asignar un atajo, pero no tiene ninguno de forma predeterminada.

En el primer inicio, el complemento desactiva los atajos de OBS cuando la ventana principal de OBS no tiene el foco, pero únicamente si el usuario nunca eligió esa opción. Los cambios posteriores siempre se respetan.

## Editor de atajos

Abra **Herramientas > Accessible OBS Studio**. Busque un comando y pulse Intro o active **Añadir o editar**. La configuración de OpenAI está disponible mediante **API Settings** en esta ventana. El cuadro «Hot Key» permite gestionar varios atajos. **Añadir otro atajo** crea una asignación adicional. Intro acepta y Escape cancela. Tab, Mayús+Tab, Intro, Escape, Alt+F4, combinaciones con la tecla Windows y órdenes reservadas del sistema no se capturan.

Los cambios se guardan en OBS solo al activar Aceptar en la ventana principal. Supr o **Eliminar** borra todas las asignaciones del comando seleccionado. Al cerrar con cambios pendientes se ofrecen Guardar, Descartar y Cancelar.

## Audio y medios

Ctrl+3 enfoca el Mezclador de audio. Los controles de volumen visibles se numeran en el orden mostrado. 1 a 9 seleccionan las primeras nueve fuentes y 0 la décima. Las flechas y los demás controles siguen siendo funciones nativas de OBS. Ctrl+M enfoca los Controles multimedia cuando OBS los muestra; el complemento no sustituye las teclas multimedia de OBS.

Ctrl+acento grave abre la **Consola de volumen accesible** modal. El complemento recuerda el control de OBS que tenía el foco y lo restaura al pulsar Escape. La consola incluye todas las fuentes de audio actualmente activas en el Mezclador de OBS, incluidas las fuentes aplicables de grupos o escenas anidadas y los dispositivos globales. Izquierda y Derecha cambian de fuente, Arriba y Abajo modifican el volumen en 1 dB, Inicio restablece 0 dB y Espacio alterna el silencio. Las teclas 1 a 9 seleccionan las nueve primeras fuentes y 0 la décima. Los cambios son inmediatos y JAWS y NVDA los anuncian. El comando y su atajo pueden modificarse en el editor.

## Análisis del lienzo y acciones

La clave API es opcional. Guárdela solo cuando la necesite mediante **API Settings** en el editor de atajos. El cuadro admite Tab y Mayús+Tab; Intro en el campo de la clave activa **Guardar clave**. El formato y la autenticación de OpenAI se comprueban antes del almacenamiento cifrado en el Administrador de credenciales de Windows. Sin clave se bloquean únicamente las funciones de OpenAI; las demás funciones siguen disponibles.

F3 no exige enfocar la vista previa. Un breve clic confirma que la solicitud ha comenzado. El foco permanece donde estaba durante la captura. La ventana del resultado recibe el foco después y, al cerrarla, se restaura el control anterior de OBS si todavía existe. Nunca se roba el foco a otra aplicación.

Las preguntas posteriores deben referirse directamente a la imagen capturada. El texto de la imagen y las preguntas se tratan como datos no fiables. Se rechazan las consultas ajenas a la imagen, y la conversación termina al cerrar la ventana o efectuar otra captura.

**Acciones sugeridas** presenta una lista limitada de transformaciones reversibles de OBS. Si no hay una fuente seleccionada, se abre una lista de fuentes accesible. Cada acción tiene su propia casilla y nivel de riesgo. Nada se ejecuta hasta activar **Aplicar selección**. Se usan las acciones y el historial Deshacer de OBS. Quedan excluidos transmisión, grabación, audio, credenciales, órdenes arbitrarias y ajustes de salida.

## Compatibilidad

Ante una versión principal de OBS no probada, el aviso ofrece **Cancelar**, **Ejecutar de todos modos** y **Analizar compatibilidad**. El análisis combina comprobaciones locales de solo lectura con fuentes oficiales de OBS y clasifica el riesgo como Bajo, Medio, Alto o Desconocido. No garantiza compatibilidad.

El informe satisfactorio se guarda para la versión exacta de OBS, la versión del complemento y la arquitectura. Las consultas posteriores muestran el informe guardado sin contactar de nuevo con OpenAI. Se abre en una ventana WebView2 accesible y se copia al Portapapeles.

## Privacidad, coste y licencia

El análisis del lienzo envía a OpenAI el lienzo, la configuración regional de OBS, un prompt de seguridad fijo y preguntas válidas. El análisis de compatibilidad envía versiones, dependencias y un manifiesto fijo, pero no escenas, medios ni credenciales. Los costes de API corresponden al propietario de la clave. No hay telemetría ni publicidad.

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL versión 2 o posterior; consulte [LICENSE.txt](../LICENSE.txt). [English](../README.md).
