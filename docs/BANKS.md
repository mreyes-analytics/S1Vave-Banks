# S1Vave-Banks — Bancos y Variaciones

Fork de [S1Vave](https://github.com/AndrewGG88/S1Vave) para el M-VAVE FM-1 que agrega:

- **4 bancos (A, B, C, D) de 8 patrones** → 32 patrones en total (antes 8).
- **4 variaciones por patrón, en pares**: cada variación guarda un tramo (inicio/fin) para el **track A** y otro para el **track B**, y los dos cambian juntos.
- **Cadenas**: de variaciones (se guarda con el patrón) y de patrones (para tocar en vivo).
- **Modo de voz por variación**: Mono, Unison, Poly o Chord, sin tocar tu sonido.
- **Página CHORD** (botón SEL): el acorde que toca el modo Chord.

Todo lo demás funciona igual que S1Vave.

## Atajos: mantener SAVE

| Mantener **SAVE** + | Hace |
|---|---|
| Teclas blancas **1–8** | Ir al patrón 1–8 **del banco elegido** |
| Teclas negras **1–8** | Copiar el patrón actual a ese lugar del banco elegido |
| Teclas blancas **9 / 10 / 11 / 12** | Elegir banco **A / B / C / D** |
| Teclas blancas **13 / 14 / 15 / 16** | Variación **1 / 2 / 3 / 4** (A y B juntos) |

**Cadenas:** mientras mantienes SAVE, presiona varias teclas una tras otra y suelta SAVE.

| Mantén SAVE y presiona | Suena |
|---|---|
| `14, 14, 15, 16` | V2 → V2 → V3 → V4 → y se repite (una vuelta cada una) |
| `1, 1, 2` | A1 → A1 → A2 → y se repite |
| `1, 2, [blanca 10], 3` | A1 → A2 → B3 (puedes cambiar de banco a media cadena) |

- Cadena de variaciones: hasta 8 teclas. Se guarda con el patrón.
- Cadena de patrones: hasta 16 teclas. No se guarda (se borra al apagar).
- Si un patrón tiene cadena de variaciones, la toca completa antes de pasar al siguiente patrón de la cadena.
- Presionar **una sola** tecla de variación o de patrón apaga esa cadena y funciona como siempre.
- Con STOP, la cadena de patrones se queda donde iba; con PLAY sigue.
- El **MUTE** de A y B se queda al cambiar de patrón mientras suena (a mano o por la cadena): si B está en mute en A1, sigue en mute en A2.
- Mientras mantienes SAVE, la línea de abajo muestra la cadena que vas armando (`VAR CHAIN 2 2 3 4`, `CHAIN A1 A1 A2`).

- Tocando (PLAY), el patrón y la variación cambian **al terminar la vuelta del track A**, para no romper el ritmo. Arriba se ve `A1>B3` (patrón en cola) o `V2>3` (variación en cola) y la tecla parpadea.
- Al cambiar de variación, si el tramo de B es distinto, B arranca su tramo nuevo en el mismo momento que A.
- Detenido, el cambio es inmediato.
- Luces: el banco y la variación activos encendidos; los demás tenues.
- Los patrones se nombran con banco + número: `A1` … `D8`. Arriba siempre se ve la variación que suena (`V2`).

## Moverte entre vistas de SEQ y borrar patrones

- **Tocar SEQ** = siguiente vista. **Mantener SEQ y girar SELECT** = vista anterior o siguiente (de STEP+ a STEPS es un giro a la izquierda).
- Vista **PATTERNS**: con **K1** eliges el patrón y giras **K2 CLEAR** dos veces (`AGAIN: CLEAR B3`) → queda vacío. Si está sonando, se borra al dar STOP. **EDIT + OCT−** (deshacer) no lo recupera.

## Página LOOP (presionar SEQ hasta llegar a LOOP, es la última)

El ciclo de SEQ queda: TRACKS → STEPS → STEP+ → TRACK → PATTERNS → **LOOP** → **LOOP+**.

| Control | Hace |
|---|---|
| **ALGORITHM** | Elegir si editas el track **A** o el **B** |
| **K3 VAR** | Qué variación **editas** (no cambia lo que suena) |
| **K1 START** | Primer step de esa variación en ese track |
| **K2 END** | Último step (girando hasta el final aparece **FIN** = hasta el último step del track) |
| **K4 VOICE** | Modo de voz de esa variación en ese track: `SOUND` (el de tu sonido), `MONO`, `UNISON`, `POLY`, `CHORD` |

La línea de abajo dice qué editas, por ejemplo `EDIT 3 A: 33-48 (16)`.

Ejemplo, patrón A1 con track A de 64 steps y B de 16:

| Variación | Track A | Track B |
|---|---|---|
| 1 | 1 – FIN (todo, igual que antes) | completo |
| 2 | 17 – 32 | completo |
| 3 | 33 – 48 | 9 – 16 (editado) |
| 4 | 49 – 64 | completo |

Defaults: en A, la 1 es todo el track y la 2–4 son 17–32, 33–48 y 49–64; en B las 4 son el track completo. Así, un patrón que no tocas suena igual que antes.

## Página LOOP+ (la que sigue después de LOOP)

Para sacar variaciones de una misma idea: copiar una variación a otra y transponerla.

| Perilla | Hace |
|---|---|
| **K1 VAR** | La variación que editas |
| **K2 TO** | A cuál la copias |
| **K3 COPY** | Copia VAR en TO (si TO ya tiene notas, gira otra vez: `AGAIN: OVERWRITE V2`) |
| **K4 TRANSP** | Transpone la variación VAR, de −24 a +24 semitonos |

- Copia las notas de cada step (gate, velocidad, ratchet, chance) y también el VOICE y el TRANSP. Respeta el inicio y fin de la variación destino: si es más corta, copia solo lo que cabe y nunca pisa otra variación.
- Es por track: ALGORITHM elige A o B.
- El TRANSP de la variación se suma al TRANSP del track (vista TRACK). En STEPS ves las notas sin transponer; al sonar suben o bajan.
- **EDIT + OCT−** deshace la copia.

Ejemplo: programas V1, en LOOP+ pones VAR 1, TO 2 y giras COPY. Luego VAR 2, TRANSP +5. En LOOP le pones VOICE = CHORD. Ya tienes V2 como la misma frase, 5 semitonos arriba y en acordes.

## Página CHORD (presionar SEL hasta llegar a CHORD)

SEL ahora recorre: ADV MIX → ADV FX → **CHORD**.

| Perilla | Hace |
|---|---|
| **K1 TYPE** | Tipo de acorde de un golpe: `MAJ`, `MIN`, `SUS4`, `7`, `MIN7`, `POWER`, `OCT` |
| **K2 / K3 / K4 (NOTE 2 / 3 / 4)** | Las notas que el acorde agrega: `OFF` o de −12 a +12 semitonos sobre la nota que tocas (+3 tercera menor, +4 tercera mayor, +7 quinta, +12 octava) |

- Si mueves K2–K4 a mano, TYPE dice `CUSTOM`.
- El acorde es parte del **sonido**: aplica a todas las variaciones que usen `CHORD`. Para conservarlo, guarda el sonido (SAVE dos veces).
- Para el Unison: SEL → **ADV MIX**, K1 SPREAD (abre el estéreo) y K2 DETUNE (más gordo).

## Escribir en silencio

En STEPS / STEP+ las teclas escriben **sin sonar** cuando lo que escriben no se está oyendo:

- el track está en **MUTE**, o
- el secuenciador está sonando y el cursor está **fuera de la variación que suena** (por ejemplo, suena V1 = 1–16 y escribes en el step 20, que es de V2).

Dentro de la variación que suena, o con el secuenciador detenido, las teclas suenan como siempre. Fuera de STEPS (HOME, TRACKS, LOOP, páginas de sonido) las teclas solo tocan y siempre suenan.

Para B:

1. Mantén **GLO** + **blanca 2** → B en mute (blanca 1 = A).
2. En STEPS, con B elegido (ALGORITHM), escribe tus notas: no se oyen y B no suena.
3. **GLO + blanca 2** otra vez → B entra con lo que programaste.

## Flujo en vivo

1. Suena la variación 2.
2. Vas a STEPS, página 3 (steps 33–48) y metes notas: **no suenan** todavía.
3. **SAVE + blanca 15**: al terminar la vuelta, A y B pasan a la variación 3 con tus notas nuevas.

- Si una variación empieza después del final del track, ese track la toca completa (la pantalla dice `PAST END`).
- Las variaciones (rangos de A y B, modo de voz, la activa y la cadena) se guardan con el patrón.
- El modo de voz por variación se pone "encima" del sonido: tu programa no cambia ni se guarda distinto. Si B toca el sonido de A, usa el modo de voz de A.
- Si dejas un patrón con cambios sin guardar y regresas a él mientras suena (a mano o por la cadena), regresa con tus cambios. Se guardan al dar STOP. Solo puede quedar **un** patrón así pendiente a la vez: si intentas dejar un segundo, aparece `STOP ONCE` (y la cadena de patrones se apaga). Guarda con SAVE antes de tocar en vivo.

## Memoria

- **Banco A** = los mismos 8 lugares de siempre (`0xF4000`): tus patrones guardados no se pierden.
- **Bancos B–D** = 24 sectores libres de la zona de datos (`0x99000`–`0xB0FFF`, dentro de `FL_DATA`, después de los programas). S1Vave no usaba esa zona.
- Un patrón guardado con S1Vave original o con S1Vave-Banks 1 se carga sin problema; lo nuevo queda con los valores default.
- En el SMK-37 sigue habiendo un solo banco (8 patrones); las variaciones sí funcionan.

## Probar sin el aparato

```
sh tests/run_host.sh tests/seq_banks.txt
sh tests/run_host.sh tests/seq_chains.txt
```
