<p align="center">
  <img src="docs/images/libertyboard-logo.png" alt="Master Board" width="160">
</p>

<h1 align="center">Master Board</h1>

<p align="center"><b>Pizarra digital y lector de PDF para escribir, dibujar, presentar y estudiar, con inteligencia artificial integrada.</b></p>

<p align="center">
  <img src="resources/startupHints/assets/guide/board.png" alt="Master Board" width="820">
</p>

Master Board junta en un solo programa lo que antes estaba repartido en muchas herramientas: un lector y anotador de PDF, una pizarra para clases y presentaciones, un modo para escribir sobre cualquier programa abierto y un panel para trabajar con ChatGPT, Claude, Gemini, Copilot y otras IA. Sirve para estudiantes, profesores, trabajadores, creadores de contenido y, en general, para todos los que quieren una herramienta de PDF y pizarra multifuncional.

Lo creó Luis, conocido en YouTube como «Inversor Chileno». Luis no programa: construyó Master Board con la ayuda de Claude, una inteligencia artificial, aportando las ideas.

## Descargar

Abre la sección **[Releases](https://github.com/inversorchileno/LibertyBoard/releases/latest)** y descarga el archivo de tu sistema:

| Sistema | Archivo | Cómo abrirlo |
|---|---|---|
| **Windows** 10 y 11 | `MasterBoard-windows.zip` | Descomprime la carpeta y abre `MasterBoard.exe`. No necesita instalación. |
| **macOS** con chip Apple (M1, M2, M3, M4…) | `MasterBoard-macos-apple-silicon.dmg` | Abre el `.dmg` y arrastra Master Board a Aplicaciones. |
| **macOS** con procesador Intel (Mac antiguos) | `MasterBoard-macos-intel.dmg` | Igual que el anterior. |
| **Linux** (64 bits) | `MasterBoard-linux-x86_64.AppImage` | Dale permiso de ejecución (clic derecho → Propiedades → Permitir ejecutar, o `chmod +x`) y ábrelo. |

¿Qué Mac tengo? Menú  → **Acerca de este Mac**: si dice «Chip Apple M…», usa la versión *apple-silicon*; si dice «Procesador Intel…», la versión *intel*.

En Windows, las versiones siguientes se instalan desde el propio programa: **Acerca → Actualización**. En macOS y Linux, ese mismo menú abre la página para descargar la nueva versión.

### Aviso de seguridad al abrirlo por primera vez

**Windows y macOS mostrarán un aviso de seguridad la primera vez que abras Master Board.** Es normal y no significa que el programa tenga un virus.

**Por qué aparece:** Windows y macOS confían de inmediato en los programas *firmados digitalmente* con un certificado comprado a una empresa autorizada (en Windows, varios cientos de dólares al año; en Apple, la cuenta de desarrollador cuesta US$99 al año y cada versión debe enviarse a Apple para su revisión). Master Board es un proyecto personal, gratuito y de código abierto, y por ahora no está firmado. Por eso el sistema no sabe quién lo hizo y te pide confirmar. Todo su código está publicado en este repositorio, y cada versión se compila automáticamente en los servidores de GitHub a partir de ese código.

**Cómo abrirlo:**
- **Windows** («Windows protegió su PC», SmartScreen): haz clic en **Más información** y luego en **Ejecutar de todas formas**.
- **macOS** («no se puede abrir porque no se puede verificar al desarrollador»): haz **clic derecho** (o Control + clic) sobre Master Board en Aplicaciones → **Abrir** → **Abrir**. Si no aparece el botón, ve a **Configuración del Sistema → Privacidad y seguridad** y presiona **Abrir igualmente** junto al aviso de Master Board.
- **Linux** no muestra este aviso; solo hay que darle permiso de ejecución al archivo.

Solo se pide la primera vez. Master Board guarda sus documentos y su configuración en su propia carpeta de usuario.

## Todas las funciones

### Escribir y dibujar
- **Bolígrafo** con cinco puntas: lápiz pasta, pluma fuente, pluma caligráfica, lápiz grafito y marcador; sensible a la presión del lápiz digital.
- **Colores**: 15 a un clic, rueda para cualquier color y **cuentagotas** para tomar un color de la pantalla; grosor y opacidad.
- **Reconocer formas**: un círculo, rectángulo, triángulo o línea dibujados a mano quedan perfectos.
- **Marcador** de punta circular o recta, como un destacador.
- **Borrador** entero (borra el trazo completo) o parcial.
- **Formas**: 20 formas (línea, polilínea, elipse, rectángulo, triángulos, rombo, flechas de bloque, estrellas, globos de diálogo, nube) con borde, relleno, estilo de línea y opacidad.
- **Flecha** con estilos de línea, puntas y flecha cónica.
- **Texto** con fuente, tamaño, negrita, cursiva y color; casillas para marcar, viñetas, números y **tablas**.
- **Notas** de colores, **notas rápidas**, **sellos** (Aprobado, Confidencial, Borrador… y tu propio PNG), **emojis** y **GIF animados** de klipy.com.
- **Adjuntar archivos**, **hipervínculos** y **sonidos** grabados con tu voz.

### Presentar
- **Puntero láser** con estela que se desvanece, punto o seguimiento del mouse sin clic; efectos estándar, brillo blanco y arcoíris.
- **Foco**, que oscurece todo menos alrededor del puntero, y **lupa**.
- **Pantalla completa** (F11), **doble pizarra** con dos documentos lado a lado y **bloquear lienzo/PDF** para no cambiar nada por error.
- **Leer en voz alta** desde el inicio, desde un texto seleccionado o desde la palabra que tocas, con las voces de Windows; **dictado** de Windows en los cuadros de texto.

### PDF
- Abrir varios PDF en **pestañas** (se reordenan arrastrándolas) y **archivos recientes**.
- **Resaltar** ajustado al tamaño de letra y **subrayar**.
- **Guardar** con las anotaciones dentro del PDF original, **Guardar como** PDF, PNG o JPEG, **Imprimir** con vista previa y **Extraer páginas**.
- **Buscar** con lista de resultados, **Índice** del PDF y enlaces que funcionan; copiar el texto del PDF con la flecha o el lazo.
- **Reconocer texto (OCR)** de un PDF escaneado, para poder buscar y copiar su texto.
- Vista de varias páginas, barra de páginas en el borde derecho, **girar** páginas, **agregar** y **borrar** páginas, zoom con porcentaje editable y pantalla táctil con dos dedos.
- Las notas rápidas y los adjuntos quedan como comentarios y adjuntos dentro del PDF guardado.

### Plantillas
- **Estilo plantilla**: color de la hoja, cuadrícula o líneas y 21 **plantillas** (calendarios diario, semanal, mensual y anual que se arman solos, listas, notas Cornell, pentagrama, papel milimetrado, plan de clase, mapa mental, guion gráfico…), en español, inglés, alemán, francés, chino y japonés.
- Color y opacidad de las líneas de la cuadrícula, del rayado y de los puntos.
- Tamaño de página/lienzo personalizado, para seguir dibujando cuando la hoja se llena.

### Medir
- **Mediciones**: distancia, perímetro y área, en la pizarra y en el escritorio, en milímetros, centímetros, metros, kilómetros, pulgadas, pies, yardas o millas.
- Se dibuja una vez una línea de referencia de 1 unidad; en la pizarra el zoom no cambia las medidas.

### Inteligencia artificial
- Herramienta **IA** con **Resumen, Traducción, Crear imagen, Quitar fondo, Borrado generativo** y **Transcribir audio**.
- Eliges una parte de la página o todo el PDF; Master Board lo adjunta, escribe la instrucción y la envía.
- La página de la IA se abre al costado, con tu sesión guardada: ChatGPT, Claude, Gemini, Copilot, DeepSeek, Grok, Le Chat y Perplexity, y cualquier otra que agregues en Preferencias.
- El resultado vuelve a la página arrastrándolo, con clic derecho o descargándolo; si se acaban los usos gratis, un aviso ofrece otra IA.

### Modo escritorio
- Escribe, dibuja, señala con el láser y usa la IA encima de cualquier programa abierto.
- La barra queda vertical a los lados o horizontal arriba y abajo, con triángulos de opciones, se achica con un clic y lleva una papelera para borrar todo.
- Borde de color alrededor de la pantalla y captura de pantalla.

### A tu manera
- Menú **Herramientas** con todas las herramientas.
- Menú **Personalización**: elige qué botones se ven en la pizarra y en el escritorio, su orden con **Mover elementos** y su **Estilo barra** (colores, opacidad y esquinas de cuadradas a redondas).
- **Preferencias** ordenadas por categorías, con **Tema** oscuro, blanco, gris o igual que Windows.
- **Acerca → Atajos**: todos los atajos de teclado, que se pueden cambiar, con aviso si chocan con Windows o con el programa.
- **Consejos y trucos** (F1): una guía con imágenes de cada herramienta y un índice para ir a cualquier tema.
- 33 idiomas.

## Compilar desde el código

La versión de Windows se compila automáticamente con GitHub Actions (`.github/workflows/windows-build.yml`) usando Qt 6 y MSVC, y se publica en Releases con las notas de `docs/releases`. Las notas de diseño y lo acordado para cada etapa están en [docs/MASTERBOARD.md](docs/MASTERBOARD.md).

## Licencia

Master Board es software libre bajo la Licencia Pública General de GNU, versión 3 (GPLv3), derivado de [OpenBoard](https://github.com/OpenBoard-org/OpenBoard); ver [LICENSE](LICENSE) y [COPYRIGHT](COPYRIGHT).
