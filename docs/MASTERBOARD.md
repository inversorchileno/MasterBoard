# Master Board

Master Board es una pizarra digital y lector PDF con puntero láser estilo Drawboard PDF y una interfaz tradicional, en ventana. Es una versión modificada de OpenBoard (GPLv3). Este archivo reúne lo acordado con el dueño del proyecto, para que cualquier sesión de trabajo nueva pueda retomarlo.

## Etapas

1. **Compilación para Windows, láser nuevo y orden de las barras.** *(hecha)*
2. **Paneles de opciones estilo Drawboard y formas nuevas.** *(hecha)*
3. **Rediseño a ventana normal con menús (modo pizarra y modo documentos).** *(hecha)*
4. **Identidad propia: nombre, logo, carpeta de datos y documentación.** *(hecha)*
5. **Menús, guardar, actualización, zoom, notas de colores, idiomas y triángulos de opciones.** *(hecha, compilaciones 9 a 13)*
6. **Pestañas, Guardar/Guardar como, girar, sellos, buscar, notas rápidas, adjuntos y sonidos.** *(hecha, compilación 14)*
7. **Logo nuevo, íconos a color, scroll continuo, pantalla completa, barra que cabe en la pantalla.** *(hecha, compilación 15)*
8. **Mejoras del Word de Luis, en 5 versiones.** *(Versión 1: compilación 20; Versión 2: compilación 21; observaciones: compilación 22; Versión 3: compilaciones 24 a 26)*

## 1. Barras de herramientas

Ambas barras van en gris oscuro sólido, sin transparencia.

**Modo escritorio**, en orden: Modo pizarra, Lápiz, Borrador, Rotulador, Láser, Selección, Flecha, Rectángulo, Elipse, Subrayar, Texto. Se quitan el teclado virtual, la captura de área y la captura de pantalla.

**Modo pizarra**, en orden: Lápiz, Borrador, Rotulador, Láser, Selección, Interactuar, Mano, Zoom +, Zoom −, Línea, Texto, Flecha, Rectángulo, Elipse, Subrayar. Se quitan la captura, el teclado virtual y el imán de la cuadrícula.

## 2. Opciones de cada herramienta (estilo Drawboard)

En modo escritorio son paneles emergentes; en modo pizarra, un panel fijo a la derecha. Los paneles son fijos: sin candado, papelera ni "…".

- **Lápiz:** tipo de trazo Básico (ancho fijo), Pro (ancho variable) o Caligrafía (ancho según dirección); grosor 1–50; 9 colores más un selector de color; opacidad.
- **Goma:** Whole (borra el trazo completo) o Partial (borra por donde pasa); tamaño 1–50.
- **Rotulador:** 9 colores de resaltador más selector; grosor 20–100; opacidad.
- **Rectángulo y elipse:** borde y relleno con colores independientes; grosor; tipo de línea (continua, punteada, guiones…); borde recto u ondulado tipo nube; opacidad.
- **Flecha:** colores; grosor; tipo de línea; puntas de inicio y fin configurables (ninguna, cuadrado, círculo, rombo, flecha abierta, flecha cerrada…); opacidad.
- **Subrayar:** línea recta horizontal, 100 % opaca, trazada bajo la palabra; colores y grosor.
- **Texto:** fuente, tamaño, color, negrita y cursiva.
- **Láser:** modo Trail (estela) o Point (punto); grosor 0,5–64; colores; efecto Standard o White burn (centro blanco con halo); opacidad; tiempo de espera y duración del desvanecimiento de 100 % a 0 %.
- Los últimos ajustes de cada herramienta se recuerdan al volver a abrir el programa.

## 3. Modo pizarra en ventana normal

- Ventana normal (maximizable), con la barra de tareas de Windows visible.
- Barra de herramientas a la izquierda, panel de opciones fijo a la derecha.
- Menú superior tradicional con Pizarra, Web, Documentos, Escritorio y Configuración.
- Fila de íconos bajo el menú: Deshacer y Repetir; Página nueva, Anterior y Siguiente; Fondos y Borrar.
- Sin las pestañas laterales de miniaturas ni de biblioteca.
- En modo escritorio desaparece todo y queda solo la barra flotante.
- El modo Web recibe el mismo tratamiento: ventana normal y menú superior.

## 4. Modo Documentos en ventana normal

- Mismo menú superior que la pizarra.
- Menú nuevo "Documento" con todas las acciones de la barra inferior izquierda: Nuevo documento, Nueva carpeta, Importar, Exportar, Cambiar nombre, Favorito, Duplicar, Papelera, Abrir en la pizarra, Añadir al documento y Añadir…, con atajos de teclado e íconos.
- Fila de íconos con las acciones frecuentes: Nuevo documento, Importar, Exportar, Abrir en la pizarra y Papelera.
- Se elimina el teclado virtual flotante en todo el programa.

## Cómo quedó la Etapa 2

- Las opciones de cada herramienta se abren en una ventanita estilo Drawboard:
  - en modo escritorio, con la flechita del botón o manteniendo presionado el botón;
  - en modo pizarra, con doble clic sobre la herramienta. En la Etapa 3 pasan al panel fijo de la derecha.
- Los ajustes se guardan al cerrar el programa.
- La flecha, el rectángulo, la elipse y el subrayado se guardan como trazos (polígonos), igual que el lápiz: se pueden mover, borrar y deshacer, y cualquier versión de OpenBoard abre los documentos. Una vez dibujados no se editan como forma (no se cambia su relleno después).
- Mantener Shift dibuja cuadrados y círculos, y flechas en ángulos de 15 grados.
- Los trazos Pro y Caligrafía dependen del lápiz: con mouse el Pro se ve como el Básico.
- El color del lápiz y del rotulador reemplaza el color seleccionado de la barra inferior de la pizarra, y el grosor reemplaza el grosor seleccionado (fino, medio o grueso).

## Cómo quedó la Etapa 3

- El programa abre en una ventana maximizada, con la barra de tareas de Windows visible. El modo escritorio sigue cubriendo toda la pantalla.
- Menú superior: **Ver** (Pizarra, Web, Documentos, Mostrar escritorio), **Documento** (solo en el modo Documentos) y **Master Board** (tamaño de página, copiar y pegar, tutorial, preferencias, podcast, salir).
- Modo pizarra: barra de herramientas a la izquierda, fila de íconos bajo el menú (Deshacer y Repetir; Páginas, Anterior y Siguiente; Fondos y Borrar) y panel de opciones fijo a la derecha, que cambia con la herramienta elegida. El doble clic sobre una herramienta sigue abriendo sus opciones junto al botón.
- Modo Documentos: fila con Nuevo documento, Importar, Exportar, Abrir en la pizarra y Papelera; el resto en el menú Documento, con atajos: Ctrl+N, Ctrl+Mayús+N, Ctrl+I, F2, Ctrl+Mayús+D, Supr y Ctrl+O.
- Sin pestañas laterales (miniaturas y biblioteca) ni teclado virtual.
- La ventana y la barra vertical vienen así por defecto en la configuración de Master Board (`RunInWindow=true`, `ToolBarOrientationVertical=true`).

## Ajustes después de la Etapa 3

- Abre en modo escritorio, con la barra desplegada.
- La barra de herramientas de la pizarra va por defecto a la derecha. Al soltarla junto al borde de arriba o de abajo se vuelve horizontal; junto a la izquierda o la derecha, vertical. Master Board recuerda el borde.
- La fila de íconos está arriba (con su propio ajuste, independiente de OpenBoard) y muestra las opciones de la herramienta elegida: ya no hay panel a la derecha. Páginas, Anterior, el contador de páginas, Siguiente, Fondos y Borrar quedan a la derecha.
- En el menú, a la derecha: botón **Escritorio** y un triángulo que oculta o muestra la fila de íconos.
- Contador de páginas «1 / 200»: se escribe un número y Enter lleva a esa página.
- Barra lateral izquierda con las miniaturas de las páginas, que se oculta o muestra con un triángulo.
- Al importar un archivo (por ejemplo un PDF) se abre de inmediato en la pizarra, en la hoja 1.
- **Resaltar:** una banda horizontal semitransparente que se arrastra sobre una línea de texto, con colores, alto y opacidad. No detecta el texto del PDF: se arrastra a lo largo de la línea.
- **Nota:** un clic pone un marcador tipo cinta con un texto. Al pasar el puntero se ve el texto; al hacer clic (con la herramienta Nota, Selección o Interactuar) se puede actualizar, borrar o cambiar de color. Se guarda en el documento como `ub:note` con `ub:color`; OpenBoard normal no lo muestra.

## Compilación y publicación

- GitHub Actions compila Master Board para Windows en cada push a `master` (`.github/workflows/windows-build.yml`), salvo cuando el push solo cambia documentación (`docs/**`, `*.md`). Qt 6.8.3 MSVC, qmake (`OpenBoard.pro` y los `.pri`), librerías de `OpenBoard-ThirdParty`, QuaZip compilado en el mismo flujo.
- Cada compilación de `master` se publica como release `build-<número de ejecución>` con `LibertyBoard-windows.zip`, y ese número queda compilado en el programa (`LIBERTYBOARD_BUILD`). El menú **Acerca → Actualización** (`src/core/UBGitHubUpdater`) compara ese número con el último release, descarga el zip y lo instala con un script de PowerShell que espera a que Master Board se cierre, copia los archivos y lo vuelve a abrir.
- Las traducciones se generan con `lrelease` **antes** de `qmake`; si no, los `.qm` no se instalan y el programa queda en inglés. La compilación falla si falta `OpenBoard_es.qm`.
- Los archivos nuevos se registran en **los dos** sistemas: qmake (`src/*/*.pri`, para Windows) y CMake (`src/*/CMakeLists.txt`, para las pruebas en Linux). Los recursos nuevos van en `resources/OpenBoard.qrc`.
- Una compilación tarda unos 15 a 20 minutos. Se sigue con `gh api repos/inversorchileno/Master Board/actions/runs`.

