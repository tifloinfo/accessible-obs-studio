# Accessible Studio 1.1.2

Accessible Studio es un complemento de accesibilidad para OBS Studio 32 de 64 bits en Windows 10 y 11. Está pensado para usuarios ciegos de teclado y lector de pantalla, y se ha probado con JAWS y NVDA. La clave de API de OpenAI e Internet solo son necesarios para las funciones de OpenAI.

Accessible Studio es un complemento de accesibilidad independiente y de terceros para OBS Studio. No está desarrollado, afiliado, patrocinado ni respaldado por el Proyecto OBS ni por sus colaboradores. OBS y OBS Studio son marcas registradas de Wizards of OBS LLC.

## Instalación

Instale la edición de 64 bits de OBS Studio 32.0 o posterior y ejecute `AccessibleStudio-1.1.2-Setup.exe`. Si OBS Studio no está instalado, está dañado o es anterior a 32.0, el instalador ofrece abrir la [página oficial de descarga de OBS](https://obsproject.com/download) y se cierra sin realizar cambios. También puede actualizar una versión antigua mediante Ayuda > Buscar actualizaciones en OBS Studio. OBS Studio 32.x es compatible. Con OBS Studio 33 o posterior, el instalador advierte de una posible incompatibilidad y ofrece la [página del complemento más reciente](https://github.com/tifloinfo/accessible-studio/releases/latest) antes de permitir una instalación explícita de todos modos. Si OBS Studio está en ejecución, el instalador pide cerrarlo por completo y elegir Reintentar; nunca lo cierra automáticamente. El complemento se instala en `C:\ProgramData\obs-studio\plugins\accessible-studio`. Los componentes de Microsoft WebView2 y Visual C++ que falten se añaden solo después de estas comprobaciones, sin sustituir archivos de OBS ni de Qt. En la página final, la casilla **Abrir el archivo Léame en el navegador web** abre la documentación HTML en español.

Si está instalada la versión publicada Accessible OBS Studio 1.0, se desinstala automáticamente y solo se eliminan sus ajustes específicos, métodos abreviados de teclado, datos en caché y clave de API de OpenAI guardada. Los demás ajustes y contenidos de OBS se conservan.

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
- Ctrl+I: iniciar o detener Audible Meter.
- Ctrl+Mayús+D: iniciar o interrumpir Sound Doctor.

Cuando NVDA es el único lector de pantalla detectado en ejecución, el complemento anuncia expresamente el nombre localizado de la región después de cambiar correctamente con F6, Mayús+F6 o Ctrl+0 a Ctrl+5. Este anuncio adicional se suprime con JAWS, Narrador, lectores desconocidos o varios lectores detectados a la vez.

La orden **.Abrir el editor de métodos abreviados de teclado** abre directamente el mismo editor y no tiene una asignación predeterminada. Su identificador interno no cambia, por lo que se conserva cualquier asignación existente.

De forma predeterminada, Accessible Studio obliga a que todos los métodos abreviados de teclado de OBS funcionen solo mientras OBS sea la aplicación activa. Mantiene **Configuración > Avanzado > Comportamiento del foco de los métodos abreviados** en **Deshabilitar métodos abreviados cuando la ventana principal no tenga el foco** y restaura el valor si cambia. Para devolver el control a OBS, marque y guarde **Permitir que OBS Studio gestione si los métodos abreviados de teclado funcionan fuera de OBS** en el editor. A partir de entonces, el complemento deja de intervenir.

En el primer inicio y después de cambiar de perfil, el complemento compara sus asignaciones previstas con las existentes. El diálogo solo aparece si existe un conflicto real. Puede eliminar únicamente las asignaciones en conflicto y aplicar los valores predeterminados de Accessibility, o conservar las asignaciones existentes; en ese caso, los valores predeterminados en conflicto quedarán sin asignar. **No volver a preguntar para esta versión** se aplica a todos los perfiles, pero una nueva versión o compilación vuelve a comprobarlos.

## Menú de Accessible Studio

**Herramientas > Accessible Studio** abre ahora un menú accesible. **Herramientas de audio** contiene la Consola de volumen accesible, Audible Meter, Sound Doctor y **Configuración avanzada de sonido**. **Herramientas de vídeo** contiene las descripciones breve y detallada del lienzo, Leer texto, Personas y fondos, Verificación visual y la administración de claves de API de OpenAI. El menú principal también contiene el Editor de métodos abreviados de teclado y **Abrir el manual del usuario**. El manual se abre en el idioma de la interfaz de OBS si está instalado; de lo contrario, se abre en inglés.

## Editor de métodos abreviados de teclado

Abra **Herramientas > Accessible Studio > Editor de métodos abreviados de teclado**. Las flechas recorren la lista de órdenes; Tab se mueve entre la orden seleccionada, la casilla de control de métodos abreviados de OBS —desmarcada de forma predeterminada— y los botones. Intro o **Añadir o editar** abre el diálogo de asignación. Intro y Aceptar comprueban inmediatamente los duplicados. Si hay un conflicto, se identifica la otra orden: No vuelve a la entrada y Sí reasigna el método abreviado.

## Mezclador y controles multimedia

Ctrl+3 enfoca el mezclador normal de OBS. El complemento ya no numera sus deslizadores ni les instala un filtro global de eventos. Ctrl+` abre la consola accesible modal: Izquierda y Derecha cambian de fuente, Arriba y Abajo modifican el volumen 1 dB e Inicio establece 0 dB. Espacio alterna de forma segura la monitorización y la salida de programa juntas, Ctrl+Espacio alterna solo la monitorización y Mayús+Espacio alterna solo la salida. Cada fuente también tiene botones separados para la salida y la monitorización. Las teclas 1 a 0 seleccionan las diez primeras fuentes.

La consola actualiza desde OBS la lista de fuentes, el volumen, la salida y la monitorización dos veces por segundo; los cambios realizados con el mezclador nativo, controladores externos o cambios de escena aparecen durante la sesión. Sus propios cambios se aplican inmediatamente. En OBS 32.2 y posteriores, el silencio y la monitorización son independientes; en OBS 32.0 y 32.1, la consola traduce los mismos controles a los estados antiguos Solo monitorización, Monitorización y salida, y silenciado.

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

Encontrará respuestas técnicas detalladas sobre Audible Meter, la corrección automática, los tonos de la Consola y Sound Doctor en las [Preguntas frecuentes sobre las funciones de sonido](Sound-Features-FAQ.es-ES.html).

Ctrl+I inicia o detiene Audible Meter; los avisos automáticos empiezan activados. I activa o desactiva conjuntamente los avisos automáticos de entrada y salida. Mientras suena un aviso de salida, Mayús+I reduce de forma conservadora todas las fuentes responsables; nunca aumenta el volumen y limita cada corrección a 12 dB. Ctrl+Mayús+I restaura la corrección automática más reciente si el nivel no se modificó posteriormente por otro medio. Los avisos anteriores al control no se corrigen automáticamente. H anuncia el nivel actual de la última fuente enfocada en la Consola, J la fuente más fuerte y su nivel actual, K el nivel activo típico de la fuente seleccionada y L la fuente con el nivel activo típico más alto. I, Mayús+I, Ctrl+Mayús+I, H, J, K y L nunca se interceptan al escribir.

Alt+1 a Alt+9 cambian a las nueve primeras escenas en el orden mostrado; Alt+0 cambia a la décima. Las escenas posteriores a las diez primeras no tienen un atajo numérico predeterminado.

**Herramientas > Accessible Studio > Herramientas de audio > Configuración avanzada de sonido** abre un diálogo de configuración al estilo de OBS. Use la lista de categorías de la izquierda para cambiar con las flechas entre **Audible Meter** y **Sound Doctor**. La página de Audible Meter contiene los avisos de salida y la lista de exclusión. La página de Sound Doctor contiene la variación dinámica mínima, la relación máxima del compresor, el alcance de las recomendaciones de limitador y el límite recomendado. Aplicar guarda sin cerrar; Aceptar guarda y cierra; Cancelar descarta los cambios posteriores al último Aplicar.

Un aviso anterior al control sostenido tiene prioridad exclusiva. No suena ningún tono durante el primer diálogo; Sí es la opción predeterminada y solo entonces inicia el tono grave. No o Escape añade la fuente a la lista de exclusión. Si no hay señal, se anuncia a los 2 segundos, se repite 10 segundos después y, tras otros 10 segundos, aparece un diálogo con Sí como opción predeterminada. Tras una corrección correcta, el tono se detiene y se debe confirmar con Aceptar el mensaje de nivel seguro. Mientras la Consola está abierta, la detección anterior al control continúa en segundo plano; los problemas se ponen en cola y se presentan al cerrarla. Si se intenta abrir la Consola durante un ajuste, Sí continúa sin abrirla; No o Escape añade la fuente a la lista y abre la Consola.

## Sound Doctor

Ctrl+Mayús+D o **Herramientas > Accessible Studio > Herramientas de audio > Sound Doctor** inicia **Sound Doctor**. Durante la supervisión, el mismo comando abre una confirmación con Sí como opción predeterminada para interrumpir y descartar las mediciones. **No volver a mostrar este mensaje** guarda la interrupción inmediata solo si también se elige Sí. No o Escape continúa la supervisión. Sound Doctor observa las fuentes de audio activas durante al menos dos minutos y conserva únicamente mediciones de tamaño fijo en memoria; no graba ni guarda audio. Si hay una emisión o grabación en curso, el informe no aparece hasta que ambas hayan terminado.

El informe WebView2, accesible por teclado, analiza niveles típicos, picos, dinámica, posible saturación y los compresores o limitadores existentes. Cada recomendación justificada incluye una casilla **Aplicar este cambio automáticamente**, inicialmente desmarcada. **Finalizar** aplica solo los cambios marcados; Escape no aplica ninguno. Los filtros nuevos se llaman visiblemente **Sound Doctor – Compressor** o **Sound Doctor – Limiter**, incluyen una marca interna y se pueden revertir con Deshacer de OBS.

## Privacidad y licencia

Las funciones del lienzo envían a OpenAI la imagen capturada, el idioma de OBS, instrucciones de seguridad fijas y preguntas de seguimiento. No hay telemetría ni publicidad. Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL versión 2 o posterior; consulte [LICENSE.txt](../LICENSE.txt). [English](../README.md).
