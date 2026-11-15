# -*- coding: utf-8 -*-
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, Image, Table, TableStyle,
                                PageBreak, Flowable, KeepTogether, NextPageTemplate)
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

D = '/usr/share/fonts/truetype/dejavu/'
pdfmetrics.registerFont(TTFont('Serif', D + 'DejaVuSerif.ttf'))
pdfmetrics.registerFont(TTFont('Serif-Bold', D + 'DejaVuSerif-Bold.ttf'))
pdfmetrics.registerFont(TTFont('Serif-Italic', D + 'DejaVuSerif-Italic.ttf'))
pdfmetrics.registerFont(TTFont('Serif-BoldItalic', D + 'DejaVuSerif-BoldItalic.ttf'))
pdfmetrics.registerFontFamily('Serif', normal='Serif', bold='Serif-Bold', italic='Serif-Italic', boldItalic='Serif-BoldItalic')
pdfmetrics.registerFont(TTFont('Type', '/home/claude/bailarina/assets/SpecialElite.ttf'))

WINE, BRASS, INK, PAPER, GREY = colors.HexColor('#6e1226'), colors.HexColor('#9a7526'), colors.HexColor('#2b2118'), colors.HexColor('#f6f0e1'), colors.HexColor('#5d665b')
W, H = A4
st = dict(
    body=ParagraphStyle('body', fontName='Serif', fontSize=9.6, leading=14, textColor=INK, alignment=TA_JUSTIFY, spaceAfter=6),
    lead=ParagraphStyle('lead', fontName='Serif-Italic', fontSize=10.6, leading=15.5, textColor=INK, alignment=TA_JUSTIFY, spaceAfter=8),
    h1=ParagraphStyle('h1', fontName='Type', fontSize=19, leading=24, textColor=WINE, spaceBefore=4, spaceAfter=8),
    h2=ParagraphStyle('h2', fontName='Serif-Bold', fontSize=11.5, leading=15, textColor=BRASS, spaceBefore=8, spaceAfter=4),
    cell=ParagraphStyle('cell', fontName='Serif', fontSize=8.5, leading=11.6, textColor=INK),
    cellb=ParagraphStyle('cellb', fontName='Serif-Bold', fontSize=8.5, leading=11.6, textColor=INK),
    head=ParagraphStyle('head', fontName='Serif-Bold', fontSize=8.5, leading=11, textColor=colors.white),
    cap=ParagraphStyle('cap', fontName='Serif-Italic', fontSize=8, leading=11, textColor=GREY, alignment=TA_CENTER, spaceAfter=8),
    bullet=ParagraphStyle('bullet', fontName='Serif', fontSize=9.6, leading=13.6, textColor=INK, leftIndent=12, bulletIndent=2, spaceAfter=2.5),
    small=ParagraphStyle('small', fontName='Serif', fontSize=8.2, leading=11.4, textColor=INK, spaceAfter=3),
)

class Flow(Flowable):
    """Diagrama de flujo de senal."""
    def __init__(self, boxes, note, width):
        super().__init__(); self.boxes, self.note, self.width, self.height = boxes, note, width, 62
    def draw(self):
        c = self.canv; n = len(self.boxes); gap = 9; bw = (self.width - gap * (n - 1)) / n
        for i, (t1, t2, lume) in enumerate(self.boxes):
            x = i * (bw + gap)
            c.setFillColor(colors.HexColor('#dfe3da') if lume else PAPER); c.setStrokeColor(GREY if lume else BRASS); c.setLineWidth(1)
            c.roundRect(x, 16, bw, 42, 4, fill=1, stroke=1)
            c.setFillColor(INK); c.setFont('Serif-Bold', 6.3); c.drawCentredString(x + bw / 2, 42, t1)
            c.setFont('Serif', 5.5); c.drawCentredString(x + bw / 2, 30, t2)
            if i < n - 1:
                c.setStrokeColor(INK); c.setFillColor(INK); c.line(x + bw + 1, 37, x + bw + gap - 3, 37)
                p = c.beginPath(); p.moveTo(x + bw + gap - 1, 37); p.lineTo(x + bw + gap - 4, 39.5); p.lineTo(x + bw + gap - 4, 34.5); p.close(); c.drawPath(p, fill=1, stroke=0)
        c.setFont('Serif-Italic', 7.4); c.setFillColor(GREY); c.drawCentredString(self.width / 2, 3, self.note)

def table(rows, widths, header=True):
    data = [[Paragraph(str(x), st['head'] if (header and r == 0) else (st['cellb'] if c == 0 else st['cell'])) for c, x in enumerate(row)] for r, row in enumerate(rows)]
    t = Table(data, colWidths=widths, repeatRows=1 if header else 0)
    style = [('VALIGN', (0, 0), (-1, -1), 'TOP'), ('TOPPADDING', (0, 0), (-1, -1), 3.5), ('BOTTOMPADDING', (0, 0), (-1, -1), 4),
             ('LINEBELOW', (0, 0), (-1, -1), 0.3, colors.HexColor('#cfc4a4')), ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, colors.HexColor('#faf6ea')])]
    if header: style += [('BACKGROUND', (0, 0), (-1, 0), WINE)]
    t.setStyle(TableStyle(style)); return t

def P(t, s='body'): return Paragraph(t, st[s])
def B(items): return [Paragraph(t, st['bullet'], bulletText='•') for t in items]

FULL = W - 40 * mm