- **Notas de cada versión:** antes de compilar se escribe `docs/releases/build-N.md` (N = número que tendrá la compilación, el siguiente `run_number` del flujo). El flujo publica la versión con esas notas y vuelve a poner en cada versión anterior las notas de su archivo, si existe. El README muestra todas las funciones: se actualiza cuando hay funciones nuevas.

## Identidad de Master Board

- **Nombre.** El programa se llama Master Board en todo lo visible: ventana, menús, mensajes, ayudas de inicio, traducciones e información del ejecutable de Windows (`LibertyBoard.exe`). El nombre interno de algunos archivos del código (`OpenBoard.pro`, `OpenBoard.qrc`, `OpenBoard_es.ts`) se mantiene para no romper la compilación ni facilitar traer mejoras de OpenBoard.
- **Logo** (desde la versión 2.0, Master Board): cuadrado azul de esquinas redondeadas con una pizarra naranja (una ola azul, una «A» roja y una estrella verde) y un puntero láser azul. Es la imagen que entregó Luis, recortada en el borde del cuadrado azul (sin el blanco de alrededor, esquinas transparentes): `docs/images/masterboard-logo.png` (1024 px). Los iconos del programa (`resources/images/OpenBoard.png`, `bigOpenBoard.png`, `ch.openboard.OpenBoard.svg`, `resources/win/MasterBoard.ico` y `MasterBoardDoc.ico`, el `.icns` de macOS y el ícono del AppImage) se generan desde ella.
- **Carpeta propia.** El nombre de aplicación y de organización es Master Board, así que los documentos y la configuración del usuario quedan en `%LOCALAPPDATA%\Master Board` y el archivo de usuario se llama `LibertyBoardUser.config`. No se lee ni se copia nada de una instalación de OpenBoard: quien instala Master Board parte de cero. La configuración de fábrica está en `resources/etc/LibertyBoard.config` y el estilo en `LibertyBoard.css`.
- **Sin actualizaciones de OpenBoard.** La búsqueda de actualizaciones está apagada y los enlaces de inicio y tutorial apuntan al repositorio de LibertyBoard.
- **Créditos.** La pestaña Acerca de muestra primero Master Board y luego indica que es una versión modificada de OpenBoard, con sus créditos y la licencia GPLv3. Los encabezados de copyright de los archivos de código se mantienen tal cual, como exige la licencia.

## Cómo quedó la Etapa 5

- **Menús** (4): **Inicio** (Nueva pizarra, Abrir/Importar, Guardar, Guardar como, Preferencias, Multipantalla, Podcast, Cerrar documento, Salir; desde la compilación 14), **Modo** (Pizarra, Web, Escritorio), **Edición** (Tamaño de página, Cortar, Copiar, Pegar, Ocultar Master Board, Dormir) y **Acerca** (Tutorial, Consejos, Master Board en GitHub, Actualización). Código en `src/gui/UBMainMenu.cpp`.
- **Guardar y Cerrar documento:** ver la Etapa 6.
- **Zoom:** recuadro fijo abajo a la derecha de la pizarra con −, % editable y + (`src/gui/UBZoomPalette`). Ya no hay zoom en la barra de herramientas.
- **Rueda del mouse en PDF:** al pasar el final de la página va a la siguiente desde arriba, y al pasar el inicio a la anterior desde abajo, con el mismo zoom (`UBBoardView::turnPageWithWheel`). No es una vista continua.
- **Orden de herramientas** (escritorio y pizarra): Lápiz, Borrador, Rotulador, Láser, Resaltar, Interactuar, Nota, Subrayar, Texto, [Línea, solo pizarra], Flecha, Rectángulo, Elipse; en pizarra al final las dos manos (Interactuar con contenidos y Mano); en escritorio al final la **captura de pantalla** (cámara), que guarda pantalla + rayas en `Imágenes\Master Board`.
- **Triángulos de opciones:** solo en modo escritorio; la pizarra no los lleva (sus opciones están en la fila de arriba y con doble clic). Todos negros con borde blanco (`src/gui/UBToolOptionsArrows`), dibujados a la resolución de la pantalla para que los iconos no se pixelen con el escalado de Windows. Apuntan hacia donde se abren las opciones: barra en la mitad izquierda de la pantalla → apunta a la derecha, abajo-derecha del icono; en la mitad derecha → apunta a la izquierda, abajo-izquierda. La ventana de opciones se abre hacia ese mismo lado. (El código también sabe dibujarlos arriba y abajo, por si la barra de escritorio se pone horizontal algún día.)
- **Opciones:** iconos estilo Drawboard en Básico/Pro/Caligrafía, Borde/Relleno, Estela/Punto y Estándar/Brillo blanco; la vista previa del lápiz cambia según el tipo de trazo; el láser muestra los títulos y valores encima de sus barras; la sección de color se llama "Color".
- **Notas de colores:** 7 colores (amarillo, rojo, naranjo, verde, azul, morado, gris), elegibles en la barra de la herramienta Nota y en la ventana de la nota. Las notas van en la capa de los dibujos, sobre el PDF, y salen en el PDF guardado.
- **Idioma:** Preferencias → Mostrar → Idioma (primera opción): idioma del sistema o uno de los 33. Se aplica al reiniciar. Todo texto nuevo de Master Board se traduce a los 33 idiomas en un commit `i18n:` aparte.
- **Preferencias** con fondo claro y letras negras; Master Board fuerza el tema claro (Qt 6.8) porque el modo oscuro de Windows dejaba texto oscuro sobre fondo oscuro. Desde la compilación 14 tienen solo las pestañas General (antes Mostrar, con idioma y presión del lápiz y del rotulador), Cuadrícula, Red, Licencias y Acerca de (con los créditos).
- **Multipantalla** apagada por defecto.
- **Portable:** el dueño usa el zip, sin instalador. Para que aparezca en programas que listan aplicaciones (como Stream Dock), se pone un acceso directo en `shell:common programs`.

## Cómo quedó la Etapa 6

- **Pestañas de documentos** (`src/gui/UBDocumentTabs`): una fila entre el menú y la barra de opciones, visible solo con 2 o más documentos abiertos. Clic cambia de documento (vuelve a la página donde estaba), la X lo cierra (pregunta si guardar cuando hubo cambios) y + abre una pizarra nueva. Un punto • junto al nombre indica cambios sin guardar. Una pizarra en blanco sin usar se reemplaza por el archivo que se abre después.
- **Abrir/Importar** (Ctrl+O) abre la ventana de Windows para elegir archivos (varios a la vez) y los abre en pestañas; ya no lleva al modo Documentos. Un archivo ya abierto se muestra en su pestaña. **Nueva pizarra** (Ctrl+N) está en Inicio.
- **Guardar** (Ctrl+S): sin preguntar, en el archivo de la pestaña: el PDF original con las anotaciones, o para una pizarra la imagen donde se guardó esa página. La primera vez en una pizarra, y con **Guardar como** (Ctrl+Mayús+S), se abre la ventana de Windows (PNG por defecto, JPEG o PDF). Se escribe primero un archivo aparte y solo al terminar reemplaza al original. Aparece «Guardado» 3 segundos a la izquierda de Buscar. El archivo de cada pestaña se recuerda mientras el programa está abierto.
- **Girar** (`src/board/UBPageRotation`): bajo el número de página, dos flechas y la palabra Girar. Gira la página con todo lo que tiene encima, un cuarto de vuelta; vertical ↔ horizontal; se puede deshacer. El PDF guardado sale con la página girada. Las pizarras nuevas siguen partiendo horizontales (decisión de Luis).
- **Pantallazo:** en escritorio, toda la pantalla menos la barra de tareas; en pizarra (botón de cámara al final de la barra), solo la pizarra, sin barras ni paletas (`src/gui/UBScreenshot`). La zona capturada se oscurece un momento, como en Windows.
- **Sello** (`src/domain/UBStamps`): herramienta después de Nota, en escritorio (con triángulo) y en pizarra (fila de sellos en la barra superior). 14 sellos tipo PDF-XChange, en inglés, o en español si Master Board está en español (no pasan por las traducciones). El último, Personalizada, pide un PNG y lo ajusta al tamaño de los demás, con margen. Se ponen como imagen: se mueven, borran y deshacen.
- **Buscar** (Ctrl+F, botón a la izquierda de Escritorio, `src/gui/UBSearchBar` y `src/pdf/UBPdfTextSearch`): busca el texto en el PDF (sin distinguir mayúsculas), marca en amarillo y lleva a cada resultado; «2 de 7» o «No encontrado». En una pizarra sin PDF avisa que solo funciona en PDF con texto. Ctrl+F dejó de ser el atajo de la herramienta Selección.
- **Nota rápida, Adjuntar archivo y Sonido** (solo pizarra, `src/domain/UBGraphicsPageMarkItem` y derivadas): íconos que mantienen su tamaño en pantalla, se mueven, se abren con un clic y se borran con Supr tras clicarlos. Se guardan en la página como `ub:comment`, `ub:attachment` y `ub:sound`; OpenBoard normal no los muestra. En el PDF guardado salen como íconos.
  - Nota rápida: globo amarillo con ventana (fecha y texto); una nota vacía se borra. Con la herramienta, la barra superior ofrece Anterior, Siguiente, Lista de comentarios, Mostrar (ocultar/mostrar todas) y Fusionar comentarios (cada nota pasa a ser una imagen fija con su fecha y texto).
  - Adjuntar archivo: clip; el archivo se copia dentro del documento (`attachments/`) y un clic lo abre con su programa (Excel, etc.).
  - Sonido: altavoz; la ventana permite grabar con el micrófono o examinar un archivo de audio, escucharlo y aceptarlo; se copia al documento (`sounds/`) y un clic lo reproduce o detiene.
