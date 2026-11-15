#!/usr/bin/env python3
"""Genera Source/MelodyData.h a partir de los MIDI de tools/melodias/midi.
Los .abc de tools/melodias/abc se convierten antes con abc2midi (paquete abcmidi).
Uso: python3 tools/make_melodies.py      (necesita: pip install mido)
Para anadir una melodia: deja el .mid en tools/melodias/midi y anade una linea a LIBRARY."""
import mido, os, sys, math
HERE = os.path.dirname(os.path.abspath(__file__))
MIDI = os.path.join(HERE, 'melodias', 'midi')
LO, HI = 52, 100            # registro del peine: lo demas se pliega por octavas

# (archivo, titulo, autor, segundos maximos, transposicion, factor de tempo)
LIBRARY = [
 ('chopin_nocturne_op9_n2', 'Nocturno op. 9 n.º 2', 'F. Chopin', 125, 0, 1.0, dict(mode='piano')),
 ('gymnopedie_1', 'Gymnopédie n.º 1', 'E. Satie', 110, 12, 1.0),
 ('gymnopedie_3', 'Gymnopédie n.º 3', 'E. Satie', 110, 12, 1.0),
 ('gnossienne_1', 'Gnossienne n.º 1', 'E. Satie', 110, 12, 1.0),
 ('gnossienne_3', 'Gnossienne n.º 3', 'E. Satie', 110, 12, 1.0),
 ('muineira_chantada', 'Muiñeira de Chantada', 'Tradicional galega', 200, 12, 1.0),
 ('gal_muineira_berdoias', 'Muiñeira de Berdoias', 'Tradicional galega', 300, 12, 2.7, dict(mode='folk', bar=3.0, pickup=0.5, drone=48)),
 ('gal_muineira_boimorto', 'Muiñeira de Boimorto', 'Tradicional galega', 300, 12, 2.7, dict(mode='folk', bar=3.0, pickup=1.5, drone=48)),
 ('gal_muineira_molete', 'Muiñeira de Molete', 'Tradicional galega', 300, 12, 2.7, dict(mode='folk', bar=3.0, pickup=1.5, drone=48)),
 ('gal_xota', 'Xota', 'Tradicional galega', 300, 12, 2.8, dict(mode='folk', bar=3.0, pickup=1.0, drone=48)),
 ('gal_mazurca_bretona', 'Mazurca da Bretoña', 'Tradicional galega', 300, 12, 2.1, dict(mode='folk', bar=3.0, pickup=1.0, drone=48)),
 ('gal_mazurca_linares', 'Mazurca de Liñares', 'Tradicional galega', 300, 12, 2.1, dict(mode='folk', bar=3.0, pickup=2.0, drone=48)),
 ('gal_pasodobre_menduina', 'Pasodobre de Menduiña', 'Tradicional galega', 300, 12, 1.85, dict(mode='folk', bar=2.0, pickup=1.0, drone=48)),
 ('gal_pasodobre_xigantes', 'Pasodobre dos xigantes', 'Tradicional galega', 300, 12, 1.8, dict(mode='folk', bar=2.0, pickup=0.0, drone=48)),
 ('gal_danza_de_tenorio', 'Danza de Tenorio', 'Tradicional galega', 300, 12, 1.5, dict(mode='folk', bar=2.0, pickup=0.0, drone=48)),
 ('gal_a_xeito_de_marcha', 'A xeito de marcha', 'Tradicional galega', 300, 12, 1.3, dict(mode='folk', bar=2.0, pickup=0.0, drone=48)),
 ('frere_jacques', 'Frère Jacques', 'Tradicional · Francia', 200, 12, 1.0),
 ('au_clair_de_la_lune', 'Au clair de la lune', 'Tradicional · Francia', 200, 12, 1.0),
 ('pont_avignon', "Sur le pont d'Avignon", 'Tradicional · Francia', 200, 12, 1.0),
 ('scarborough_fair', 'Scarborough Fair', 'Tradicional · Inglaterra', 200, 12, 1.0),
 ('drunken_sailor', 'Drunken Sailor', 'Tradicional · Inglaterra', 200, 12, 1.0),
 ('auld_lang_syne', 'Auld Lang Syne', 'Tradicional · Escocia', 200, 12, 1.0),
 ('korobeiniki', 'Korobéiniki', 'Tradicional · Rusia', 200, 12, 1.0),
 ('o_tannenbaum', 'O Tannenbaum', 'Tradicional · Alemania', 200, 12, 1.0),
 ('lieber_augustin', 'O du lieber Augustin', 'Tradicional · Austria', 200, 12, 1.0),
 ('bella_ciao', 'Bella ciao', 'Tradicional · Italia', 200, 12, 1.0),
 ('dvorak_largo', 'Largo · Sinfonía del Nuevo Mundo', 'A. Dvořák', 200, 12, 1.0),
 ('wtk1-prelude1', 'Preludio en do mayor', 'J. S. Bach', 150, 12, 1.2),
 ('bach_air_bmv_1068', 'Aria de la suite n.º 3', 'J. S. Bach', 110, 0, 1.0),
 ('eine-kleine-nachtmusik-mvt1-5', 'Pequeña serenata nocturna', 'W. A. Mozart', 80, 0, 1.0),
 ('KV331_3_RondoAllaTurca', 'Marcha turca', 'W. A. Mozart', 210, 0, 2.2),
 ('Dans_l_antre_du_roi_de_la_montagne', 'En la gruta del rey de la montaña', 'E. Grieg', 100, 12, 1.0),
 ('SchumannOp15No07', 'Träumerei', 'R. Schumann', 130, 12, 1.15),
 ('debussy_Ste_Bergamesq_Clair', 'Claro de luna', 'C. Debussy', 110, 0, 1.0),
 ('waltz-op39-15', 'Vals op. 39 n.º 15', 'J. Brahms', 120, 0, 1.0, dict(mode='piano')),
 ('SchubertF-D839_AveMaria', 'Ave Maria', 'F. Schubert', 110, 0, 1.0),
]