# ---------------------------------------------------------------------------------------------
L = {}
L['en'] = dict(
 part='ENGLISH', footer='Bailarina · User manual',
 s1='1 · What Bailarina is',
 intro=["Bailarina is a virtual instrument (VST3) that recreates a mechanical music box — the kind hidden inside old jewellery boxes, where a little ballerina starts to turn in front of a mirror when the lid is opened. Nothing in it is sampled: every sound is computed in real time by a physical model of the comb, the pins, the wooden case and the clockwork.",
        "The intention is twofold. First, to recreate a music box faithfully through physical modelling, with its mechanism, its tunes and its flaws. Second, to <b>expand the possibilities of the instrument</b>: because the model is made of parameters and not recordings, the same engine can be pushed far beyond what any real music box could do, towards bells, kalimbas, glass, drones and timbres that have no name yet.",
        "<b>Lume Mode</b> is a special mode with its own panel. It is meant to recreate the sound of <b>very, very old recordings</b> — and also to make <b>Traditional Galician Noise</b> and other beautiful things."],
 s2='2 · Quick start',
 quick=["Load Bailarina on an instrument track. It needs no audio input.",
        "Click the <b>mirror</b> (or the CILINDRO switch): the cylinder starts turning, the ballerina dances and the selected tune plays.",
        "Choose another tune on the <b>MELODÍAS</b> card. Scroll the list with the mouse wheel.",
        "Play it from a <b>MIDI keyboard</b> or a MIDI item: each note plucks its own tooth of the comb, whether the cylinder is turning or not.",
        "Turn the brass knobs. Drag up/down; the current value is shown under each knob."],
 install="<b>Installation.</b> Copy <i>Bailarina.vst3</i> to the VST3 folder of your system and rescan plugins in your DAW. Linux: <font face='Type'>~/.vst3/</font>. Windows: <font face='Type'>C:\\Program Files\\Common Files\\VST3\\</font>. macOS: <font face='Type'>~/Library/Audio/Plug-Ins/VST3/</font> (and <i>Bailarina.component</i>, the Audio Unit, to <font face='Type'>~/Library/Audio/Plug-Ins/Components/</font>). The macOS build is universal (Apple silicon and Intel) and is not notarised, so macOS may block it the first time: allow it in Privacy &amp; Security, or run <font face='Type'>xattr -cr</font> on the plugin. The Windows and macOS versions are built automatically and have not yet been tested in a DAW.",
 s3='3 · How it works',
 flow=[('MIDI + CYLINDER', 'note triggers', 0), ('COMB', 'one modal tooth per note', 0), ('PINS · DAMPERS', 'pluck, buzz, click', 0), ('CASE', '10 wood resonances', 0), ('MECHANISM', 'governor noise', 0), ('LUME MODE', 'distortion + reflections', 1)],
 flownote='Signal path. The Lume Mode stage is only active when its switch is on.',
 how=["There are two ways of making Bailarina sound, and they can be used together. The <b>cylinder</b> is an internal player: each tune is a set of pins on a rotating drum, exactly one revolution long. <b>External MIDI</b> plucks the teeth directly. Both feed the same comb.",
      "From there the signal follows the path of the real object: the vibration of the steel teeth is passed to the wooden case, which colours and radiates it; the clockwork adds its own whirr; and finally the optional Lume Mode processes the result."],
 s4='4 · The synthesis method',
 syn=[("The comb: modal synthesis of a vibrating bar", "Each tooth of a music box comb is a small steel bar clamped at one end and free at the other. Such a bar does not vibrate at harmonic multiples of its fundamental: its natural modes fall at <b>1 : 6.27 : 17.55 : 34.39</b>. Those inharmonic partials are what makes a music box sound crystalline rather than string-like. Bailarina models every tooth as up to four independent damped resonators tuned to exactly those ratios (partials above 19 kHz are left out). There are 128 teeth, one per MIDI note. Higher partials die away faster than the fundamental, and high notes die away faster than low ones, as in the real instrument."),
      ("The pin: lift and release", "A pin does not strike the tooth: it lifts it and lets it go. The model starts each note from an initial displacement, so the tone begins cleanly with a tiny tick, and a short burst of filtered noise reproduces the scrape of the pin. A stronger pluck (higher MIDI velocity) is louder and slightly brighter."),
      ("Re-plucking and dampers", "When a pin reaches a tooth that is still vibrating, the tooth chatters against it for a few milliseconds before being released: that is the characteristic buzz of cheap or worn music boxes. Real combs carry small dampers to prevent it. Bailarina models this contact phase: the old vibration is choked and, depending on the Dampers control, a buzz is produced."),
      ("The case", "The comb alone is very quiet; the wooden box is the loudspeaker. It is modelled as a bank of ten resonances spread across the stereo field. Its size can be changed from a thimble to a trunk."),
      ("The clockwork", "While the cylinder turns, the governor (the small spinning fan that keeps the speed constant) adds a whirr and gear ticks that follow the speed of the cylinder. The spring, the wear of the pins and the failures of the governor are modelled too: see Age and Real spring below.")],
 s5='5 · The interface',
 ui=["<b>Lid.</b> Tune card (left), mirror and ballerina (centre), name plate and switches (right).",
     "<b>Mechanism.</b> The governor, the comb and the cylinder with the real pins of the current tune (one turn = the whole tune), and the winding key. Teeth light up when they sound.",
     "<b>Front panel.</b> Twelve brass knobs.",
     "<b>Lume panel.</b> Appears under the box when LUME MODE is switched on."],
 cap1='Main window. The labels of the instrument are in Spanish; this manual gives both names.',
 s6='6 · Sound parameters', ph=['Knob', 'Range · default', 'What it does'],
 params=[('TONO<br/>Tone', '0–100 % · 55 %', 'Weight of the upper partials of each tooth. Low values give a dark, round note; at the top the partials become louder than the fundamental and the sound turns to glass.'),
         ('DURACIÓN<br/>Decay', '0.05–30 s · 2.5 s', 'Ring time of the fundamental at A4 (decay to −60 dB). Low notes last longer and high notes less. From a dry wooden tap to half a minute of sustain.'),
         ('PÚA<br/>Pin', '0–100 % · 35 %', 'Level of the noise of the pin leaving the tooth.'),
         ('APAGADORES<br/>Dampers', '0–100 % · 60 %', 'At 100 % a repeated note is silenced cleanly before being plucked again. Lower values lengthen the contact (4 to 30 ms) and let the tooth buzz against the pin.'),
         ('EDAD<br/>Age', '0–100 % · 15 %', 'Wear of the whole machine: tuning, teeth, pins and speed. See section 8.'),
         ('CAJA<br/>Case', '0–100 % · 50 %', 'Amount of wooden-case resonance. At 0 % only the bare comb is heard; at 100 % almost only the box.'),
         ('TAMAÑO<br/>Size', '0–100 % · 50 %', 'Size of the case. Its resonances move from about five times higher (a thimble) to five times lower (a trunk), and larger boxes ring longer.'),
         ('MECANISMO<br/>Mechanism', '0–100 % · 15 %', 'Level of the governor whirr and gear ticks. Only heard while the cylinder turns; it gets rougher with Age.'),
         ('ESTÉREO<br/>Stereo', '0–100 % · 60 %', 'Width of the comb: low teeth to the left, high teeth to the right.'),
         ('AFINACIÓN<br/>Tuning', '−24…+24 semitones · 0', 'Transposes the whole comb. Takes effect on the next pluck of each tooth.'),
         ('VELOCIDAD<br/>Speed', '×0.10–×4.00 · ×1', 'Speed of the cylinder. The pitch does not change: only the tempo of the tune.'),
         ('VOLUMEN<br/>Volume', '−36…+6 dB · −3 dB', 'Output level, followed by a gentle limiter.')],
 s7='7 · The cylinder', sh=['Control', 'What it does'],
 cyl=[('MELODÍAS card', 'Click a tune to load it. Mouse wheel scrolls the list. The tune that is playing is shown in red.'),
      ('CILINDRO<br/>parar / sonar', 'Stops or starts the cylinder. Clicking the mirror does the same. The cylinder resumes where it stopped and loops the tune.'),
      ('RADIO', 'Plays every tune in the library in random order, each one once, with two seconds of silence between pieces. No tune is repeated until all have been played.'),
      ('CUERDA REAL<br/>Real spring', 'When on, the spring runs down: a full spring lasts about 75 seconds, the music slows down towards the end and stops. Each click on the winding key adds about a fifth of the spring; the small gauge under the key shows what is left.'),
      ('TEMPO<br/>manual / DAW', 'Manual: the tune plays at its own tempo, scaled by Speed. DAW: one quarter note of the tune lasts one quarter note of the project, the cylinder only turns while the transport is playing and rewinds when playback starts. Speed still multiplies. If the host gives no tempo, Bailarina says so and stays in manual.')],
 lib="<b>Library.</b> Bailarina includes 44 public-domain tunes arranged for the comb: classical pieces (Tchaikovsky, Beethoven, Brahms, Mozart, Pachelbel, Bach, Chopin, Satie, Dvořák, Grieg, Schumann, Debussy, Schubert), eleven traditional Galician tunes (muiñeiras, xota, mazurcas, pasodobres…) and ten traditional European songs.",
 own="<b>Your own tunes.</b> Any MIDI file placed in the folder <font face='Type'>Bailarina/Melodias</font> (Linux: inside <font face='Type'>~/.config/</font>; Windows: inside <font face='Type'>%APPDATA%</font>; macOS: inside <font face='Type'>~/Library/</font>) is added to the end of the list the next time the plugin is loaded. Notes outside the comb are folded by octaves into its range.",
 midi="<b>External MIDI.</b> Every note-on plucks its tooth; velocity sets the strength. Note-offs are ignored, because a music box cannot stop a tooth: the note simply rings out (use Decay and Dampers to shape it). All Notes Off silences the comb.",
 s8='8 · Age: a machine that wears out',
 agei="Age is a single knob that makes the whole mechanism older. Its effects arrive gradually, so the first quarter of its travel is only a pleasant imprecision and the last quarter is a machine that barely works. All defects are fixed for each tooth and each pin: an old box is always wrong in the same places.",
 ah=['From', 'What starts to fail'],
 age=[('0 %', 'Each tooth goes slightly out of tune (up to ±140 cents at the top) and each pin is slightly misplaced (up to ±70 ms). The speed of the cylinder wavers and drifts.'),
      ('15 %', 'Worn pins: some notes sound much weaker.'),
      ('25 %', 'Broken pins: some notes are missing, up to about one in five.'),
      ('35 %', 'Governor failures: the cylinder sticks, almost stops for a moment and then races ahead.'),
      ('40 %', 'Bent pins that pluck the neighbouring tooth: wrong notes.'),
      ('45 %', 'Rusty teeth: dull, short notes with a louder click.')],
 s9='9 · Lume Mode',
 lume=["Lume Mode is a special mode for recreating the sound of the oldest recordings — wax cylinders, worn shellac, a gramophone horn in another room — and for making Traditional Galician Noise and other beautiful things. Switching it on opens a second panel under the box, built like a piece of 1950s test equipment.",
       "Inside it the sound goes through two stages, always in this order: a <b>distortion</b> that runs from a soft overdrive to complete destruction, and then a reverb made only of <b>early reflections</b>, which runs from a hint of a small room to a dense metallic swarm."],
 cap2='The Lume panel. The pilot lamp is lit while the mode is on and the meter shows the level of the effect.',
 lp=[('DISTORSIÓN<br/>Distortion', '0–10', 'Amount of drive. Low: soft, slightly asymmetric overdrive, like a tired valve. From the middle upwards hard clipping and wave folding are added; the red zone is extreme. Loudness is compensated, so the knob changes the character more than the level.'),
     ('REFLEXIONES<br/>Reflections', '0–10', 'Level and number of early reflections (from 5 to 18 per channel). In the upper part of the travel the reflections feed back into themselves and turn into a long, dense tail.'),
     ('SALA<br/>Room', '0–10', 'Spread of the reflections in time, from 15 ms (a metallic, comb-like colour, the inside of a horn) to about 480 ms (a large hall).'),
     ('DRY / WET', '0–10', 'In series: crossfade between the original sound and the processed sound. In parallel: how much of the effect is added to the original.'),
     ('SERIE / PARALELO<br/>Series / Parallel', 'switch', 'Series: the module replaces the original sound (according to Dry/Wet). Parallel: the original sound always passes untouched and the effect is added on top of it.')],
 s10='10 · Starting points', rh=['Sound', 'Settings to try'],
 rec1="These are starting points, not presets: move everything.",
 rec=[('A well-kept music box', 'Age 5–15 %, Dampers 60 %, Case 50 %, Mechanism 15 %.'),
      ('Glass bells', 'Decay 10–15 s, Tone 85 %, Size 80 %, Case 70 %, Dampers 100 %.'),
      ('Thumb piano', 'Decay 0.4 s, Tone 30 %, Pin 60 %, Size 35 %, Tuning −12.'),
      ('Wooden percussion', 'Decay 0.05–0.15 s, Case 100 %, and sweep Size.'),
      ('Buzzing prepared comb', 'Dampers 0 % and fast repeated notes from the keyboard.'),
      ('Slow drone', 'Decay 30 s, Speed ×0.10, Size 100 %.'),
      ('The attic box', 'Age 60–75 %, Real spring on, Mechanism 40 %.'),
      ('Wax cylinder', 'Lume in series, Dry/Wet 10. Distortion 2, Reflections 2, Room 1. Age 45 %.'),
      ('Shellac in the parlour', 'Lume in series, Dry/Wet 6. Distortion 3, Reflections 5, Room 4.'),
      ('Traditional Galician Noise', 'Any muiñeira (or Radio on). Age 70 % or more. Lume in parallel: Distortion 9–10, Reflections 7, Room 3, and Dry/Wet to taste.'),
      ('Cathedral of rust', 'Lume in series, Dry/Wet 9. Distortion 1, Reflections 10, Room 10.')],
 credh='Credits', cred=["<b>Bailarina</b> is an instrument by <b>Eloy Platas</b>: concept, design and development.",
        "Built with the JUCE framework. The lettering of the Lume panel uses the Special Elite typeface (Astigmatic, Apache License 2.0).",
        "All tunes are public-domain compositions. Several arrangements come from editions of the Mutopia Project, and ten of the Galician tunes from the bagpipe transcriptions of Nito Martínez; the full list of sources and licences is in the file CREDITS.md that comes with the source code."],
 notes="<b>Notes.</b> All knobs and switches can be automated. Version 0.1.",
)

