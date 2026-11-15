# Créditos

**Bailarina** es un instrumento de **Eloy Platas**: idea, diseño y desarrollo.

## Melodías

Todas las composiciones son de dominio público. Los datos de notas proceden de:

**Arreglos propios del proyecto** (escritos a mano, `Source/Melodies.h` y `tools/melodias/abc/`):
El lago de los cisnes, Para Elisa, Canción de cuna, Ah vous dirai-je maman, Greensleeves, Himno a la alegría,
Minueto en sol, Canon en re, Largo de la Sinfonía del Nuevo Mundo, Muiñeira de Chantada, Frère Jacques,
Au clair de la lune, Sur le pont d'Avignon, Scarborough Fair, Drunken Sailor, Auld Lang Syne, Korobéiniki,
O Tannenbaum, O du lieber Augustin, Bella ciao.

**Transcripciones para gaita de Nito Martínez (github.com/nitomartinez/gaita):**
Muiñeiras de Berdoias, Boimorto y Molete, Xota, Mazurcas da Bretoña y de Liñares, Pasodobres de Menduiña y
dos xigantes, Danza de Tenorio y A xeito de marcha. Son temas tradicionales; el repositorio no declara
licencia para las transcripciones, así que conviene citar la fuente (o pedir permiso al autor) si publicas
el plugin. Aquí están simplificados: sin adornos, con un bordón de tónica y quinta añadido.

**Ediciones del Mutopia Project (mutopiaproject.org), licencia Public Domain:**
Satie, Gymnopédies n.º 1 y 3 · Bach, Preludio BWV 846 y Aria BWV 1068 · Mozart, KV 525 (1.er mov.) y
Rondó alla turca KV 331 · Grieg, op. 46 n.º 4 · Schumann, op. 15 n.º 7 · Debussy, Clair de lune ·
Brahms, op. 39 n.º 15 · Schubert, D 839.

El Nocturno de Chopin y el vals de Brahms llevan el acompañamiento reducido (bajo y una nota por tiempo).

**Ediciones del Mutopia Project con licencia Creative Commons Attribution-ShareAlike:**
- Chopin, Nocturno op. 9 n.º 2 — CC BY-SA 3.0, edición de Renato Biolcati Rinaldi
- Satie, Gnossienne n.º 1 — CC BY-SA 4.0, edición de Knute Snortum
- Satie, Gnossienne n.º 3 — CC BY-SA 4.0, edición de Frédéric Duperray

Estas tres exigen atribución y compartir-igual para las obras derivadas de la edición. Si eso no encaja con
la licencia con la que publiques el plugin, quítalas de `LIBRARY` en `tools/make_melodies.py` y regenera
`Source/MelodyData.h`, o sustitúyelas por un MIDI propio.

No se incluye *Edelweiss* (Rodgers y Hammerstein, 1959): tiene derechos vigentes.

## Tipografía

El rótulo del panel Lume usa *Special Elite* (Astigmatic), con licencia Apache 2.0; el texto de la licencia está en
`assets/SpecialElite-LICENSE.txt`.