- **Colores** en la barra superior: dos filas de muestras más pequeñas y sin la palabra «Color».
- **Zoom:** mostrar u ocultar la barra de opciones ya no vuelve al 100 %; se mantienen el zoom y el lugar.

## Cómo quedó la Etapa 7

- **Logo nuevo** (ver Identidad).
- **Íconos** de Sello, Nota rápida, Adjuntar archivo y Sonido con el estilo de Bolígrafo/Borrador: objetos a color con degradado y sombra (`resources/images/stylusPalette`). El ícono de **Nota** es la cinta que queda en la página, del color elegido para la próxima nota (`src/gui/UBNoteToolIcon`); cambia al cambiar el color.
- **Interactuar** (el dedo con chispa) salió de la barra de la pizarra.
- **Barras que caben:** la barra vertical (pizarra y escritorio) achica sus botones cuando la pantalla no alcanza (`UBActionPalette::fitInLength`); los triángulos se achican con ellos.
- **Pestañas:** la fila mide lo mismo que las pestañas (24 px).
- **Pantalla completa** (Edición, F11): oculta el menú y la barra de tareas; la fila de pestañas se muestra siempre en ese modo, con «Salir de pantalla completa» a la derecha.
- **Sello → Quitar:** botón junto al título; mientras está activo, un clic en un sello lo quita al instante (también en escritorio). Se reconocen los sellos puestos durante la sesión; los de una página reabierta se borran con Selección.
- **Pantallazo en pizarra:** el oscurecimiento es una capa dentro de la ventana (en Windows la ventana aparte quedaba detrás).
- **Grabar sonido:** curva del micrófono en la ventana de Sonido (`src/gui/UBSoundWave`): plana en silencio, onda al hablar.
- **Barra de desplazamiento del documento** (`src/gui/UBDocumentScrollBar`): en el borde derecho de la pizarra, abarca todo el documento hoja tras hoja; se arrastra para ir rápido a cualquier parte, con el mismo zoom. Solo con 2 o más páginas y solo mientras el puntero está cerca del borde derecho; luego se oculta. La barra de herramientas a la derecha le deja espacio.
- **Íconos con el color elegido:** Bolígrafo, Rotulador, Láser y Resaltar cambian sus partes de color al color con que dibujan (`src/gui/UBToolColorIcon`), igual que la Nota.
- **Mano** justo encima de la Nota en la barra de la pizarra.
- **Goma:** dibujos para Completo (trazo punteado que desaparece entero) y Parcial (trazo con un hueco), en pizarra y escritorio.
- **Texto en escritorio:** con Selección, un clic sobre un elemento (texto, su marco o sus botones) lo usa como en la pizarra: cerrar, cambiar tamaño, girar, mover; un clic en lo vacío sigue pasando al escritorio.
- **Scroll continuo** (`UBBoardView::turnPageWithWheel`, `drawNeighbourPages`): la hoja anterior y la siguiente se dibujan pegadas a la actual (16 px de separación); la rueda sigue a su ritmo y, cuando la vecina ocupa el centro, pasa a ser la hoja actual sin mover la imagen. Sigue siendo una hoja a la vez por dentro (no es una vista continua completa).

## Doble pizarra (compilación 16)

- **Modo → Doble pizarra** (`src/board/UBDualBoard`): dos documentos lado a lado (dos PDF, dos pizarras, o un PDF y una pizarra de notas).
- Solo un lado es la pizarra a la vez, el **activo**, con marco azul oscuro: ahí trabajan las herramientas, las páginas, guardar y la barra de desplazamiento. El otro lado (marco claro, `UBOtherBoardView`) muestra su documento como quedó y se puede desplazar; un clic en él, o en su pestaña, lo vuelve activo sin moverlo, con su zoom y lugar.
- La **línea divisoria** se arrastra: parte en 50/50 y cada lado queda entre 20 % y 80 %.
- **Dos documentos como máximo:** con uno abierto, el otro lado es una pizarra nueva (con su pestaña); con más de dos, un aviso pide elegir cuáles cerrar (el actual queda). Abrir un tercero estando en el modo pregunta lo mismo. Cerrar uno de los dos vuelve a pizarra simple; al salir del modo, el otro documento queda en las pestañas.
- Íconos en los menús nuevos (Guardar como, Doble pizarra, Pantalla completa, GitHub); todo menú nuevo lleva su ícono.
- Flecha, Rectángulo y Elipse muestran en su ícono el color de borde y el relleno elegidos (`src/gui/UBShapeToolIcon`).
- Preferencias → Paleta del lápiz: Arriba, Abajo, Izquierda o Derecha (antes Horizontal/Vertical); se puede seguir arrastrando.
- La fila de pestañas tiene la misma holgura arriba y abajo.
- **Inicio → Nueva pizarra** (antes «Nuevo documento»), rotulador con dibujo de destacador tipo Stabilo.

## Lector de PDF completo (compilación 17)

- **Reabrir y recientes** (`src/core/UBOpenedFiles`): abrir de nuevo un PDF ya trabajado (mismo archivo, sin cambios desde que se guardó) retoma su documento de Master Board, con notas, adjuntos y sonidos, sin preguntar si reemplazar. **Inicio → Archivos recientes** muestra los 10 últimos.
- **Inicio → Imprimir** (Ctrl+P, `src/board/UBDocumentPrinter`): diálogo del sistema con rango de páginas o página actual; cada página con su orientación. Ctrl+P ya no elige el bolígrafo.
- **Recuperación** (`UBDocumentTabs`): si el programa se cierra sin terminar, al abrir pregunta si recuperar los documentos que estaban en las pestañas. El autoguardado es cada minuto.
- **Páginas / Borrar página** (`src/gui/UBPageButtons`): dos botones uno sobre otro donde estaba «Páginas», con una línea punteada a su izquierda. «Borrar página» pregunta antes y no funciona con una sola página. El botón de la derecha se llama «Limpiar página»: quita todo lo agregado (el PDF queda) y con pulsación larga da más formas de borrar.
- **Inicio → Extraer páginas** (`src/board/UBPageExtractor`): página actual, todas o rangos («1-3, 5, 8-»); PDF en un archivo, PDF por página, PNG o JPEG por página; nombre, carpeta y abrir al terminar. El PDF sale con el mismo motor que Guardar (texto y giro del original, anotaciones encima). Los exportadores PDF aceptan una lista de páginas (`setPages`).
- **Búsqueda con lista** (`UBSearchBar`): con Buscar abierto, un recuadro a la derecha lista cada resultado con su página y la línea donde aparece; clic para ir. Se actualiza mientras se escribe.
- **Vista continua real** (`UBBoardView::drawNeighbourPages`): se dibujan todas las páginas que caben (hasta 8 a cada lado); con zoom pequeño la rueda avanza página a página con las demás alrededor.
- **Copiar texto del PDF**: con la flecha de selección, un recuadro donde no hay objetos copia el texto del PDF (palabras completas) al portapapeles.
- **Índice y enlaces** (`src/pdf/UBPdfNavigation`, `src/gui/UBPdfOutlinePanel`): botón **Índice** junto a Buscar con el índice del PDF; con la flecha, clic en un enlace del PDF lleva a la página, o pregunta antes de abrir una dirección web. Buscar e Índice se turnan el mismo lugar.
- **Lazo** (`UBDrawingController::isLassoSelection`): la flecha tiene opciones; el botón «Lazo» cambia a selección a mano alzada y el ícono de la paleta pasa a ser un lazo; entonces el botón ofrece «Flecha». En el escritorio la flecha sigue sin opciones.
- **Reconocer formas** (`src/domain/UBShapeRecognizer`): casilla en las opciones del bolígrafo; línea, flecha (dibujada de un trazo, con la punta al final o al principio), círculo, óvalo, triángulo, cuadrado, rectángulo y polígonos de hasta 6 lados se vuelven limpios al levantar el lápiz.
- **Pantalla táctil** (`UBBoardView::viewportEvent`): dos dedos hacen zoom y desplazan; un dedo funciona como el mouse; se ignora la palma (contacto grande) y todo toque mientras se usa el lápiz y un momento después. **No probado en Linux** (sin pantalla táctil).
- **Notas y adjuntos dentro del PDF** (`src/pdf/UBPdfAnnotations`): al guardar o extraer, las notas rápidas quedan como comentarios del PDF y los archivos y sonidos como adjuntos del PDF, en su lugar; otros lectores los muestran. Los íconos de Master Board ya no se dibujan en el PDF (sí al imprimir y en imágenes).
- **Compatibilidad con poppler:** la compilación de Windows usa poppler 23.01 (OpenBoard-ThirdParty) y Linux 24.02; lo que difiere entre versiones se resuelve con sobrecargas (ver `UBPdfNavigation.cpp`, `UBPdfTextSearch.cpp`, `UBPdfAnnotations.cpp`).
- **Español completo:** el archivo de traducción no tiene textos sin traducir (incluidos 180 heredados de OpenBoard) y la ventana de consejos tiene versión en español (`resources/startupHints/locales/es`). Otros idiomas aún tienen textos de OpenBoard sin traducir.
- **Modo → Pizarra** sale de la doble pizarra; ambos documentos quedan en sus pestañas.

## Impresión, OCR y pestañas (compilación 18)

