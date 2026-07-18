# Accessible OBS Studio 1.0

Accessible OBS Studio es un complemento de accesibilidad para OBS Studio 32 de 64 bits en Windows 10 y 11. Está pensado para usuarios ciegos de teclado y lector de pantalla, y se ha probado con JAWS y NVDA. La clave de API de OpenAI e Internet solo son necesarios para las funciones de OpenAI.

## Instalación

Instale la edición de 64 bits de OBS Studio 32.0 o posterior y ejecute `AccessibleOBSStudio-1.0-Setup.exe`. Si OBS Studio no está instalado, está dañado o es anterior a 32.0, el instalador ofrece abrir la [página oficial de descarga de OBS](https://obsproject.com/download) y se cierra sin realizar cambios. También puede actualizar una versión antigua mediante Ayuda > Buscar actualizaciones en OBS Studio. OBS Studio 32.x es compatible. Con OBS Studio 33 o posterior, el instalador advierte de una posible incompatibilidad y ofrece la [página del complemento más reciente](https://tiflo.info/aobs) antes de permitir una instalación explícita de todos modos. Si OBS Studio está en ejecución, el instalador pide cerrarlo por completo y elegir Reintentar; nunca lo cierra automáticamente. El complemento se instala en `C:\ProgramData\obs-studio\plugins\accessible-obs-studio`. Los componentes de Microsoft WebView2 y Visual C++ que falten se añaden solo después de estas comprobaciones, sin sustituir archivos de OBS ni de Qt. En la página final, la casilla **Abrir el archivo Léame en el navegador web** abre la documentación HTML en español.

## Métodos abreviados de teclado predeterminados

- F3: descripción básica del lienzo, con un máximo de 80 caracteres.
- Mayús+F3: descripción detallada.
- Alt+F3: leer el texto visible sin traducirlo ni comentarlo.
- Ctrl+F3: describir personas y fondos.
- F4: Verificación visual de la emisión o grabación para detectar problemas de diseño, cámara, iluminación, encuadre, nitidez, grano, apariencia, ropa, fondo y objetos no deseados.
- Ctrl+M: enfocar los controles multimedia visibles.
- F5, F7 y F8: iniciar o detener la emisión, grabación o cámara virtual.
- F6 / Mayús+F6: área principal siguiente / anterior.
- Ctrl+0 a Ctrl+5: lienzo, escenas, fuentes, mezclador de audio, transiciones o controles.
- Ctrl+` (tecla bajo Escape): abrir la consola de volumen accesible.

La orden **Accessible OBS Studio: Abrir el editor de métodos abreviados de teclado** abre directamente el mismo editor y no tiene una asignación predeterminada. Su identificador interno no cambia, por lo que se conserva cualquier asignación existente.

De forma predeterminada, Accessible OBS Studio obliga a que todos los métodos abreviados de teclado de OBS funcionen solo mientras OBS sea la aplicación activa. Mantiene **Configuración > Avanzado > Comportamiento del foco de los métodos abreviados** en **Deshabilitar métodos abreviados cuando la ventana principal no tenga el foco** y restaura el valor si cambia. Para devolver el control a OBS, marque y guarde **Permitir que OBS Studio gestione si los métodos abreviados de teclado funcionan fuera de OBS** en el editor. A partir de entonces, el complemento deja de intervenir.

En el primer inicio y después de cambiar de perfil, el complemento compara sus asignaciones previstas con las existentes. El diálogo solo aparece si existe un conflicto real. Puede eliminar únicamente las asignaciones en conflicto y aplicar los valores predeterminados de Accessibility, o conservar las asignaciones existentes; en ese caso, los valores predeterminados en conflicto quedarán sin asignar. **No volver a preguntar para esta versión** se aplica a todos los perfiles, pero una nueva versión o compilación vuelve a comprobarlos.

## Editor de métodos abreviados de teclado

Abra **Herramientas > Accessible OBS Studio** para abrir directamente el editor de métodos abreviados de teclado. Las flechas recorren la lista de órdenes; Tab se mueve entre la orden seleccionada, la casilla de control de métodos abreviados de OBS —desmarcada de forma predeterminada— y los botones. Intro o **Añadir o editar** abre el diálogo de asignación. Intro y Aceptar comprueban inmediatamente los duplicados. Si hay un conflicto, se identifica la otra orden: No vuelve a la entrada y Sí reasigna el método abreviado. Use **Configuración de API de OpenAI** en este editor para configurar OpenAI.

## Mezclador y controles multimedia

Ctrl+3 enfoca el mezclador normal de OBS. El complemento ya no numera sus deslizadores ni les instala un filtro global de eventos. Ctrl+` abre la consola accesible modal: Izquierda y Derecha cambian de fuente, Arriba y Abajo modifican el volumen 1 dB, Inicio establece 0 dB, Espacio silencia o reactiva y 1 a 0 seleccionan las diez primeras fuentes.

Al abrirse, la consola toma el control exclusivo de las fuentes, volúmenes y estados de silencio disponibles en ese momento. Sus cambios se aplican inmediatamente a OBS, pero durante la sesión no supervisa cambios realizados con el mezclador nativo, controladores externos, cambios de escena u otros métodos. No manipule el mezclador de otra forma mientras la consola esté abierta; ciérrela y vuelva a abrirla para cargar cambios externos.

## Descripción del lienzo

Los cinco modos capturan el lienzo renderizado por OBS. Solo la respuesta más reciente usa el rol ARIA alert agresivo; nunca se repite la pregunta del usuario. Los cinco modos admiten preguntas de seguimiento.

En la descripción básica, **Descripción detallada** siempre está disponible, **Leer texto** solo si se detectó texto, **Personas y fondos** solo si se detectaron personas y **Correcciones sugeridas** solo si un problema puede corregirse realmente de forma automática. Estas acciones reutilizan la imagen ya enviada.

El modo **Personas y fondos** da prioridad a las personas visibles y después describe su fondo inmediato. Se omiten los detalles no relacionados de la interfaz, el texto o la escena, salvo que afecten directamente a la presentación de una persona.

**Verificación visual** comprueba únicamente el aspecto de la emisión o grabación. Se mantienen las comprobaciones visuales de diseño de OBS, capturas vacías, cámara, iluminación, pantalla completa de Zoom, encuadre, desenfoque, posible suciedad de la lente, grano, apariencia, ropa, fondo y objetos no deseados. Se ignora el contenido verbal: idioma, ortografía, gramática, traducción, redacción, hechos, números, tema, tono, idoneidad, subtítulos y leyendas. El texto solo se notifica como objeto visual si es demasiado pequeño, está cortado, borroso, tiene poco contraste, queda obstruido u oculta elementos visuales importantes. Una ventana de diálogo o error solo se notifica si tapa contenido o revela un problema visible de captura o diseño, nunca por su mensaje. La corrección automática se limita a una lista fija de transformaciones reversibles de OBS. **Comprobar de nuevo** captura una imagen nueva e informa de mejoras, empeoramientos, cambios y problemas visuales restantes.

La clave de API se valida antes de guardarla, se almacena en el Administrador de credenciales de Windows y nunca se muestra. Para eliminarla se pide confirmación y se anuncia el resultado correcto.

## Privacidad y licencia

Las funciones del lienzo envían a OpenAI la imagen capturada, el idioma de OBS, instrucciones de seguridad fijas y preguntas de seguimiento. No hay telemetría ni publicidad. Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL versión 2 o posterior; consulte [LICENSE.txt](../LICENSE.txt). [English](../README.md).
