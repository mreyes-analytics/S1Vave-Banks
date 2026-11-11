# S1Vave-Banks — Bancos y Variaciones

Fork de [S1Vave](https://github.com/AndrewGG88/S1Vave) para el M-VAVE FM-1 que agrega:

- **4 bancos (A, B, C, D) de 8 patrones** → 32 patrones en total (antes 8).
- **4 variaciones por patrón, en pares**: cada variación guarda un tramo (inicio/fin) para el **track A** y otro para el **track B**, y los dos cambian juntos.

Todo lo demás funciona igual que S1Vave.

## Atajos: mantener SAVE

| Mantener **SAVE** + | Hace |
|---|---|
| Teclas blancas **1–8** | Ir al patrón 1–8 **del banco elegido** |
| Teclas negras **1–8** | Copiar el patrón actual a ese lugar del banco elegido |
| Teclas blancas **9 / 10 / 11 / 12** | Elegir banco **A / B / C / D** |
| Teclas blancas **13 / 14 / 15 / 16** | Variación **1 / 2 / 3 / 4** (A y B juntos) |

- Tocando (PLAY), el patrón y la variación cambian **al terminar la vuelta del track A**, para no romper el ritmo. Arriba se ve `A1>B3` (patrón en cola) o `V2>3` (variación en cola) y la tecla parpadea.
- Al cambiar de variación, si el tramo de B es distinto, B arranca su tramo nuevo en el mismo momento que A.
- Detenido, el cambio es inmediato.
- Luces: el banco y la variación activos encendidos; los demás tenues.
- Los patrones se nombran con banco + número: `A1` … `D8`. Arriba siempre se ve la variación que suena (`V2`).

## Página LOOP (presionar SEQ hasta llegar a LOOP, es la última)

El ciclo de SEQ queda: TRACKS → STEPS → STEP+ → TRACK → PATTERNS → **LOOP**.

| Control | Hace |
|---|---|
| **ALGORITHM** | Elegir si editas el track **A** o el **B** |
| **K3 VAR** | Qué variación **editas** (no cambia lo que suena) |
| **K1 START** | Primer step de esa variación en ese track |
| **K2 END** | Último step (girando hasta el final aparece **FIN** = hasta el último step del track) |
| **K4 PLAY** | Manda a sonar la variación que estás editando (al terminar la vuelta) |

La línea de abajo dice qué editas, por ejemplo `EDIT 3 A: 33-48 (16)`.

Ejemplo, patrón A1 con track A de 64 steps y B de 16:

| Variación | Track A | Track B |
|---|---|---|
| 1 | 1 – FIN (todo, igual que antes) | completo |
| 2 | 17 – 32 | completo |
| 3 | 33 – 48 | 9 – 16 (editado) |
| 4 | 49 – 64 | completo |

Defaults: en A, la 1 es todo el track y la 2–4 son 17–32, 33–48 y 49–64; en B las 4 son el track completo. Así, un patrón que no tocas suena igual que antes.

## Flujo en vivo

1. Suena la variación 2.
2. Vas a STEPS, página 3 (steps 33–48) y metes notas: **no suenan** todavía.
3. **SAVE + blanca 15**: al terminar la vuelta, A y B pasan a la variación 3 con tus notas nuevas.

- Si una variación empieza después del final del track, ese track la toca completa (la pantalla dice `PAST END`).
- Las variaciones (rangos de A y B, y la activa) se guardan con el patrón.

## Memoria

- **Banco A** = los mismos 8 lugares de siempre (`0xF4000`): tus patrones guardados no se pierden.
- **Bancos B–D** = 24 sectores libres de la zona de datos (`0x99000`–`0xB0FFF`, dentro de `FL_DATA`, después de los programas). S1Vave no usaba esa zona.
- Un patrón guardado con S1Vave original se carga sin problema; sus variaciones quedan con los valores default.
- En el SMK-37 sigue habiendo un solo banco (8 patrones); las variaciones sí funcionan.

## Probar sin el aparato

```
sh tests/run_host.sh tests/seq_banks.txt
```