- **Imprimir** (`src/board/UBDocumentPrinter`, clase `UBPrintDialog`): ventana propia con vista previa hoja por hoja. Impresora (las del sistema o «Archivo PDF») y botón «Ajustes de la impresora» (el diálogo del sistema para bandeja, calidad…); copias e intercalar; todas, actual o rangos («1-3, 5, 8-»); papel (Carta, Oficio, A4, A3, A5); orientación automática por página, vertical u horizontal; ajustar a la hoja o tamaño real; 1, 2 o 4 páginas por hoja; color o escala de grises; una o dos caras (si la impresora puede); imprimir o no las anotaciones. Recuerda impresora, papel, escala, color y caras.
- **OCR** (`src/pdf/UBOcrEngine*`, `src/pdf/UBPdfOcr`, `src/gui/UBOcrRunner`): **Inicio → Reconocer texto (OCR)** lee las páginas sin texto del documento (300 ppp), con avance y botón Detener; al abrir un PDF cuyas primeras páginas son solo imágenes, ofrece hacerlo (un «No» se recuerda para ese PDF). En Windows usa el lector de Windows (Windows.Media.Ocr, C++/WinRT, `windowsapp.lib`) en los idiomas del usuario; en Linux, `tesseract` si está instalado (`tesseract-ocr` y `tesseract-ocr-spa`). Las palabras quedan en `objects/<pdf>.ocr.json` junto al PDF del documento; Buscar y Copiar texto (recuadro o lazo) las usan. Al guardar/extraer, el PDF lleva el texto invisible bajo cada página escaneada (`UBPdfAnnotations::writeTextLayer`), también con la página girada.
- **Índice y Buscar** se cierran al volver a apretar su botón (o Ctrl+F).
- **Pestañas:** espacio bajo los nombres y una línea blanca continua de 1 px entre pestañas y barra de opciones.
- **Lazo:** rodear texto sin objetos también lo copia.

## Versión 1 de las mejoras del Word (compilación 20)

Luis entregó un Word con 26 mejoras («mejoras liberty.docx», con capturas de referencia) y se acordó hacerlas en versiones: 1) funciones nuevas y simples, 2) consumo medio, 3) consumo alto, 4) menús Herramientas y Personalización, atajos e IA, 5) vista continua. No se pasa a la siguiente versión hasta que Luis pruebe la anterior y lo pida.

- **Cuenta nueva:** el repositorio pasó de `noteimportaaaa` a `inversorchileno`. El actualizador, los enlaces y la documentación usan la dirección nueva; las versiones antiguas siguen encontrando las nuevas porque GitHub responde con una redirección 301 que el actualizador sigue.
- **Barra de escritorio** (`UBDesktopPalette`): soltada cerca del borde de arriba o de abajo queda horizontal (triángulos de opciones abajo o arriba de los íconos); cerca de la izquierda o la derecha, vertical. Recuerda orientación y borde. Un **triangulito rojo** en el lado que mira al borde de la pantalla la reduce al ícono comprimido con un clic (empujarla contra el borde izquierdo o derecho sigue comprimiéndola). Al final, una **papelera** que borra todo lo dibujado en la pantalla.
- **Borde en modo escritorio** (`src/desktop/UBDesktopFrame`): ventana propia, transparente a los clics, con un borde alrededor de la pantalla. Preferencias → General → «Borde en modo escritorio»: mostrar o no, color y grosor de 1 a 12 px (por defecto rojo, 1 px desde la compilación 21). Se oculta al volver a la pizarra y al sacar pantallazos.
- **Pestañas:** la franja entre el menú y las pestañas bajó de 18 a 11 px (las pestañas heredaban un margen de 6 px del estilo de OpenBoard). El espacio bajo los nombres y la línea blanca siguen igual.
- **Agregar página:** el botón «Páginas», sobre «Borrar página», se llama «Agregar página».
- **Estilo plantilla** (antes «Fondos», `UBBackgroundPalette`): ventana opaca con 3 plantillas (lisa, cuadriculada, rayada) y debajo una fila «Color de la hoja» (blanco, negro, crema, amarillo, celeste, verde, rosado, azul oscuro, verde oscuro y «…» para cualquier color). Las plantillas negras de OpenBoard ya no se ofrecen: la hoja negra se elige con el color. Cambiar de plantilla mantiene el color. Un color oscuro hace la hoja «oscura» para los trazos y la cuadrícula. Se guarda en la página como `ub:background-color` y como relleno del rectángulo de fondo; OpenBoard la abre blanca o negra.
- **Flecha cónica:** casilla «Cónica» en las opciones de la flecha (barra superior en pizarra, triángulo en escritorio). La línea crece de fina a 2,4 veces su grosor en la punta, con una cabeza mayor, en un solo contorno.
- **Láser arcoíris:** la estela toma los colores del arcoíris a lo largo de su recorrido, con su brillo; en modo Punto los colores cambian con el tiempo. Desde las observaciones a la V2 es una tercera opción del efecto (Estándar, Quemado blanco, Arcoíris) y no una casilla aparte.
- **Consejos y trucos:** la primera página muestra el logo de Master Board en vez del globo de OpenBoard.
- **Rotulador:** «Punta circular» o «Punta recta», con dibujo. Con la recta, al levantar el lápiz el trazo se rehace como con la punta biselada de un destacador: ancho en líneas horizontales, más fino en las verticales, con extremos rectos.

## Versión 2 de las mejoras del Word (compilación 21)

- **Cinco plumas como en Samsung Notes** (aprobadas por Luis con su captura de referencia), en lugar de Básico/Pro/Caligrafía; los íconos son dibujos de la pluma en el color elegido (`penStyleIcon`), en botones más chicos en la barra de la pizarra. Se guardan con los mismos valores de `UBPenStrokeType` (0 a 2 como antes, 3 y 4 nuevos):
  - **Lápiz pasta:** más grueso lento y más fino rápido (velocidad suavizada en `UBGraphicsScene::updateSpeedFactor`), algo más fino al subir a la derecha, y la presión.
  - **Pluma fuente:** plumilla plana inclinada hacia arriba a la derecha; al levantar el lápiz el trazo se rehace con esa plumilla (`UBGraphicsStroke::redrawnWithChiselTip`), con extremos cortados en diagonal.
  - **Pluma caligráfica:** lisa y pareja, con la presión.
  - **Lápiz grafito:** fino, más oscuro que el color, con grano; **Marcador:** ancho, con textura de fieltro. Ambos se rehacen al levantar el lápiz como hebras translúcidas ásperas (`UBGraphicsStroke::redrawnTextured`).
- **Colores en círculos** en tres filas en la barra de la pizarra (dibujados como íconos, con anillo blanco en el elegido); en escritorio quedan igual. **Cuentagotas** al final de toda fila de colores, en ambos modos (`src/gui/UBEyedropper`: foto de la pantalla con lupa; clic toma el color, Esc cancela).
- **Resaltar → «Ajustar al tamaño de letra»:** en un PDF la banda toma el alto de la línea de texto (palabras del PDF o del OCR, `UBPdfTextSearch::wordsIn`) con 2 puntos de margen.
- **Bloquear lienzo** (candado, a la izquierda de Índice, `UBDrawingController::setCanvasLocked`): sin rayar, escribir, resaltar, Limpiar página, Cortar ni Pegar; solo láser, mano, foco y leer/dictar; la flecha de selección no hace nada (desde las observaciones a la V2). El escritorio no se bloquea.
- **Edición → Tamaño página/lienzo → Tamaño personalizado...:** siempre disponible; ancho y alto en píxeles, con botones para agrandar el lienzo una vez y media; se puede deshacer.
- **Láser «Seguimiento»** (como Repen): sigue al mouse sin clic y cada parte de la estela se desvanece con los tiempos del láser. Cada capa del brillo se dibuja opaca en una imagen aparte y se pone translúcida de una vez, para que no queden puntos donde se juntan los tramos.
- **Foco y lupa** (herramienta nueva tras el láser, en ambos modos, `src/gui/UBSpotlight`): el foco oscurece todo menos un círculo (radio 150 px y oscuridad 75 % por defecto); la lupa amplía bajo el puntero (200 % por defecto). Es una ventana transparente al mouse; en Windows queda fuera de las capturas de pantalla (`SetWindowDisplayAffinity`) para que la lupa vea lo que hay debajo. En Linux necesita un compositor (`xcompmgr`) para verse.
- **Hipervínculo:** la herramienta «Insertar archivo» (adjuntar) tiene Archivo o Hipervínculo; con Hipervínculo un clic pide la dirección y deja un ícono de cadena (`UBGraphicsHyperlinkItem`, `<ub:hyperlink xlink:href>`) que abre el navegador. En escritorio la herramienta está en la barra y solo pone hipervínculos. En el PDF guardado es un comentario con la dirección.
- **Edición → Leer y dictar:** «Leer en voz alta» (Ctrl+Mayús+L, Qt Text to Speech: voces de Windows) lee los textos seleccionados o el texto de la página; «Dictar (Windows + H)» abre el dictado de Windows; «Transcribir un audio» queda apagado hasta la IA de la Versión 4. La compilación de Windows instala el módulo `qtspeech`.
- **Ajustes pedidos con la V2:** en la barra de la pizarra la flecha muestra un solo ícono por grupo (estilo de línea, inicio, fin) que despliega el resto, con «Cónica» y la opacidad a la izquierda; las barras de tiempo del láser ocupan la mitad («Espera», «Desvanecer»); el borde del modo escritorio parte en 1 px.

## Observaciones a la Versión 2 (compilación 22)

Luis revisó la compilación 21 y entregó «observaciones_V2.docx» con 15 puntos. Los aprobó y se publicaron en la compilación 22; la V3 espera su autorización.