L['es'] = dict(
 part='CASTELLANO', footer='Bailarina · Manual de uso',
 s1='1 · Qué es Bailarina',
 intro=["Bailarina es un instrumento virtual (VST3) que recrea una caja de música mecánica: la que llevaban dentro los antiguos joyeros, donde una pequeña bailarina empezaba a girar ante un espejo al abrir la tapa. No hay ninguna muestra grabada: todo el sonido se calcula en tiempo real con un modelo físico del peine, las púas, la caja de madera y la maquinaria de relojería.",
        "La intención es doble. Por un lado, recrear con fidelidad una caja de música mediante modelado físico, con su mecanismo, sus melodías y sus defectos. Por otro, <b>expandir las posibilidades del instrumento</b>: como el modelo está hecho de parámetros y no de grabaciones, el mismo motor puede llevarse mucho más allá de lo que haría una caja real, hacia campanas, kalimbas, cristal, bordones y timbres que todavía no tienen nombre.",
        "El <b>Lume Mode</b> es un modo especial con su propio panel. Sirve para recrear el sonido de <b>grabaciones antiquísimas</b>, y además para hacer <b>Traditional Galician Noise</b> y otras cosas bellas."],
 s2='2 · Primeros pasos',
 quick=["Carga Bailarina en una pista de instrumento. No necesita entrada de audio.",
        "Haz clic en el <b>espejo</b> (o en el interruptor CILINDRO): el cilindro empieza a girar, la bailarina baila y suena la melodía elegida.",
        "Elige otra melodía en la tarjeta <b>MELODÍAS</b>. La lista se recorre con la rueda del ratón.",
        "Tócala desde un <b>teclado MIDI</b> o un ítem MIDI: cada nota pulsa su propia lámina del peine, gire o no el cilindro.",
        "Mueve los mandos de latón arrastrando arriba y abajo; el valor actual aparece bajo cada mando."],
 install="<b>Instalación.</b> Copia <i>Bailarina.vst3</i> en la carpeta VST3 de tu sistema y vuelve a escanear los plugins en tu DAW. Linux: <font face='Type'>~/.vst3/</font>. Windows: <font face='Type'>C:\\Program Files\\Common Files\\VST3\\</font>. macOS: <font face='Type'>~/Library/Audio/Plug-Ins/VST3/</font> (y <i>Bailarina.component</i>, el Audio Unit, en <font face='Type'>~/Library/Audio/Plug-Ins/Components/</font>). La versión de macOS es universal (Apple silicon e Intel) y no está notarizada, así que macOS puede bloquearla la primera vez: permítela en Privacidad y seguridad, o ejecuta <font face='Type'>xattr -cr</font> sobre el plugin. Las versiones de Windows y macOS se compilan automáticamente y todavía no se han probado en un DAW.",
 s3='3 · Cómo funciona',
 flow=[('MIDI + CILINDRO', 'disparo de notas', 0), ('PEINE', 'una lámina modal por nota', 0), ('PÚAS · APAGADORES', 'pulsación, zumbido, clic', 0), ('CAJA', '10 resonancias de madera', 0), ('MECANISMO', 'ruido del regulador', 0), ('LUME MODE', 'distorsión + reflexiones', 1)],
 flownote='Recorrido de la señal. La etapa Lume Mode solo actúa con su interruptor encendido.',
 how=["Bailarina suena de dos maneras, que pueden usarse a la vez. El <b>cilindro</b> es un reproductor interno: cada melodía es un conjunto de púas sobre un rodillo que gira, y dura exactamente una vuelta. El <b>MIDI externo</b> pulsa las láminas directamente. Los dos atacan el mismo peine.",
      "A partir de ahí la señal sigue el camino del objeto real: la vibración de las láminas de acero pasa a la caja de madera, que la colorea y la radia; la maquinaria añade su propio zumbido; y al final, si se quiere, el Lume Mode procesa el resultado."],
 s4='4 · El método de síntesis',
 syn=[("El peine: síntesis modal de una barra vibrante", "Cada lámina del peine de una caja de música es una barrita de acero empotrada por un extremo y libre por el otro. Una barra así no vibra en múltiplos armónicos de su fundamental: sus modos naturales caen en <b>1 : 6,27 : 17,55 : 34,39</b>. Esos parciales inarmónicos son los que hacen que una caja de música suene cristalina y no como una cuerda. Bailarina modela cada lámina con hasta cuatro resonadores amortiguados independientes, afinados exactamente en esas proporciones (los parciales por encima de 19 kHz se omiten). Hay 128 láminas, una por nota MIDI. Los parciales agudos se apagan antes que la fundamental y las notas agudas antes que las graves, como en el instrumento real."),
      ("La púa: levantar y soltar", "La púa no golpea la lámina: la levanta y la suelta. El modelo arranca cada nota desde un desplazamiento inicial, de modo que el tono empieza limpio, con un pequeño tic, y una ráfaga breve de ruido filtrado reproduce el roce de la púa. Una pulsación más fuerte (más velocidad MIDI) suena más alta y algo más brillante."),
      ("El re-pulsado y los apagadores", "Cuando una púa llega a una lámina que todavía vibra, la lámina castañetea contra ella unos milisegundos antes de soltarse: es el zumbido típico de las cajas baratas o gastadas. Los peines reales llevan pequeños apagadores para evitarlo. Bailarina modela esa fase de contacto: la vibración anterior se ahoga y, según el mando Apagadores, aparece el zumbido."),
      ("La caja", "El peine solo apenas suena; la caja de madera es el altavoz. Se modela como un banco de diez resonancias repartidas en el campo estéreo. Su tamaño puede variar de un dedal a un baúl."),
      ("La maquinaria", "Mientras gira el cilindro, el regulador (el pequeño molinete que mantiene la velocidad) añade un zumbido y unos tics de engranaje que siguen la velocidad del cilindro. También se modelan la cuerda, el desgaste de las púas y las averías del regulador: véanse Edad y Cuerda real más abajo.")],
 s5='5 · La interfaz',
 ui=["<b>Tapa.</b> Tarjeta de melodías (izquierda), espejo y bailarina (centro), placa e interruptores (derecha).",
     "<b>Mecanismo.</b> El regulador, el peine y el cilindro con las púas reales de la melodía cargada (una vuelta = la melodía entera), y la llave de la cuerda. Las láminas se iluminan al sonar.",
     "<b>Frente.</b> Doce mandos de latón.",
     "<b>Panel Lume.</b> Aparece bajo la caja al encender LUME MODE."],
 cap1='Ventana principal.',
 s6='6 · Parámetros de sonido', ph=['Mando', 'Recorrido · inicial', 'Qué hace'],
 params=[('TONO', '0–100 % · 55 %', 'Peso de los parciales agudos de cada lámina. Abajo, una nota oscura y redonda; arriba del todo los parciales superan a la fundamental y el sonido se vuelve de cristal.'),
         ('DURACIÓN', '0,05–30 s · 2,5 s', 'Tiempo que tarda en apagarse la fundamental en La4 (caída de 60 dB). Las notas graves duran más y las agudas menos. De un golpe seco de madera a medio minuto de sostenido.'),
         ('PÚA', '0–100 % · 35 %', 'Nivel del ruido de la púa al soltar la lámina.'),
         ('APAGADORES', '0–100 % · 60 %', 'Al 100 %, una nota repetida se silencia limpiamente antes de volver a pulsarse. Valores bajos alargan el contacto (de 4 a 30 ms) y dejan que la lámina zumbe contra la púa.'),
         ('EDAD', '0–100 % · 15 %', 'Desgaste de toda la máquina: afinación, láminas, púas y velocidad. Véase la sección 8.'),
         ('CAJA', '0–100 % · 50 %', 'Cantidad de resonancia de la caja de madera. Al 0 % se oye el peine desnudo; al 100 %, casi solo la caja.'),
         ('TAMAÑO', '0–100 % · 50 %', 'Tamaño de la caja. Sus resonancias van de unas cinco veces más agudas (un dedal) a cinco veces más graves (un baúl), y las cajas grandes resuenan más tiempo.'),
         ('MECANISMO', '0–100 % · 15 %', 'Nivel del zumbido del regulador y de los tics del engranaje. Solo se oye mientras gira el cilindro, y se vuelve más áspero con la Edad.'),
         ('ESTÉREO', '0–100 % · 60 %', 'Anchura del peine: láminas graves a la izquierda, agudas a la derecha.'),
         ('AFINACIÓN', '−24…+24 semitonos · 0', 'Transporta todo el peine. Actúa en la siguiente pulsación de cada lámina.'),
         ('VELOCIDAD', '×0,10–×4,00 · ×1', 'Velocidad del cilindro. No cambia la altura: solo el tempo de la melodía.'),
         ('VOLUMEN', '−36…+6 dB · −3 dB', 'Nivel de salida, seguido de un limitador suave.')],
 s7='7 · El cilindro', sh=['Control', 'Qué hace'],
 cyl=[('Tarjeta MELODÍAS', 'Un clic carga la melodía. La rueda del ratón recorre la lista. La que está sonando aparece en rojo.'),
      ('CILINDRO<br/>parar / sonar', 'Detiene o arranca el cilindro. Hacer clic en el espejo hace lo mismo. El cilindro continúa donde se paró y repite la melodía en bucle.'),
      ('RADIO', 'Reproduce todas las melodías de la librería en orden aleatorio, una vez cada una, con dos segundos de silencio entre piezas. Ninguna se repite hasta haber sonado todas.'),
      ('CUERDA REAL', 'Encendida, la cuerda se agota: una cuerda completa dura unos 75 segundos, la música se va frenando al final y se para. Cada clic en la llave da aproximadamente una quinta parte de la cuerda; el pequeño indicador bajo la llave muestra la que queda.'),
      ('TEMPO<br/>manual / DAW', 'Manual: la melodía suena a su propio tempo, multiplicado por Velocidad. DAW: una negra de la melodía dura una negra del proyecto, el cilindro solo gira con el transporte en marcha y se rebobina al empezar la reproducción. Velocidad sigue multiplicando. Si el anfitrión no informa del tempo, Bailarina lo avisa y sigue en manual.')],
 lib="<b>Librería.</b> Bailarina incluye 44 melodías de dominio público arregladas para el peine: piezas clásicas (Chaikovski, Beethoven, Brahms, Mozart, Pachelbel, Bach, Chopin, Satie, Dvořák, Grieg, Schumann, Debussy, Schubert), once temas tradicionales gallegos (muiñeiras, xota, mazurcas, pasodobres…) y diez canciones tradicionales europeas.",
 own="<b>Melodías propias.</b> Cualquier archivo MIDI que dejes en la carpeta <font face='Type'>Bailarina/Melodias</font> (Linux: dentro de <font face='Type'>~/.config/</font>; Windows: dentro de <font face='Type'>%APPDATA%</font>; macOS: dentro de <font face='Type'>~/Library/</font>) se añade al final de la lista la próxima vez que se cargue el plugin. Las notas que quedan fuera del peine se pliegan por octavas a su registro.",
 midi="<b>MIDI externo.</b> Cada nota pulsa su lámina; la velocidad fija la fuerza. Los note-off se ignoran, porque una caja de música no puede parar una lámina: la nota suena hasta apagarse (Duración y Apagadores le dan forma). All Notes Off silencia el peine.",
 s8='8 · Edad: una máquina que se desgasta',
 agei="Edad es un único mando que envejece todo el mecanismo. Sus efectos llegan poco a poco: el primer cuarto del recorrido es solo una imprecisión agradable y el último, una máquina que apenas funciona. Todos los defectos son fijos para cada lámina y cada púa: una caja vieja falla siempre en los mismos sitios.",
 ah=['Desde', 'Qué empieza a fallar'],
 age=[('0 %', 'Cada lámina se desafina un poco (hasta ±140 cents al final) y cada púa queda algo descolocada (hasta ±70 ms). La velocidad del cilindro oscila y deriva.'),
      ('15 %', 'Púas gastadas: algunas notas suenan mucho más flojas.'),
      ('25 %', 'Púas rotas: faltan notas, hasta una de cada cinco aproximadamente.'),
      ('35 %', 'Averías del regulador: el cilindro se atasca, casi se para un instante y luego sale disparado.'),
      ('40 %', 'Púas dobladas que pulsan la lámina de al lado: notas equivocadas.'),
      ('45 %', 'Láminas oxidadas: notas sordas y cortas, con más clic.')],
 s9='9 · Lume Mode',
 lume=["El Lume Mode es un modo especial para recrear el sonido de las grabaciones más antiguas —cilindros de cera, pizarra gastada, la bocina de un gramófono en otra habitación— y para hacer Traditional Galician Noise y otras cosas bellas. Al encenderlo se abre un segundo panel bajo la caja, construido como un instrumento de medida de los años cincuenta.",
       "Dentro, el sonido pasa por dos etapas, siempre en este orden: una <b>distorsión</b> que va de un overdrive suave a la destrucción completa, y después una reverb hecha solo de <b>primeras reflexiones</b>, que va de la insinuación de un cuarto pequeño a un enjambre metálico y denso."],
 cap2='El panel Lume. El piloto está encendido mientras el modo está activo y el vúmetro marca el nivel del efecto.',
 lp=[('DISTORSIÓN', '0–10', 'Cantidad de saturación. Abajo: overdrive suave y algo asimétrico, como una válvula cansada. De la mitad hacia arriba se añaden recorte duro y plegado de onda; la zona roja es extrema. El volumen se compensa, así que el mando cambia más el carácter que el nivel.'),
     ('REFLEXIONES', '0–10', 'Nivel y número de primeras reflexiones (de 5 a 18 por canal). En la parte alta del recorrido las reflexiones se realimentan y se convierten en una cola larga y densa.'),
     ('SALA', '0–10', 'Separación de las reflexiones en el tiempo, de 15 ms (un color metálico, de peine, el interior de una bocina) a unos 480 ms (una sala grande).'),
     ('DRY / WET', '0–10', 'En serie: fundido entre el sonido original y el procesado. En paralelo: cuánto efecto se suma al original.'),
     ('SERIE / PARALELO', 'conmutador', 'Serie: el módulo sustituye al sonido original (según Dry/Wet). Paralelo: el sonido original pasa siempre intacto y el efecto se suma por encima.')],
 s10='10 · Puntos de partida', rh=['Sonido', 'Ajustes para probar'],
 rec1="Son puntos de partida, no presets: muévelo todo.",
 rec=[('Una caja bien cuidada', 'Edad 5–15 %, Apagadores 60 %, Caja 50 %, Mecanismo 15 %.'),
      ('Campanas de cristal', 'Duración 10–15 s, Tono 85 %, Tamaño 80 %, Caja 70 %, Apagadores 100 %.'),
      ('Kalimba', 'Duración 0,4 s, Tono 30 %, Púa 60 %, Tamaño 35 %, Afinación −12.'),
      ('Percusión de madera', 'Duración 0,05–0,15 s, Caja 100 %, y barrer Tamaño.'),
      ('Peine preparado que zumba', 'Apagadores 0 % y notas repetidas rápidas desde el teclado.'),
      ('Bordón lento', 'Duración 30 s, Velocidad ×0,10, Tamaño 100 %.'),
      ('La caja del desván', 'Edad 60–75 %, Cuerda real encendida, Mecanismo 40 %.'),
      ('Cilindro de cera', 'Lume en serie, Dry/Wet 10. Distorsión 2, Reflexiones 2, Sala 1. Edad 45 %.'),
      ('Pizarra en el salón', 'Lume en serie, Dry/Wet 6. Distorsión 3, Reflexiones 5, Sala 4.'),
      ('Traditional Galician Noise', 'Cualquier muiñeira (o la Radio encendida). Edad 70 % o más. Lume en paralelo: Distorsión 9–10, Reflexiones 7, Sala 3, y Dry/Wet al gusto.'),
      ('Catedral de óxido', 'Lume en serie, Dry/Wet 9. Distorsión 1, Reflexiones 10, Sala 10.')],
 credh='Créditos', cred=["<b>Bailarina</b> es un instrumento de <b>Eloy Platas</b>: idea, diseño y desarrollo.",
        "Hecho con el framework JUCE. El rótulo del panel Lume usa la tipografía Special Elite (Astigmatic, licencia Apache 2.0).",
        "Todas las melodías son composiciones de dominio público. Varios arreglos proceden de ediciones del Mutopia Project, y diez de los temas gallegos de las transcripciones para gaita de Nito Martínez; la lista completa de fuentes y licencias está en el archivo CREDITS.md que acompaña al código fuente."],
 notes="<b>Notas.</b> Todos los mandos e interruptores se pueden automatizar. Versión 0.1.",
)