def read(path):
    """Devuelve notas {t (s), tb (negras), note, vel, track, dur (s)}, bpm inicial y compas."""
    mid = mido.MidiFile(path)
    tempos, ts = [], None
    for tr in mid.tracks:
        tick = 0
        for msg in tr:
            tick += msg.time
            if msg.type == 'set_tempo': tempos.append((tick, msg.tempo))
            elif msg.type == 'time_signature' and ts is None: ts = (msg.numerator, msg.denominator)
    tempos.sort()
    if not tempos or tempos[0][0] > 0: tempos.insert(0, (0, 500000))
    def secs(tick):
        t, last, tempo = 0.0, 0, tempos[0][1]
        for tk, tp in tempos:
            if tk >= tick: break
            t += mido.tick2second(tk - last, mid.ticks_per_beat, tempo); last, tempo = tk, tp
        return t + mido.tick2second(tick - last, mid.ticks_per_beat, tempo)
    notes = []
    for ti, tr in enumerate(mid.tracks):
        tick, on = 0, {}
        for msg in tr:
            tick += msg.time
            if msg.type == 'note_on' and msg.velocity > 0 and msg.channel != 9:
                on[msg.note] = dict(t=secs(tick), tb=tick / mid.ticks_per_beat, note=msg.note, vel=msg.velocity, track=ti, dur=0.5)
                notes.append(on[msg.note])
            elif msg.type in ('note_off', 'note_on') and msg.note in on:
                on[msg.note]['dur'] = secs(tick) - on[msg.note]['t']; del on[msg.note]
    return notes, 60e6 / tempos[0][1], ts or (4, 4)

def groups(notes):
    g = {}
    for n in notes: g.setdefault(round(n['tb'], 3), []).append(n)
    return [g[k] for k in sorted(g)]

def simplify_piano(notes, ts):
    """Melodia = voz superior de la mano derecha. Acompanamiento = bajo en cada tiempo
    y una sola nota del primer acorde a contratiempo."""
    tracks = sorted({n['track'] for n in notes}, key=lambda t: -sum(x['note'] for x in notes if x['track'] == t) / sum(1 for x in notes if x['track'] == t))
    mel = [n for n in notes if n['track'] == tracks[0]]
    acc = [n for n in notes if n['track'] != tracks[0]]
    out = [max(g, key=lambda n: n['note']) for g in groups(mel)]
    if not acc: return out
    beat = 1.5 if (ts[1] == 8 and ts[0] % 3 == 0) else 4.0 / ts[1]
    origin = min(n['tb'] for n in acc) % beat
    used = set()
    for g in groups(acc):
        pos = (g[0]['tb'] - origin) / beat
        idx, frac = math.floor(pos + 0.02), (pos + 0.02) % 1.0
        if frac < 0.06: out.append(min(g, key=lambda n: n['note']))
        elif idx not in used:
            used.add(idx); out.append(max(g, key=lambda n: n['note']))
    return out