1. **Bloquear lienzo:** la flecha de selección ya no selecciona nada (`UBBoardView`: con el lienzo bloqueado sale antes de presionar, mover y soltar).
2. **Foco y lupa:** el ícono de la herramienta pasa a una lupa cuando se elige Lupa (`UBSpotlightOptionsPanel::updateToolIcon`, `magnifier.svg`).
3. **Láser seguimiento:** la estela leía color y efecto solo al empezar, por eso seguía en arcoíris; ahora los toma en cada movimiento (`UBGraphicsLaserTrail::takeSettings`).
4. **Movimiento rápido:** la estela de seguimiento se suaviza con curvas entre los puntos (`smoothFollowing`), sin quiebres.
5. **Barra «Espera» en 5 s:** el tirador quedaba cortado; las barras de tiempo tienen ancho fijo (`UBValueSlider::setSliderWidth`).
6. **Círculos de color:** el doble de grandes (el estilo general de la aplicación les daba un margen de 4 px que los achicaba), con menos espacio entre ellos, en la misma superficie.
7. **Foco y lupa:** el clic en el ícono de la barra solo elige la herramienta y muestra sus opciones; los botones Foco y Lupa de las opciones prenden, cambian y apagan (un clic en el que está prendido lo apaga). Cambiar de herramienta los apaga (`UBDrawingController::setSpotlightShown`, `UBSpotlightOptionsPanel::onModeClicked`). En pizarra y escritorio.
8. **Flecha cónica** con línea punteada o de trazos: cada trazo es un trapecio con su grosor, con extremos redondos.
9. **Un color más:** 13 colores (se sumaron blanco, gris, café y azul marino) más la rueda de colores y el cuentagotas = 15 lugares.
10. **Arcoíris** es la tercera opción del efecto del láser, junto a Estándar y Quemado blanco.
11. **Barra de escritorio:** vuelve a su grosor; el triangulito rojo se dibuja en una franja fuera de la barra.
12. y 14. **Herramienta «Leer y dictar»** (`UBStylusTool::Voice`, solo en la pizarra, tras Texto): Leer o Dictar, y el ícono cambia según lo elegido. Leer: «Desde el inicio» (lee la página y sigue con las siguientes) o «Texto seleccionado» (se arrastra un recuadro sobre el texto); botones Reproducir, Pausar, Detener y Siguiente hoja (`UBReadAloud`). Dictar: botón que abre el dictado de Windows.
13. **Dictado en español de Chile:** el dictado de Windows no acepta el teclado es-CL; al dictar, si el teclado es de otro español que no sea España ni México, se cambia a español de México para esta aplicación (`UBPlatformUtils::startDictation`). Sin probar en Windows.
15. **Rotulador:** usa los mismos 13 colores que la pluma.

## Versión 3 de las mejoras del Word (compilaciones 24 a 26)

