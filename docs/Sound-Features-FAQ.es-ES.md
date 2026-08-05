# Preguntas frecuentes sobre las funciones de sonido

Este documento explica Sound Doctor, Audible Meter y la Consola de volumen accesible de Accessible Studio 1.1.3.

## Descripción general

### ¿Qué diferencia hay entre estas funciones?

**Audible Meter** vigila continuamente las fuentes activas y comunica problemas de nivel mediante tonos y anuncios del lector de pantalla. La **Consola de volumen accesible** permite inspeccionar y cambiar con el teclado el volumen, la salida y la monitorización de OBS. **Sound Doctor** ejecuta una sesión de diagnóstico independiente, analiza estadísticas de nivel y propone filtros opcionales de compresor y limitador. Audible Meter y Sound Doctor no pueden ejecutarse a la vez.

### ¿Graban, transmiten o conservan mi audio?

No. Solo leen valores numéricos de nivel proporcionados por OBS. No capturan la forma de onda, crean una grabación, envían audio por la red ni escriben historial de mediciones o informes en disco. Las estadísticas ocupan memoria de tamaño fijo y se descartan al detener la función. Solo se conservan preferencias normales y las identidades incluidas en la lista de exclusión anterior al control.

## Sound Doctor

### ¿Qué hace exactamente?

Inícielo con Ctrl+Mayús+D o desde **Herramientas > Accessible Studio > Herramientas de audio > Sound Doctor**. Tras confirmar, observa las fuentes activas durante 120 segundos mientras se usan normalmente. Una fuente debe aportar al menos 45 segundos de señal activa para optar a una recomendación. Si sigue activa una transmisión o grabación, la medición continúa y el informe espera hasta que ambas se detengan.

Por fuente conserva distribuciones de tamaño fijo del nivel RMS, observaciones de pico y tiempo activo, además de identidad, nivel inicial del control y huella de la cadena de filtros. También inspecciona compresores y limitadores existentes. No reconoce palabras o contenido ni envía mediciones a un servicio de IA.

### ¿Cómo decide que puede ayudar un compresor?

Solo considera tipos que probablemente sean entradas en directo, como micrófonos y capturas directas de entrada. Compara la mediana RMS activa, el pasaje habitual, con el percentil 95, el pasaje fuerte. La diferencia es la variación dinámica.

Recomienda compresión únicamente si hay 45 segundos activos, no cambiaron el control ni los filtros, es una probable entrada en directo, no existe compresor —aunque esté desactivado— y la variación alcanza el mínimo configurado.

El umbral propuesto es la mediana más 5 dB, redondeado y limitado entre −24 y −10 dB. Ataque: 6 ms; liberación: 100 ms; ganancia de salida: 0 dB. La relación es adaptativa: 2:1 por debajo de 10 dB, 2,5:1 desde 10 hasta menos de 14 dB y el máximo configurado desde 14 dB. Los valores predeterminados son 8 dB de variación mínima y 3:1 de máximo.

### ¿Distingue con certeza el habla de la música?

No. Los niveles describen volumen y dinámica, no contenido. El tipo de fuente solo es una aproximación a «entrada en directo». Un micrófono puede llevar música y una captura de aplicación puede llevar voz. Por eso el informe aconseja no seleccionar la compresión si la fuente es principalmente musical.

### ¿Cuándo recomienda un limitador?

De forma predeterminada, para toda probable entrada en directo con 45 segundos activos, sin cambios durante la prueba y sin limitador existente. No exige recorte ni picos próximos al límite: es una protección preventiva de muro contra futuros picos inesperados. Se propone al final de la cadena, con límite predeterminado de −3 dBFS y liberación de 60 ms. El sonido bajo el límite no cambia.

El ajuste avanzado puede ampliar la política a todas las fuentes suficientemente activas. Es útil como protección estricta, pero puede ser innecesario en música o medios ya masterizados.

### ¿Están relacionadas las recomendaciones de compresor y limitador?

No. Se evalúan por separado. Una fuente puede recibir ninguna, una o ambas. El compresor controla la variación dinámica; el limitador final impone un techo rígido de picos.

### ¿Qué más muestra el informe?

Muestra ajustes de compresores y limitadores existentes, avisa si un limitador no está al final, identifica picos repetidos anteriores al control cerca del recorte y explica pruebas insuficientes o cambios en la fuente. El audio recortado antes de OBS solo se corrige reduciendo la ganancia del dispositivo o aplicación.

### ¿Sound Doctor cambia algo automáticamente?

No por ejecutarse. Cada propuesta tiene inicialmente desmarcada **Aplicar este cambio automáticamente**. **Finalizar** aplica solo las seleccionadas. Antes vuelve a comprobar fuente, volumen y filtros y omite recomendaciones obsoletas. Los nuevos filtros tienen nombres visibles de Sound Doctor, se ordenan deliberadamente y admiten Deshacer y Rehacer de OBS. Escape cierra sin aplicar.

### ¿Puedo modificar la política?

Abra **Herramientas > Accessible Studio > Herramientas de audio > Configuración avanzada de sonido** y la página **Sound Doctor**:

- **Variación dinámica mínima para la compresión:** 6–15 dB; predeterminado 8 dB. Un valor menor da más recomendaciones.
- **Relación máxima recomendada:** 2,5:1, 3:1 o 3,5:1; predeterminado 3:1.
- **Fuentes aptas para limitador:** solo probables entradas en directo de forma predeterminada, o todas las fuentes activas.
- **Límite recomendado:** −12 a −1 dBFS; predeterminado −3 dBFS.