def part(lang):
    c = L[lang]; s = []
    s += [P(c['s1'], 'h1'), P(c['intro'][0], 'lead'), P(c['intro'][1]), P(c['intro'][2])]
    s += [P(c['s2'], 'h1')] + [Paragraph(t, st['bullet'], bulletText='%d.' % (i + 1)) for i, t in enumerate(c['quick'])] + [Spacer(1, 4), P(c['install'], 'small')]
    s += [P(c['s3'], 'h1'), Flow(c['flow'], c['flownote'], FULL), Spacer(1, 6), P(c['how'][0]), P(c['how'][1])]
    s += [PageBreak(), P(c['s4'], 'h1')]
    for h, t in c['syn']: s += [KeepTogether([P(h, 'h2'), P(t)])]
    s += [PageBreak(), P(c['s5'], 'h1'), Image('man_main.png', width=FULL, height=FULL * 660 / 940), P(c['cap1'], 'cap')] + B(c['ui'])
    s += [PageBreak(), P(c['s6'], 'h1'), table([c['ph']] + c['params'], [30 * mm, 36 * mm, FULL - 66 * mm])]
    s += [PageBreak(), P(c['s7'], 'h1'), table([c['sh']] + c['cyl'], [34 * mm, FULL - 34 * mm]), Spacer(1, 8), P(c['lib']), P(c['own']), P(c['midi'])]
    s += [P(c['s8'], 'h1'), P(c['agei']), table([c['ah']] + c['age'], [20 * mm, FULL - 20 * mm])]
    s += [PageBreak(), P(c['s9'], 'h1'), P(c['lume'][0], 'lead'), P(c['lume'][1]), Image('man_lume.png', width=FULL, height=FULL * 200 / 940), P(c['cap2'], 'cap'),
          table([c['ph']] + c['lp'], [36 * mm, 30 * mm, FULL - 66 * mm])]
    s += [PageBreak(), P(c['s10'], 'h1'), P(c['rec1']), table([c['rh']] + c['rec'], [46 * mm, FULL - 46 * mm]), Spacer(1, 8), P(c['notes'], 'small'), P(c['credh'], 'h1')] + [P(t) for t in c['cred']]
    return s