- **Plantillas** (#3b, `src/domain/UBPageTemplates`): Estilo plantilla tiene dos pestañas, **Hoja** (las tramas y el tamaño de la cuadrícula) y **Plantillas**; el color de la hoja sirve para ambas. 21 plantillas aprobadas por Luis, dibujadas por Master Board (las de internet suelen tener derechos de autor): lista de tareas, lista con viñetas, planificador diario, calendario semanal (con y sin horas), calendario mensual, planificador anual, lista de compras, seguimiento de hábitos, notas Cornell, hoja de reunión, hoja de puntos, pauta de caligrafía, papel milimetrado, pentagrama, plan de clase, lista de asistencia, planilla de notas, tabla de 2 columnas, mapa mental y guion gráfico.
  - Grises medio transparentes sobre fondo transparente: se ven en cualquier color de hoja.
  - Los calendarios se arman para la «Fecha de los calendarios» (hoy por defecto): los días caen en su día real cada año; la semana empieza el primer día de la semana del idioma.
  - Textos de las plantillas en español, inglés, alemán, francés, chino y japonés; los demás idiomas, en inglés (pedido de Luis). No pasan por las traducciones.
  - Es un SVG detrás de la página, marcado en su descripción; elegir una trama lo quita. En una página de PDF o con una imagen detrás, la plantilla va en una página nueva después.
- **Formas** (#13, `UBShapeKind`, `UBShapeGeometry::outline`): Rectángulo y Elipse se unieron en la herramienta **Formas** con 20 formas: línea, polilínea, elipse, rectángulo, paralelogramo, triángulo, triángulo rectángulo, rombo, pentágono, hexágono, flechas de bloque (derecha, izquierda, arriba, abajo), estrellas de 4, 5 y 6 puntas, globos de diálogo rectangular y ovalado, y nube. Polilínea: un clic por esquina, cualquier tecla termina (menos Mayús, Ctrl y Alt), Esc la borra para partir de cero. En la barra: las formas a la izquierda en 3 filas, luego colores, Borde/Relleno en una columna, grosor y opacidad con el número sobre la barra. El ícono es la forma elegida. La elipse salió de las barras.
- **Listas** (#5, `UBGraphicsTextItem::toggleList`): casillas para marcar, viñetas y números, en la barra de Texto y en la barra de cada cuadro de texto. Un clic marca la casilla; se guarda en el HTML del texto.
- **Tablas** (#6, `src/domain/UBTextTables`, `src/gui/UBTextBoxes`): Tabla en las opciones de Texto pregunta filas y columnas (2 × 3) y la pone en el cuadro que se escribe o en uno nuevo. Tab pasa a la celda siguiente (agrega fila al final), Mayús+Tab vuelve. Se arrastran las líneas para ensanchar columnas y agrandar filas. Botón de tabla en el cuadro: añadir/eliminar fila o columna, columnas del mismo ancho, estilo (destacar encabezado o primera columna, líneas: todas, horizontales, exteriores o ninguna, color de celdas) y eliminar tabla. Es una tabla del HTML del texto: OpenBoard la muestra.
- **Mientras se escribe en un cuadro de texto**, la barra muestra las opciones de Texto (la herramienta pasa a Selección al crear el cuadro).
- **Emojis y GIFs** (#17, `UBStylusTool::Emoji`, `src/domain/UBEmojis`, `src/gui/UBGifWindow`): herramienta nueva en ambas barras (con triángulo en escritorio). Emojis: 72, en 3 filas; clic elige, clic en la página lo pone como imagen a color, del tamaño elegido. GIFs: sin cuenta (decisión de Luis), el botón GIFs abre la página de klipy.com en una ventana angosta a la derecha, con su buscador; se arrastra un GIF a la página. La ✕ arriba a la derecha o un segundo clic en GIFs la cierran.
- **GIF animados** (`UBGraphicsPixmapItem::setAnimation`): una imagen de varios cuadros se mueve en la pizarra, también arrastrada desde un navegador y al reabrir la página (el GIF se guarda tal cual). Al guardar como imagen o PDF, o imprimir, queda el cuadro visible.

### Ajustes pedidos durante la V3

- **Leer en voz alta** (antes Leer y dictar) solo lee: «Desde el inicio», «Texto seleccionado» (el recuadro se marca en amarillo mientras se lee) y **«Desde el clic»** (lee desde la palabra clicada y sigue con las páginas siguientes). Todos los botones son cuadros con dibujo arriba y nombre abajo.
- **Dictar** pasó a Texto (`src/gui/UBDictation`): botón Dictar en las opciones de Texto (dicta en el cuadro que se escribe o crea uno en el centro) y micrófono a la izquierda de la F en cada cuadro. El cuadro que se escribe tapa casi por completo la página (92 % opaco).
- **Foco y lupa:** el ícono de la barra solo elige la herramienta; los botones Foco y Lupa de las opciones prenden, cambian y apagan.
- **Láser:** las barras de opacidad, espera y desvanecer del mismo ancho, con el nombre arriba y el número al lado.
- **Colores:** círculos de 20 px y exactamente 3 filas en todas las herramientas; la Nota ofrece los 13 colores de la pluma con la rueda y el cuentagotas.
- **Guardar/Descartar/Cancelar** en el idioma del programa en Windows: se carga `qtbase_<idioma>.qm` (el `qt_<idioma>.qm` no carga si falta algún módulo de Qt).
- **Compilación 25** (pruebas de Luis): un GIF arrastrado desde klipy.com da la dirección de su página; Master Board abre la página y descarga el GIF o WebP que comparte (`UBGifWindow::findGif`). Las tablas nacen con líneas negras de 1,5 puntos (antes grises de 1 punto, casi invisibles) y Estilo de tabla ofrece color y grosor de las líneas. Flecha/Lazo y Archivo/Hipervínculo son cuadros con dibujo arriba y nombre abajo. La línea recta salió de la barra (está en Formas).
- **Compilación 26:** el GIF arrastrado se busca en la ventana de klipy.com que lo muestra (la página de cada GIF se arma al ejecutarse, así que descargada no lo trae): se prueba primero la versión GIF de cada dirección, luego WebP. Las formas del escritorio ocupan todo el ancho (2 filas de 10, más grandes). En un PDF el candado dice «Bloquear PDF» / «Desbloquear PDF».
- **Traducciones:** desde la V3 los textos nuevos se traducen al español, alemán, francés, chino y japonés; los demás idiomas los muestran en inglés (pedido de Luis).

- **Después de la compilación 26** (sin compilar aún): la ventana de klipy.com recuerda las cookies (perfil web propio en `webprofiles/gifs`, antes preguntaba cada vez); las pestañas de documentos se reordenan arrastrándolas; el sello propio (hoy «Tu PNG») es el primero y parte vacío en cada sesión.

## Versión 4 de las mejoras del Word (sin compilar aún)

- **Menú Herramientas** (#29a, `src/gui/UBToolbarChoice`): todas las herramientas de la pizarra y del escritorio, con su atajo; sirve también para las que se quitaron de la barra.
- **Menú Personalización** (#29b): submenús **Escritorio** y **Pizarra / PDF**, con una casilla por botón de la barra. Al marcar o desmarcar, el menú no se cierra; lo elegido se guarda (`hiddenBoardTools`, `hiddenDesktopTools`). Por defecto se ve todo, como antes. El botón del escritorio que vuelve a la pizarra no se puede quitar.
- **Acerca → Atajos** (#30, `src/gui/UBShortcutsDialog`), debajo de Abrir tutorial: la lista de todas las funciones (menús, botones junto a los menús, herramientas de pizarra y escritorio, páginas) con su atajo y un buscador. Se elige una función, se presionan las teclas en el cuadro y **Asignar**; también **Quitar**, **Original** y **Restaurar todos**. Se guarda en `LibertyBoardShortcuts/<objectName>` de la configuración.
  - Las funciones sin atajo recibieron uno (sobre todo Ctrl+Mayús+letra y teclas F): Bolígrafo Ctrl+Mayús+P, Foco F, Resaltar I, Selector E, Mano M, Nota O, Sello T, Emojis G, Nota rápida Q, Adjuntar J, Sonido K, Subrayar U, Leer en voz alta V, Flecha A, Formas R, captura C (escritorio X), Bloquear lienzo B, OCR Y; Tutorial F1, Atajos Ctrl+F1, Consejos Mayús+F1, Índice F7, Dictar F8, Pizarra Ctrl+1, Doble pizarra Ctrl+2, Nueva página Ctrl+Intro, Duplicar página Ctrl+Mayús+Intro, entre otros. No se usa Ctrl+Alt (es Alt Gr en los teclados en español) ni Alt+Mayús (cambia el idioma del teclado en Windows).
  - Se rechaza con un aviso que explica por qué: la tecla Windows, las combinaciones de Windows (Alt+Tab, Alt+F4, Alt+Esc, Alt+Espacio, Ctrl+Esc, Ctrl+Mayús+Esc, Ctrl+Alt+Supr, Impr Pant, Mayús+F10, Ctrl+Alt+flechas que giran la pantalla), las teclas solas que sirven para escribir o moverse, Ctrl+Alt con letra (Alt Gr), Ctrl+A, Alt + la letra de un menú y un atajo que ya tiene otra función.
  - Al final, las **teclas fijas** de OpenBoard (páginas con RePág/AvPág y flechas, zoom con Ctrl+ +/−, desplazamiento con Ctrl+flechas), que no se cambian.
  - Los atajos de las herramientas funcionan también con la herramienta quitada de la barra. En el modo escritorio no se pudieron probar en Linux (la capa transparente no recibe el teclado en la prueba).

- **Panel de IA** (#32, `src/gui/UBAiPanel`): botón **IA** (estrellas) en la barra de la pizarra, atajo F9. Pregunta la acción: Resumen, Traducción (pide idioma de origen, o «Detectarlo», y de destino), Crear imagen (pide la descripción), Quitar fondo, Borrado generativo (pide qué borrar) o Transcribir un audio (pide el archivo); luego si se da **una parte de la página** (se arrastra un rectángulo, Esc cancela) o **todo el PDF / la pizarra** (o la página entera, para las de imagen). Lo exporta con las anotaciones (`<datos>/ai/LibertyBoard-selection.png`, `-page.png` o `-document.pdf`) y abre a la derecha la página de la IA, con perfil web propio (`webprofiles/ai`, guarda la sesión; el navegador no se presenta como QtWebEngine para que dejen iniciar sesión).
  - La instrucción para la IA la arma Master Board (en el idioma del programa), se escribe sola en la caja de la IA y queda copiada; el archivo se entrega a la página como si se eligiera o pegara (hasta 20 MB). Si la página no lo permite: Ctrl+V y arrastrar el archivo desde el panel. Depende de cómo esté hecha cada página: puede dejar de funcionar si la cambian.
  - Lista de IA arriba del panel; el borde izquierdo se arrastra para ensanchar. Si después de dar la instrucción la página dice que se alcanzó un límite («You've reached...», «Has alcanzado el límite»...), aviso con botones para hacer lo mismo en otra IA.
  - El resultado vuelve a la página arrastrándolo (las imágenes se toman como imagen, no como dirección, porque esa dirección exige la sesión), con **Pegar en la página**, o solo si la IA lo hace descargar: queda en Descargas y una imagen se pone en la página.
  - Preferencias, pestaña **IA**: nombre y dirección de cada IA (por defecto ChatGPT, Claude, Gemini, Copilot, DeepSeek, Grok, Le Chat y Perplexity; cada uno puede agregar más), Agregar, Quitar y Lista original.
  - **Transcribir un audio** también en Edición → Leer y dictar (Ctrl+F11) y junto a Dictar en las opciones de Texto.
  - Probado en Linux con una página local que imita un chat (las páginas de las IA no se alcanzan desde la máquina de pruebas): instrucción escrita, PDF y PNG adjuntados, aviso de límite y arrastre de una imagen a la página. Falta probarlo con las IA reales en Windows.

### Ajustes pedidos después de la compilación 27

- **Sello «Tu PNG»** (antes «PNG personalizado»; en inglés «Your PNG»): el «+» arriba y «TU PNG» en una línea debajo, al tamaño máximo que cabe.
- **Personalización:** cada línea tiene el ícono del botón a la izquierda, su nombre y la casilla a la derecha; un clic en el nombre o el ícono también marca. Nueva opción **Mover elementos...** (`UBToolbarChoice::openOrderWindow`): pestañas Pizarra / PDF y Escritorio con solo las herramientas visibles, se arrastran o se usan Subir / Bajar / Orden original; las ocultas guardan su lugar. En el escritorio el botón que vuelve a la pizarra queda siempre primero. Se guarda en `boardToolOrder` / `desktopToolOrder` (`UBActionPalette::setOrder`). Lo elegido en Personalización se escribe en disco al instante.
- **IA como herramienta** (`UBStylusTool::Ai`, `actionAi`, `UBAiOptionsPanel`): en la pizarra muestra en la barra de opciones un cuadro con ícono propio por acción (Resumen, Traducción, Crear imagen, Quitar fondo, Borrado generativo, Transcribir, Abrir la IA). En el escritorio está en la barra y su botón abre esos cuadros (3 por fila) sin Transcribir; Resumen, Traducción, Quitar fondo y Borrado piden una zona de la pantalla (sobre una foto fija de la pantalla); Crear imagen solo pide qué crear. En el escritorio el panel va dentro de la capa que cubre el escritorio (si fuera una ventana aparte no recibiría los clics) y lo que trae la IA queda sobre el escritorio, movible y de tamaño ajustable.
- **Traer el resultado:** clic derecho sobre una imagen o un texto seleccionado de la IA → «Poner la imagen/el texto en la página» (o en el escritorio); la imagen la copia la propia página. Arrastrar al escritorio no funcionó en la pantalla de prueba de Linux.
- **Pruebas de Luis con ChatGPT (Word «Mejoras IA»):** el PDF entero va al campo de archivos que acepta su tipo (antes caía en el de imágenes y ChatGPT lo rechazaba); el mensaje se envía solo, apretando el botón de enviar cuando la página lo habilita (es decir, cuando el archivo ya subió) o Enter si no lo encuentra; una imagen arrastrada desde la IA con dirección `blob:` se lee dentro de la página de la IA, y una con dirección de la IA se descarga con su sesión, y se pone donde se soltó (antes daba «Fallo al descargar contenido blob:...»). El panel va pegado al borde contrario a la barra de herramientas (a la izquierda si la barra está vertical a la derecha; si no, a la derecha), con la altura de la pizarra, sin tapar la barra de opciones, el zoom ni una barra horizontal.
- **Nombres dobles en Personalización y Mover elementos:** Foco / Lupa, Adjuntar archivo / Hipervínculo, Selector: flecha / lazo (Emojis y GIFs ya lo tenía).
- **Menú Acerca:** sin «Abrir tutorial». **Consejos y trucos** (F1) es ahora una guía de 19 páginas con capturas de cada herramienta, en español, inglés, alemán, francés, japonés y chino (los demás idiomas, en inglés). Las páginas se generan con `tools/guide/gen.py` desde los textos de `tools/guide/pages_*.py`; las imágenes están en `resources/startupHints/assets/guide` (tomadas en español). La ventana de consejos mide 1000 × 760. **Creador** (Ctrl+Mayús+F2): texto aprobado por Luis sobre quién hizo Master Board y cómo, traducido a los 33 idiomas; el enlace a su video se agregará cuando lo publique.
- **Después de la compilación 28:** arreglo de la barra de páginas (al arrastrarla, cada valor centraba la vista en el centro leído de la vista, que redondea medio píxel siempre hacia el mismo lado: la página se corría a la derecha hasta salir de la pantalla; ahora solo cambia la altura y se aplica el último valor cada 30 ms). La ventana de GIFs se ubica como el panel de IA (`UBAiPanel::sidePlace`), también dentro de la capa del escritorio. Edición sin «Leer y dictar» (queda Ctrl+Mayús+L). Creador sin el párrafo de la licencia. Preferencias → Acerca de: descripción de Master Board, actualización del software y los botones «Traductores» y «Licencias», que despliegan los créditos y los avisos legales (GPLv3, copyright de OpenBoard y de las bibliotecas de terceros). README con todas las funciones y notas de cada versión en `docs/releases`.

## macOS y Linux (compilación 30)

- `.github/workflows/linux-macos-build.yml` compila en cada push (salvo cambios solo de documentos) y, en `master`, adjunta los archivos a la release que publica la compilación de Windows del mismo commit (`attach-to-release.sh`).
- **Linux:** Ubuntu 24.04, AppImage con linuxdeploy (`LIBERTYBOARD_RELOCATABLE`), y se comprueba que abre.
- **macOS:** dos trabajos, `apple-silicon` (macos-15, Qt y bibliotecas de Homebrew) e `intel` (macos-15-intel, Qt oficial 6.8.3 con aqtinstall y bibliotecas de conda-forge, porque Homebrew ya no trae paquetes listos para Intel y los compila durante horas). En los dos se compilan QuaZip 1.4 y **poppler 24.02** (la versión de Linux: la de Homebrew, 26, tiene otra interfaz y pide C++20). `fix-macos-libraries.sh` lleva dentro de la app las bibliotecas que macdeployqt deja apuntando a Homebrew (el ayudante de QtWebEngine), y un paso falla la compilación si alguna sigue apuntando a carpetas de la máquina. FFmpeg 8 ya no tiene `AVCodec::sample_fmts` (se usa `avcodec_get_supported_config`).
- Sin firma de Apple ni de Microsoft: la primera vez cada sistema muestra un aviso, explicado en el README.

## Nuevo nombre: Master Board (versión 2.0)

- Pedido por Luis después de la compilación 30: el programa se llama **Master Board** en todo lo que se ve (título, menús, mensajes en los 33 idiomas, guía, README, notas de las versiones y títulos de las releases, que la compilación de Windows renombra) y tendrá un logo nuevo.
- **Se mantienen a propósito los nombres internos**, que no se ven: `setOrganizationName`/`setApplicationName("LibertyBoard")` deciden la carpeta de los documentos y de la configuración (cambiarlos haría parecer perdidos los documentos de Luis); las claves de configuración `LibertyBoard...`, los perfiles web `LibertyBoardAi`/`LibertyBoardGifs` (sesiones de las IA y de Klipy), la marca de las plantillas (`LibertyBoard page template`), el identificador de macOS y la dirección del repositorio. El nombre mostrado viene de `setApplicationDisplayName("Master Board")`.
- Archivos: `MasterBoard.exe`, `MasterBoard-windows.zip`, `MasterBoard.app` en `MasterBoard-macos-*.dmg`, `MasterBoard-linux-x86_64.AppImage`. La compilación 31 traía además una copia `LibertyBoard.exe` para los accesos directos antiguos; desde la 32 no se incluye (Luis parte de cero) y Master Board borra la que quede en su carpeta al iniciar.
- Las notas antiguas ya no nombran los archivos de entonces (`.zip` de Windows, `.dmg`, `.AppImage`).

## Versión 2.0 (mejoras finales, sin compilar aún)

Pedidas en el Word «mejoras_finales».

- **Versión 2.0** (`version.txt`): Acerca → Actualización empieza siempre con «Estás usando Master Board 2.0 (compilación N).», haya o no una nueva; Preferencias → Acerca de también la muestra. La actualización compara el número de compilación, no la versión.
- **Preferencias como PDF-XChange:** categorías con ícono a la izquierda (General, Apariencia, Lápiz y barra de herramientas, Pantallas y escritorio, Documentos y PDF, IA, Red, Actualización, Acerca de) y su página a la derecha, con título y secciones que se desplazan (`UBPreferencesController::buildCategories`, sobre los widgets de `preferences.ui`). Aceptar, Aplicar y Cancelar: Cancelar, Esc y la X devuelven los ajustes de cuando se abrió la ventana o del último Aplicar (instantánea de todos los `UBSetting`). La sección de la posición de la barra de herramientas se llama «Barra de herramientas» y la de la barra de arriba «Barra de opciones». Sin pestañas Traductores ni Cuadrícula; las licencias quedan tras el botón **Referencias**.
- **Tema** (Apariencia): oscuro (por defecto), blanco, gris (claro con letras negras) o igual que Windows/el sistema. `UBTheme` cambia los colores de las hojas de estilo oscuras y de los íconos dibujados de la barra de menú, la barra de opciones, las pestañas de documentos, la página actual y el zoom; se aplica al volver a abrir LibertyBoard.
- **Color de las líneas** de la cuadrícula, del rayado y de los puntos, con su opacidad, en Estilo plantilla → Hoja (para hoja clara u oscura, según la página).
- **Personalización → Estilo barra** (`UBBarStyle`): color interior, color del borde, opacidad y esquinas de cuadradas a redondas de las barras de herramientas de la pizarra y del escritorio, al instante; Cancelar vuelve atrás y Restaurar pone el gris de siempre.
- **Mover elementos** con **Aplicar**: el orden se ve al instante sin cerrar; Cancelar después de Aplicar vuelve al orden de cuando se abrió.
- **Podcast y Multipantalla** salen de Inicio: van al final del menú Herramientas y como botones arriba a la derecha, a la izquierda de Bloquear lienzo; no se ofrecen en las barras.
- **Consejos y trucos** en una ventana propia (sin el marco gris de OpenBoard): barra de título con X, la página a la izquierda y el **índice** de las páginas a la derecha; 20 páginas, con una nueva de Mediciones.
- **Mediciones** (`UBMeasureTool`, `UBStylusTool::Measure`, `actionMeasure`, Ctrl+F5; Ctrl+Mayús+D ya era Duplicar), en la pizarra y en el escritorio: Distancia, Perímetro y Área. La primera vez la ventana pide la unidad (mm, cm, m, km, in, ft, yd, mi) y una línea de 1 unidad; luego Distancia se arrastra, Perímetro y Área se marcan con un clic en cada esquina y doble clic (o Enter) en la última. La ventana se mueve, tiene Reiniciar y X; elegir de nuevo la opción activa también cierra. Con otra herramienta la ventana conserva la última medida y las líneas se van. En la pizarra la referencia está en coordenadas de la página (el zoom no cambia las medidas). En el escritorio la unidad se elige dentro de la ventana (un diálogo aparte no recibe los clics) y la capa de las líneas queda bajo la barra. Probado: 3,00 cm, 4,00 cm² y 8,00 cm con una referencia de 100 px, también con zoom.

## Últimas mejoras (V2.32)

Pedidas en el documento «ultimas_mejoras_ahora_si».

- **Versión V2.N:** `UBGitHubUpdater::versionText` da «V» + versión mayor (`version.txt`) + «.» + número de compilación (V2.31, V2.32…), en la actualización, las Preferencias, los títulos de las releases (la compilación de Windows los pone: V1.N hasta la 30, V2.N desde la 31) y las notas.
- **Barras de opciones en columnas:** en la barra de arriba cada `UBValueSlider` es una columna (nombre, valor, barra) mediante `setStacked`; el título de sección que la precede pasa a ser su nombre (propiedad `captioned` para que no reaparezca al cambiar de modo). Se agregó «Tamaño» a los grosores sin nombre y «Opacidad» en el lápiz; el marcador decía «Avanzado».
- **Mediciones:** referencia y distancia con dos clics; **Finalizar medida** (igual que Enter) mientras se dibuja un perímetro o área; **Modificar** después, para arrastrar las esquinas, y se termina con Enter o Finalizar medida.
- **Medidas en lienzo** (`UBCanvasRulers`, Edición): reglas de 24 px sobre la vista, en píxeles de la página desde su esquina superior izquierda (`-ancho/2, -alto/2` en la escena, o la del PDF); el paso rotulado es el menor de 5, 10, 20, 50, 100… que deja al menos 50 px de pantalla.
- **Tema:** al aceptar o aplicar un tema distinto del de inicio (`UBTheme::chosenAtStart`), un aviso y `UBApplication::restart`, que cierra como Salir y vuelve a abrir el programa con un ayudante que espera a que termine (PowerShell en Windows, `sh` en Linux y macOS, el AppImage si corre desde uno). Cambiar todo al instante no era seguro: muchos colores e íconos se fijan al crear cada parte.
- **Guía del programa** (antes Consejos y trucos) y **Sobre Master Board** (antes Creador), al final del menú Acerca.
- **Rayado entre hojas:** las hojas vecinas se ubican según la hoja (`pageRectOf`: tamaño nominal o el PDF), no según `normalizedSceneRect`, que crece con la tinta y separaba la hoja siguiente. Se dibujan en el fondo (`drawBackground`), no en el primer plano, que en repintados parciales tapaba el trazo con un cuadro blanco; la tinta que sale de una vecina se dibuja después, sin su fondo (`UBGraphicsScene::setDrawsBackground`).
- **Dirección de desplazamiento** (Versión 5 del primer Word, punto 2, quedó pendiente hasta ahora): en **Estilo plantilla**, con los dibujos de Samsung Notes: **Vertical**, **Horizontal** y **Horizontal en 2 páginas** (`UBBoardView::PageFlow`, ajuste `Board/LibertyBoardPageFlow`). Horizontal: las hojas siguen a la derecha y la rueda mueve de lado. En 2 páginas: parejas fijas 1-2, 3-4… como cuaderno abierto (8 px entre las dos, 48 px entre parejas); la rueda pasa de pareja en pareja (si la pareja es más alta que la vista, primero baja por ella) y al cambiar de página con botones, miniaturas o Agregar página se muestra la pareja entera (`fitPair`). En todos los modos, un clic o un trazo en la hoja de al lado la vuelve la hoja actual sin mover la imagen (`switchToPageAt`). La barra del documento va abajo cuando las hojas están lado a lado. Probado en Linux con 3 hojas en los tres modos.

## Mejoras después de la V2.32 (sin compilar aún; será la V2.35)

Pedidas por Luis al revisar la V2.32 («puntos 1, 2 y 3»).

- **Dirección de desplazamiento** también en **Personalización** (submenú de 4 opciones), además de Estilo plantilla, y un cuarto modo: **Vertical en 2 páginas** (`VerticalPairsFlow` = 3): parejas 1-2, 3-4… lado a lado, una pareja debajo de la otra, con su propio dibujo en Estilo plantilla. Orden en la ventana: Vertical, Vertical en 2 páginas, Horizontal, Horizontal en 2 páginas.
- **Medidas en lienzo:** la regla cambia de lado solo cuando la barra de herramientas queda a lo largo de ese borde (barra a la izquierda: la regla Y va a la derecha; barra arriba: la regla X va abajo). El cuadro del zoom se corre para no taparla (`placeZoomPalette` descuenta las reglas).
- **Botones de arriba a la derecha** más juntos (separación 2, relleno 2px 4px).
- **Modo oscuro** (`Board/LibertyBoardDarkMode`): botón con una luna a la izquierda de Multipantalla y casilla en Personalización. Invierte los colores de lo que se ve (`CompositionMode_Difference` en `drawForeground`); no cambia el documento ni lo que se guarda.
- **Panel de páginas:** en cada miniatura, al pasar el mouse o al estar elegida, botones **Borrar** y **Duplicar**, y una **estrella** de favorita arriba a la derecha. Filtro **Todo / Favoritas** arriba del panel. Las favoritas se guardan en el documento (`favorite-pages` en `metadata.rdf`, clase `UBPageFavorites`) y se corren al borrar, duplicar, insertar o mover páginas.
- **Cambiar color, tamaño y opacidad de lo seleccionado** (`UBStrokeStylePanel`): al elegir trazos, formas o flechas con la flecha o el lazo aparece un cuadro con colores, Tamaño (25 % a 400 %) y Opacidad; se puede deshacer. Para cambiar el grosor se simplifica cada trazo (Douglas-Peucker) y se rehace su contorno; en trazos del marcador con miles de puntos tarda menos de 1 s.
- **Reconocer y convertir texto manuscrito** (`UBHandwriting`): casilla en las opciones del bolígrafo (las 5 puntas). Un momento (1,2 s) después de que el lápiz descansa, las palabras escritas se reemplazan por un cuadro de texto del mismo tamaño y color, en un solo paso de deshacer. Usa el reconocedor de Windows (`Windows.UI.Input.Inking`, C++/WinRT, `-lwindowsapp`); en Linux y macOS la casilla queda desactivada. Para probar el flujo en Linux existe la variable `MASTERBOARD_FAKE_HANDWRITING`.
- **Trazo entre hojas, como Xournal:** al terminar un trazo que pasa a la hoja de al lado, cada parte queda en la hoja que le corresponde (`UBBoardView::cutLinesAcrossPages`, a partir de `UBGraphicsScene::takeFinishedStrokes`).
- Las dos casillas «Reconocer» del bolígrafo van una debajo de la otra, para ocupar menos ancho.
- **Solo en Windows:** lo que usa una herramienta de Windows sin equivalente en Linux y macOS queda en gris con el aviso «Solo en Windows» (texto manuscrito y dictado: el botón Dictar y el micrófono del cuadro de texto, `UBDictation::isAvailable`). El OCR usa Tesseract fuera de Windows, y leer en voz alta, el tema del sistema y Podcast tienen su versión en cada sistema.
- **Arreglado:** la cuadrícula de las hojas vecinas se dibujaba con un zoom viejo.
- **Guía del programa** actualizada en los 6 idiomas (páginas 3, 8, 17, 19 y 20) y las **33 traducciones** completas, con todo lo nuevo.

Probado en Linux con capturas: los 4 modos de desplazamiento, modo oscuro, favoritas (también después de reiniciar y de duplicar), color/tamaño/opacidad con deshacer, el corte entre hojas y el flujo del manuscrito con el reconocedor de prueba. No se pudo probar aquí: el reconocimiento real en Windows, el lápiz en la hoja de al lado con el lápiz digital, ni PDFs en los modos horizontales.

## Mejoras pequeñas después de la V2.35 (V2.36)

- Barras de opciones: el nombre y el valor de cada deslizador van centrados sobre él. Los círculos de colores no se estiran (`UBColorSwatches::setThreeRows` con tamaño fijo): ya no queda un hueco después de ellos, por ejemplo en Subrayar.
- Rotulador: la línea de muestra dibuja la punta elegida (`UBStrokePreview::setChiselTip`: punta plana inclinada -65°, con extremos rectos).
- Sellos en la barra: **Tu PNG** y un botón **Sellos** que abre todos los demás en una ventana debajo (4 columnas) y muestra el sello elegido; antes la fila no cabía y solo se podía deslizar con pantalla táctil.
- Agregar sonido: la barra dice «Haz clic en la página para grabar.».
- Bloquear lienzo/PDF ocupa solo el ancho de su nombre: el espacio hasta Índice es igual al de los demás botones.
- **Esperar antes de ocultar** (Estilo barra y Preferencias): 0,5 s y de 1 a 10 s, por defecto 1 s (`App/LibertyBoardBarHideDelay`, `UBBarStyle::hideDelay`); volver sigue siendo rápido.
- Guía del programa actualizada en los 6 idiomas (páginas 10, 17 y 19) y los 33 idiomas traducidos con todo lo nuevo.
- Deshacer una línea cortada entre hojas: la escena corta la línea justo después de guardar su paso de Deshacer, dentro de una misma macro (`UBGraphicsScene::setStrokesFinishedHandler`), y cada parte puesta en otra hoja agrega su propio paso (`UBCrossPagePartUndoCommand` en `UBBoardView.cpp`), que la quita de esa hoja y la guarda. Un solo Deshacer quita las dos partes.
- Los avisos de los botones de las miniaturas dicen Clonar, Cerrar y Favorito, como pidió Luis.
- **Estilo barra:** Ubicación (Arriba, Abajo, Izquierda, Derecha) para la barra de la pizarra y la del escritorio a la vez (`UBBarStyle::placeBars`, `UBDesktopPalette::placeAtEdge`); los bordes de Preferencias → Lápiz y barra de herramientas también mueven las dos. **Ocultar automáticamente**, en Estilo barra y en Preferencias: al marcarlo pregunta si deslizar hacia su lado o desvanecer (la lista al lado lo cambia después; ajustes `App/LibertyBoardBarAutoHide` y `App/LibertyBoardBarHideEffect`). La barra se oculta 0,9 s después de que el mouse la deja (no con un botón apretado, un menú o la ventana de opciones abierta) y vuelve igual al acercar el mouse a su lado. Oculta, solo se mueve fuera de su ventana pasado el borde (`UBBarStyle::watch`), así el resto del programa la sigue viendo mostrada; una barra alta se oculta a la izquierda o la derecha y una ancha arriba o abajo.
- Miniaturas: la estrella de favorita va abajo a la derecha (en las páginas angostas de un PDF chocaba con Borrar y Duplicar); los tres dicen qué hacen al pasar el mouse.

## Forma de trabajo

- El dueño es **Luis** (Chile), analista financiero, sin experiencia en programación. Habla en español; se le responde en español, claro y sin jerga. Usa Windows 11, pantalla ancha (unos 2540 px) y lápiz digital.
- Pide cambios en tandas, a menudo con capturas marcadas en verde. Si dice "no compiles aún", se hace commit sin push a `master` (o push a otra rama).
- Antes de cada push: compilar en Linux y probar lo visible en la aplicación real, con capturas. Al terminar, decirle qué se probó y qué no, y qué debe revisar él en Windows.
- Push a `master` = nueva versión para él. Se le avisa del número de compilación y que actualice con **Acerca → Actualización**.
- Commits en inglés con el formato `tipo: resumen` y un cuerpo que explica qué se hizo y en qué se probó (ver `CODING_STYLE.md`). Nunca usar sus documentos reales para pruebas.
- Respetar la GPLv3: se mantienen los encabezados de copyright del código, los archivos `LICENSE` y `COPYRIGHT`, y los avisos legales (GPLv3, copyright de OpenBoard y de las bibliotecas) en Preferencias → Acerca de → **Referencias**.

## Pruebas en Linux

- Compilar con CMake + Ninja (Qt 6 del sistema) e instalar con `ninja install` (queda como `/usr/local/bin/openboard`).
- Pantalla virtual: `Xvfb :99 -screen 0 2540x1000x24 -dpi 96` + `openbox` (+ `xcompmgr` para ver ventanas translúcidas como el foco); abrir con `HOME=<carpeta de prueba> LANG=es_ES.UTF-8 LANGUAGE=es QTWEBENGINE_DISABLE_SANDBOX=1 LIBGL_ALWAYS_SOFTWARE=1`. Clics con `xdotool`, capturas con `import -window root`.
- Para abrir un PDF de prueba: ejecutar un segundo `openboard <archivo.pdf>`, que lo pasa al programa ya abierto. No reutilizar el mismo nombre de PDF (pregunta si reemplazar).

## Ideas pendientes

- **Instalador** (Inno Setup, hay un script de OpenBoard en `release_scripts/windows`): por ahora el dueño prefiere el portable.
- Importar un PDF con el mismo nombre que uno ya importado, pero cambiado por otro programa, sigue preguntando si reemplazar (si no cambió, se retoma su documento desde la compilación 17).
- Firmas y formularios PDF: descartado por Luis.
- La grabación y la reproducción de sonido no se pudieron probar en Linux (sin dispositivo de audio).
- Se vio una vez que el programa se cerraba al abrir un segundo PDF mientras aparecía esa pregunta de reemplazo (viene de OpenBoard); revisar.
- La ventana de bienvenida (consejos) se abre al inicio hasta que se desmarca.
- **Grabación de audio sincronizada con lo escrito** (como en Notability: al tocar un trazo, el audio salta a ese momento): Luis no la quiere por ahora; queda como posible mejora futura.