def simplify_folk(notes, bar, pickup, drone):
    """Melodia de gaita: sin adornos (notas de gracia), con un bordon de tonica y quinta
    como el ronco, en el primer tiempo y a mitad de compas."""
    track = min(n['track'] for n in notes)
    mel = [n for n in notes if n['track'] == track and n['dur'] > 0.09]
    out = [max(g, key=lambda n: n['note']) for g in groups(mel)]
    if drone is None: return out
    end = max(n['tb'] for n in out)
    spb = (max(n['t'] for n in out) - min(n['t'] for n in out)) / max(1e-9, end - min(n['tb'] for n in out))
    t0s, t0b = min(n['t'] for n in out), min(n['tb'] for n in out)
    tb = pickup
    while tb <= end + 1e-6:
        for off, nt, v in ((0.0, drone, 62), (bar / 2.0 if bar != 3 else 1.0, drone + 7, 50)):
            if tb + off <= end + 1e-6 and tb + off >= t0b - 1e-6:
                out.append(dict(t=t0s + (tb + off - t0b) * spb, tb=tb + off, note=nt, vel=v, track=99, dur=1.0))
        tb += bar
    return out

def convert(entry):
    fn, title, composer, max_s, tr, tf = entry[:6]
    opt = entry[6] if len(entry) > 6 else {}
    notes, bpm, ts = read(os.path.join(MIDI, fn + '.mid'))
    before = len(notes)
    if opt.get('mode') == 'piano': notes = simplify_piano(notes, ts)
    elif opt.get('mode') == 'folk': notes = simplify_folk(notes, opt['bar'], opt.get('pickup', 0.0), opt.get('drone'))
    notes = [(n['t'], n['note'], n['vel']) for n in notes]
    t0 = min(n[0] for n in notes)
    bar_s = 60.0 / bpm * (opt['bar'] if 'bar' in opt else ts[0] * 4.0 / ts[1])
    total = max(n[0] for n in notes) - t0
    if total > max_s:                      # corta en una barra de compas
        limit = math.floor(max_s / bar_s) * bar_s
        notes = [n for n in notes if n[0] - t0 < limit - 1e-6]
        end = limit
    else:
        end = math.ceil((total + 60.0 / bpm) / bar_s - 1e-6) * bar_s
    pins = {}
    for t, n, v in notes:
        n += tr
        while n < LO: n += 12
        while n > HI: n -= 12
        key = (round((t - t0) * bpm / 60.0, 3), n)
        pins[key] = max(pins.get(key, 0), v)
    out = sorted((b, n, v) for (b, n), v in pins.items())
    return dict(id=fn, title=title, composer=composer, bpm=bpm * tf, length=end * bpm / 60.0, pins=out, secs=end / tf, before=before)

def cid(s): return 'kPins_' + ''.join(c if c.isalnum() else '_' for c in s)
def cstr(s): return '"' + ''.join(c if 32 <= ord(c) < 127 and c not in '"\\' else ''.join('\\%03o' % b for b in c.encode('utf-8')) for c in s) + '"'

def main():
    mels = [convert(e) for e in LIBRARY]
    o = ['// GENERADO por tools/make_melodies.py - no editar a mano', '#pragma once', 'namespace mbox', '{',
         'struct RawPin { float beat; unsigned char note; unsigned char vel; };',
         'struct RawMelody { const char* title; const char* composer; double bpm; double lengthBeats; const RawPin* pins; int count; };', '']
    for m in mels:
        o.append('static const RawPin %s[] = {' % cid(m['id']))
        line = ''
        for b, n, v in m['pins']:
            fb = ('%.3f' % b).rstrip('0'); fb += '0' if fb.endswith('.') else ''
            line += '{%sf,%d,%d},' % (fb, n, v)
            if len(line) > 150: o.append(line); line = ''
        if line: o.append(line)
        o.append('};')
    o.append('static const RawMelody kGeneratedMelodies[] = {')
    for m in mels:
        o.append('    { %s, %s, %.3f, %.3f, %s, %d },' % (cstr(m['title']), cstr(m['composer']), m['bpm'], m['length'], cid(m['id']), len(m['pins'])))
    o += ['};', 'static const int kNumGeneratedMelodies = %d;' % len(mels), '} // namespace mbox', '']
    open(os.path.join(HERE, '..', 'Source', 'MelodyData.h'), 'w').write('\n'.join(o))
    for m in mels:
        ns = [p[1] for p in m['pins']]
        print('%-34s bpm %6.1f  %5.1f s  %4d puas (de %4d notas)  rango %d-%d' % (m['title'][:34], m['bpm'], m['secs'], len(ns), m['before'], min(ns), max(ns)))
main()
