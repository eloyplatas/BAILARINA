# Bailarina

<img width="937" height="652" alt="Captura de pantalla_2026-11-15_02-08-07" src="https://github.com/user-attachments/assets/356f57ca-f21c-4252-96ad-39c558ac68ae" />


Caja de música por modelado físico (síntesis modal). Plugin VST3 / Standalone (AU en macOS) hecho con JUCE.

Bailarina es un instrumento virtual (VST3) que recrea una caja de música mecánica: la que llevaban dentro los antiguos joyeros, donde una pequeña bailarina empezaba a girar ante un espejo al abrir la tapa. No hay ninguna muestra grabada: todo el sonido se
calcula en tiempo real con un modelo físico del peine, las púas, la caja de madera y la maquinaria de relojería.

La intención es doble. Por un lado, recrear con fidelidad una caja de música mediante modelado físico, con su mecanismo, sus melodías y sus defectos. Por otro, expandir las posibilidades del instrumento: como el modelo está hecho de parámetros y no de grabaciones, el mismo motor puede llevarse mucho más allá de lo que haría una caja real, hacia campanas, kalimbas, cristal,
bordones y timbres que todavía no tienen nombre.

El Lume Mode es un modo especial con su propio panel. Sirve para recrear el sonido de grabaciones antiquísimas, y además para hacer Traditional Galician Noise y otras cosas bellas. 

<img width="940" height="860" alt="Bailarina_interfaz_v6" src="https://github.com/user-attachments/assets/6f6b6d25-206a-411b-a4f8-def9d0567389" />



- **Motor** (`Source/MusicBoxEngine.h`, C++ puro sin JUCE): cada nota es una lámina del peine modelada como barra empotrada-libre (parciales 1 : 6,27 : 17,55 : 34,39), púa con fase de contacto y zumbido de re-pulsado, apagadores, caja de madera (banco de resonadores) y ruido del regulador.
- **Cilindro** (`Source/CylinderPlayer.h`): 44 melodías de dominio público (ver `CREDITS.md`), velocidad manual (x0,1 a x4) o tempo del DAW, cuerda real opcional y modo radio (todas las piezas en orden aleatorio, con dos segundos de pausa).
- **Edad**: desafinación de hasta ±140 cents, láminas oxidadas, púas rotas, flojas o dobladas, y averías del regulador (lloro, deriva, atascos y acelerones del cilindro).
- **Lume Mode** (`Source/LumeFx.h`, `Source/LumePanel.cpp`): distorsión (de overdrive suave a extrema) seguida de una reverb de primeras reflexiones, en serie o en paralelo con el sonido original y con dry/wet. Al activarlo aparece un panel inferior con aspecto de instrumento de medida de los años 50.
- **MIDI externo**: cada nota pulsa su lámina (los note-off se ignoran, como en una caja real).
- **Melodías propias**: cualquier `.mid` que dejes en `~/.config/Bailarina/Melodias/` se añade a la lista al cargar el plugin.

## Añadir melodías a la librería interna

Deja el `.mid` en `tools/melodias/midi/` (o un `.abc` en `tools/melodias/abc/` y conviértelo con `abc2midi`), añade una línea a `LIBRARY` en `tools/make_melodies.py` y ejecuta `python3 tools/make_melodies.py` (necesita `pip install mido`). Regenera `Source/MelodyData.h`.

## Compilar (Linux)

    sudo apt install build-essential cmake ninja-build libasound2-dev libx11-dev libxrandr-dev \
         libxinerama-dev libxcursor-dev libxext-dev libfreetype-dev libfontconfig1-dev
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release        # descarga JUCE 8.0.4
    cmake --build build
    cp -r build/Bailarina_artefacts/Release/VST3/Bailarina.vst3 ~/.vst3/

Con una copia local de JUCE: `cmake -B build -DJUCE_DIR=/ruta/a/JUCE`.

Probar el sonido sin plugin: `build/render_demo list` y `build/render_demo 8 30 salida.wav 0.7` (melodía, segundos, archivo, edad).