La sesión de 120 segundos y el requisito fijo de 45 segundos activos no se configuran. Aplicar guarda sin cerrar; Aceptar guarda y cierra; Cancelar descarta desde la última aplicación.

## Audible Meter y corrección automática

### ¿Qué supervisa Audible Meter?

Ctrl+I inicia o detiene su único modo sin ventana. Sigue todas las fuentes activas. Usa el pico posterior al control para avisos de salida, el pico anterior para capturas en directo aptas y resúmenes RMS para niveles habituales. Funciona haya o no transmisión o grabación.

### ¿Qué significa el pitido automático?

El tono automático alto indica que alguna fuente ha pasado el tiempo real configurado en el nivel de aviso de salida o por encima. El tiempo predeterminado es 1,5 segundos. El nivel sigue el modo de medidor de OBS: −9 dBFS con pico de muestra o −2 dBFS con pico verdadero. Descensos menores de un segundo no reinician toda la exposición; un segundo por debajo rearma el aviso. Es un aviso, no una medida exacta, y el lector anuncia la fuente más problemática.

El tono bajo significa que, después de aceptar ajustar una entrada apta, su señal anterior al control sigue en rojo. Reduzca la ganancia en el micrófono, interfaz, entrada de Windows o aplicación. El control de OBS no puede reparar esa distorsión.

### ¿Qué significan los dos tonos de la Consola?

Con Audible Meter activo y la Consola abierta, la fuente enfocada produce un tono medio en la zona amarilla de salida y uno alto en la roja. Verde, silencio, fuente no disponible o foco fuera de un control de fuente no producen tono. Son mediciones inmediatas, no avisos retardados, y I no los desactiva.

Con pico de muestra, amarillo comienza en −20 dBFS y rojo en −9 dBFS. Con pico verdadero, amarillo comienza en −13 dBFS y rojo en −2 dBFS.

### ¿Audible Meter baja el sonido por sí solo?

No. Solo Mayús+I mientras suena el aviso alto solicita expresamente una corrección. Para cada fuente responsable calcula como objetivo el nivel de aviso menos el margen de seguridad, lo compara con el pico máximo observado, redondea la reducción necesaria hacia arriba a 0,5 dB y baja el control de OBS sin superar el máximo configurado. Nunca eleva una fuente ni cambia una fuente solo por aviso anterior al control.

Los valores predeterminados son margen de 3 dB y reducción máxima de 12 dB. Es protección conservadora, no mezcla artística.

### ¿Puedo deshacerla?

Sí. Ctrl+Mayús+I restaura la última corrección de Mayús+I. Solo restaura una fuente si su control sigue en el valor automático; omite las modificadas después. Iniciar o detener el medidor, cambiar de colección o guardar ajustes nuevos borra este registro de restauración de un nivel.

### ¿Cómo modifico el comportamiento?

En la página **Audible Meter** de Configuración avanzada de sonido:

- **Nivel de aviso de salida:** −30 a 0 dBFS; predeterminado −9 dBFS con pico de muestra o −2 dBFS con pico verdadero.
- **Tiempo por encima:** 0,1–30 segundos; predeterminado 1,5 segundos.
- **Margen de seguridad de pico:** 0–12 dB; predeterminado 3 dB.
- **Reducción automática máxima:** 0,5–30 dB; predeterminado 12 dB.
- **Lista de exclusión anterior al control:** quite excepciones guardadas para volver a comprobar entradas sin cambios.

No alteran las zonas fijas de OBS, los tonos de medición, H/J/K/L ni el límite rojo anterior al control.

### ¿Puedo silenciar temporalmente los avisos?

Pulse I con Audible Meter activo. Al reactivarlos comienza una exposición nueva. Los tonos de la Consola continúan. Las letras no se interceptan al escribir en un control editable.

### ¿Para qué sirven H, J, K y L?

- H anuncia nivel y zona actuales de la última fuente enfocada en la Consola.
- J anuncia la fuente activa más alta en ese instante.
- K anuncia el nivel RMS activo habitual de la fuente seleccionada durante la sesión.
- L anuncia la fuente con el nivel RMS habitual más alto de la sesión.

### ¿Por qué tiene prioridad un aviso anterior al control?

Señala posible daño antes del control de OBS. El diálogo de decisión es silencioso. Sí inicia la guía y el tono bajo continúa mientras la entrada esté roja. Un nivel activo seguro sostenido lo resuelve. El silencio no cuenta: hay anuncios a los 2 y 12 segundos y una pregunta a los 22. No o Escape guarda la fuente sin cambios en la lista de exclusión. Si cambia su identidad o configuración se evalúa de nuevo.

### ¿Qué puedo hacer en la Consola de volumen accesible?

Ctrl+Grave la abre. Izquierda/Derecha seleccionan; Arriba/Abajo cambian 1 dB; Inicio establece 0 dB; 1–9 seleccionan las primeras nueve fuentes y 0 la décima. Espacio alterna monitorización y salida juntas, Ctrl+Espacio solo monitorización y Mayús+Espacio solo salida. Normalmente aparecen fuentes activas del programa; **Mostrar todas las fuentes** añade las inactivas. Los cambios son inmediatos y la Consola no eleva por sí sola una fuente normal por encima de 0 dB.

### ¿Cuál es el flujo más seguro?

Ejecute Sound Doctor con voz y sonido representativos, revise cada propuesta y no seleccione compresión musical salvo que busque ese efecto. Mantenga Audible Meter activo, use los tonos de la Consola para inspección inmediata, corrija avisos anteriores al control en el dispositivo o aplicación original y use Mayús+I solo cuando quiera la reducción conservadora.