state = {'footer': ''}
def page(canvas, doc):
    canvas.saveState()
    canvas.setStrokeColor(BRASS); canvas.setLineWidth(0.6); canvas.line(20 * mm, 16 * mm, W - 20 * mm, 16 * mm)
    canvas.setFont('Serif-Italic', 8); canvas.setFillColor(GREY)
    canvas.drawString(20 * mm, 11 * mm, state['footer']); canvas.drawRightString(W - 20 * mm, 11 * mm, str(doc.page))
    canvas.restoreState()

def cover(canvas, doc):
    canvas.saveState()
    canvas.setFillColor(colors.HexColor('#2a1309')); canvas.rect(0, 0, W, H, fill=1, stroke=0)
    canvas.setStrokeColor(colors.HexColor('#c8a14b')); canvas.setLineWidth(1.2); canvas.rect(12 * mm, 12 * mm, W - 24 * mm, H - 24 * mm)
    canvas.setLineWidth(0.4); canvas.rect(14 * mm, 14 * mm, W - 28 * mm, H - 28 * mm)
    canvas.setFillColor(colors.HexColor('#f4dfa0')); canvas.setFont('Serif-Bold', 46); canvas.drawCentredString(W / 2, H - 62 * mm, 'BAILARINA')
    canvas.setFont('Serif', 11); canvas.setFillColor(colors.HexColor('#e9dcc0'))
    canvas.drawCentredString(W / 2, H - 73 * mm, 'A PHYSICALLY MODELLED MUSIC BOX  ·  CAJA DE MÚSICA POR MODELADO FÍSICO')
    iw = W - 50 * mm; ih = iw * 860 / 940
    canvas.drawImage('/home/claude/out/Bailarina_interfaz_v6.png', 25 * mm, H - 84 * mm - ih, width=iw, height=ih)
    canvas.setFillColor(colors.HexColor('#f0e4cb')); canvas.setFont('Type', 20); canvas.drawCentredString(W / 2, 43 * mm, 'User manual  ·  Manual de uso')
    canvas.setFont('Serif-Bold', 13); canvas.setFillColor(colors.HexColor('#f4dfa0')); canvas.drawCentredString(W / 2, 30.5 * mm, 'Eloy Platas')
    canvas.setFont('Serif-Italic', 9.5); canvas.setFillColor(colors.HexColor('#c8a14b')); canvas.drawCentredString(W / 2, 22.5 * mm, 'English · Castellano')
    canvas.restoreState()

