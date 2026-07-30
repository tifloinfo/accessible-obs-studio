# Accessible OBS Studio 1.1, prueba de Audible Meter

Accessible OBS Studio es un complemento de accesibilidad para OBS Studio 32 de 64 bits en Windows 10 y 11. Está pensado para usuarios ciegos de teclado y lector de pantalla, y se ha probado con JAWS y NVDA. La clave de API de OpenAI e Internet solo son necesarios para las funciones de OpenAI.

## Instalación

Instale la edición de 64 bits de OBS Studio 32.0 o posterior y ejecute `AccessibleOBSStudio-1.0.7-Setup.exe`. Si OBS Studio no está instalado, está dañado o es anterior a 32.0, el instalador ofrece abrir la [página oficial de descarga de OBS](https://obsproject.com/download) y se cierra sin realizar cambios. También puede actualizar una versión antigua mediante Ayuda > Buscar actualizaciones en OBS Studio. OBS Studio 32.x es compatible. Con OBS Studio 33 o posterior, el instalador advierte de una posible incompatibilidad y ofrece la [página del complemento más reciente](https://tiflo.info/aobs) antes de permitir una instalación explícita de todos modos. Si OBS Studio está en ejecución, el instalador pide cerrarlo por completo y elegir Reintentar; nunca lo cierra automáticamente. El complemento se instala en `C:\ProgramData\obs-studio\plugins\accessible-obs-studio`. Los componentes de Microsoft WebView2 y Visual C++ que falten se añaden solo después de estas comprobaciones, sin sustituir archivos de OBS ni de Qt. En la página final, la casilla **Abrir el archivo Léame en el navegador web** abre la documentación HTML en español.

## Métodos abreviados de teclado predeterminados

- F3: descripción básica del lienzo, con un máximo de 80 caracteres.
- Mayús+F3: descripción detallada.
- Alt+F3: leer el texto visible sin traducirlo ni comentarlo.
- Ctrl+F3: describir personas y fondos.
- F4: Verificación visual de la emisión o grabación para detectar problemas de diseño, cámara, iluminación, encuadre, nitidez, grano, apariencia, ropa, fondo y objetos no deseados.
- Ctrl+M: enfocar los controles multimedia visibles.
- F5, F7 y F8: iniciar o detener la emisión, grabación o cámara virtual.
- Alt+F2: mostrar el estado de la emisión, grabación, cámara virtual y Modo Estudio.
- Alt+F7: pausar o reanudar la grabación.
- F6 / Mayús+F6: área principal siguiente / anterior.
- Ctrl+0 a Ctrl+5: lienzo, escenas, fuentes, mezclador de audio, transiciones o controles.
- Ctrl+` (tecla bajo Escape): abrir la consola de volumen accesible.

Cuando NVDA es el único lector de pantalla detectado en ejecución, el complemento anuncia expresamente el nombre localizado de la región después de cambiar correctamente con F6, Mayús+F6 o Ctrl+0 a Ctrl+5. Este anuncio adicional se suprime con JAWS, Narrador, lectores desconocidos o varios lectores detectados a la vez.

La orden **.Abrir el editor de métodos abreviados de teclado** abre directamente el mismo editor y no tiene una asignación predeterminada. Su identificador interno no cambia, por lo que se conserva cualquier asignación existente.

De forma predeterminada, Accessible OBS Studio obliga a que todos los métodos abreviados de teclado de OBS funcionen solo mientras OBS sea la aplicación activa. Mantiene **Configuración > Avanzado > Comportamiento del foco de los métodos abreviados** en **Deshabilitar métodos abreviados cuando la ventana principal no tenga el foco** y restaura el valor si cambia. Para devolver el control a OBS, marque y guarde **Permitir que OBS Studio gestione si los métodos abreviados de teclado funcionan fuera de OBS** en el editor. A partir de entonces, el complemento deja de intervenir.

En el primer inicio y después de cambiar de perfil, el complemento compara sus asignaciones previstas con las existentes. El diálogo solo aparece si existe un conflicto real. Puede eliminar únicamente las asignaciones en conflicto y aplicar los valores predeterminados de Accessibility, o conservar las asignaciones existentes; en ese caso, los valores predeterminados en conflicto quedarán sin asignar. **No volver a preguntar para esta versión** se aplica a todos los perfiles, pero una nueva versión o compilación vuelve a comprobarlos.

## Editor de métodos abreviados de teclado

Abra **Herramientas > Accessible OBS Studio** para abrir directamente el editor de métodos abreviados de teclado. Las flechas recorren la lista de órdenes; Tab se mueve entre la orden seleccionada, la casilla de control de métodos abreviados de OBS —desmarcada de forma predeterminada— y los botones. Intro o **Añadir o editar** abre el diálogo de asignación. Intro y Aceptar comprueban inmediatamente los duplicados. Si hay un conflicto, se identifica la otra orden: No vuelve a la entrada y Sí reasigna el método abreviado. Use **Configuración de API de OpenAI** en este editor para configurar OpenAI.

## Mezclador y controles multimedia

Ctrl+3 enfoca el mezclador normal de OBS. El complemento ya no numera sus deslizadores ni les instala un filtro global de eventos. Ctrl+` abre la consola accesible modal: Izquierda y Derecha cambian de fuente, Arriba y Abajo modifican el volumen 1 dB e Inicio establece 0 dB. Espacio alterna de forma segura la monitorización y la salida de programa juntas, Ctrl+Espacio alterna solo la monitorización y Mayús+Espacio alterna solo la salida. Cada fuente también tiene botones separados para la salida y la monitorización. Las teclas 1 a 0 seleccionan las diez primeras fuentes.

Al abrirse, la consola toma el control exclusivo de las fuentes, volúmenes, estados de salida y estados de monitorización disponibles en ese momento. Sus cambios se aplican inmediatamente a OBS, pero durante la sesión no supervisa cambios realizados con el mezclador nativo, controladores externos, cambios de escena u otros métodos. En OBS 32.2 y posteriores, el silencio y la monitorización son independientes; en OBS 32.0 y 32.1, la consola traduce los mismos controles a los estados antiguos Solo monitorización, Monitorización y salida, y silenciado. No manipule el mezclador de otra forma mientras la consola esté abierta; ciérrela y vuelva a abrirla para cargar cambios externos.

Cuando el foco está dentro de los controles multimedia, Izquierda y Derecha retroceden o avanzan 5 segundos. Mayús+Izquierda y Mayús+Derecha retroceden o avanzan 1 minuto; Re Pág retrocede 5 minutos y Av Pág avanza 5 minutos. Fuera de los controles multimedia, estas teclas conservan su función normal.

La consola de volumen muestra inicialmente solo las fuentes de la salida de programa actual; las fuentes exclusivas de Vista previa quedan excluidas en Modo Estudio. Active con Intro el botón no predeterminado **Mostrar todas las fuentes** para ver todas las fuentes del mezclador, con las activas primero. El mismo botón pasa a llamarse **Mostrar solo las fuentes activas**. Espacio no activa este botón de vista.

Los cambios de emisión, grabación, pausa de grabación, cámara virtual y Modo Estudio se anuncian al lector de pantalla. Alt+F2 muestra **Información de estado**, incluidos «reconectando» y «grabación en pausa». Alt+F7 pausa o reanuda una grabación.

## Descripción del lienzo

Los cinco modos capturan el lienzo renderizado por OBS. Cada nueva respuesta inicial o de seguimiento se anuncia una vez mediante una región activa ARIA asertiva; nunca se repite la pregunta del usuario. Los cinco modos admiten preguntas de seguimiento.

En la descripción básica, **Descripción detallada** siempre está disponible, **Leer texto** solo si se detectó texto, **Personas y fondos** solo si se detectaron personas y **Correcciones sugeridas** solo si un problema puede corregirse realmente de forma automática. Estas acciones reutilizan la imagen ya enviada.

El modo **Personas y fondos** da prioridad a las personas visibles y después describe su fondo inmediato. Se omiten los detalles no relacionados de la interfaz, el texto o la escena, salvo que afecten directamente a la presentación de una persona.

**Verificación visual** comprueba únicamente el aspecto de la emisión o grabación. Se mantienen las comprobaciones visuales de diseño de OBS, capturas vacías, cámara, iluminación, pantalla completa de Zoom, encuadre, desenfoque, posible suciedad de la lente, grano, apariencia, ropa, fondo y objetos no deseados. Se ignora el contenido verbal: idioma, ortografía, gramática, traducción, redacción, hechos, números, tema, tono, idoneidad, subtítulos y leyendas. El texto solo se notifica como objeto visual si es demasiado pequeño, está cortado, borroso, tiene poco contraste, queda obstruido u oculta elementos visuales importantes. Una ventana de diálogo o error solo se notifica si tapa contenido o revela un problema visible de captura o diseño, nunca por su mensaje. La corrección automática se limita a una lista fija de transformaciones reversibles de OBS. **Comprobar de nuevo** captura una imagen nueva e informa de mejoras, empeoramientos, cambios y problemas visuales restantes.

Al elegir una corrección automática, la lista solo contiene fuentes con capacidad de vídeo; si solo hay una, se selecciona automáticamente. Para contenido mal colocado se da prioridad a **Ajustar al lienzo**. Tras la aprobación se captura una imagen nueva y se comprueba únicamente si la ampliación produjo desenfoque, grano, ruido o pixelación inutilizables. Si la calidad no es aceptable o no se puede confirmar, el ajuste se deshace automáticamente y se aplica **Centrar por completo**. **Estirar a la pantalla** nunca se ofrece.

La clave de API se valida antes de guardarla, se almacena en el Administrador de credenciales de Windows y nunca se muestra. Para eliminarla se pide confirmación y se anuncia el resultado correcto.

## Audible Meter

Ctrl+I inicia o detiene Audible Meter; los avisos automáticos empiezan activados. I activa o desactiva conjuntamente los avisos automáticos de entrada y salida. H anuncia el nivel actual de la última fuente enfocada en la Consola, J la fuente más fuerte y su nivel actual, K el nivel activo típico de la fuente seleccionada y L la fuente con el nivel activo típico más alto. I, H, J, K y L nunca se interceptan al escribir.

Alt+1 a Alt+9 cambian a las nueve primeras escenas en el orden mostrado; Alt+0 cambia a la décima. Las escenas posteriores a las diez primeras no tienen un atajo numérico predeterminado.

El retraso predeterminado de los avisos automáticos es de 1,5 segundos y puede modificarse como valor decimal en la ventana Accessible OBS Studio. Los avisos de salida evalúan el nivel posterior al control de OBS. Los avisos anteriores al control se aplican solo a entradas en directo, como micrófonos, tarjetas de sonido y audio de aplicaciones, no a medios reproducidos directamente por OBS. El diálogo pregunta si se desea ajustar el nivel anterior al control. No o Escape desactiva solo esa comprobación; los avisos de salida y los tonos de la Consola siguen activos. Mientras la Consola de volumen está abierta, se suspenden todos los avisos automáticos y sus temporizadores; solo la fuente enfocada produce tonos de medición. El tono grave indica una entrada en directo en rojo antes del control; el medio indica amarillo y el agudo rojo para la fuente enfocada o un aviso de salida.

## Privacidad y licencia

Las funciones del lienzo envían a OpenAI la imagen capturada, el idioma de OBS, instrucciones de seguridad fijas y preguntas de seguimiento. No hay telemetría ni publicidad. Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL versión 2 o posterior; consulte [LICENSE.txt](../LICENSE.txt). [English](../README.md).