class SetFooter(Flowable):
    def __init__(self, t): super().__init__(); self.t = t
    def wrap(self, *a): return 0, 0
    def draw(self): state['footer'] = self.t

def divider(title, sub):
    return [Spacer(1, 90 * mm), Paragraph(title, ParagraphStyle('d', fontName='Type', fontSize=34, leading=40, textColor=WINE, alignment=TA_CENTER)),
            Spacer(1, 6 * mm), Paragraph(sub, ParagraphStyle('d2', fontName='Serif-Italic', fontSize=12, leading=16, textColor=BRASS, alignment=TA_CENTER)), PageBreak()]

doc = BaseDocTemplate('Bailarina_manual_EN_ES.pdf', pagesize=A4, leftMargin=20 * mm, rightMargin=20 * mm, topMargin=18 * mm, bottomMargin=22 * mm,
                      title='Bailarina — User manual / Manual de uso', author='Eloy Platas', subject='Physically modelled music box VST3')
frame = Frame(20 * mm, 22 * mm, FULL, H - 40 * mm, id='f', leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0)
doc.addPageTemplates([PageTemplate(id='cover', frames=[frame], onPage=cover), PageTemplate(id='body', frames=[frame], onPageEnd=page)])
story = [NextPageTemplate('body'), PageBreak()]
story += [SetFooter(L['en']['footer'])] + divider('English', 'User manual') + part('en')
story += [PageBreak(), SetFooter(L['es']['footer'])] + divider('Castellano', 'Manual de uso') + part('es')
doc.build(story)
