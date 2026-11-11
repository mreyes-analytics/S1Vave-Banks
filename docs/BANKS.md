# S1Vave-Banks — Bancos y Variaciones

Fork de [S1Vave](https://github.com/AndrewGG88/S1Vave) para el M-VAVE FM-1 que agrega:

- **4 bancos (A, B, C, D) de 8 patrones** → 32 patrones en total (antes 8).
- **4 variaciones por patrón**: cada una toca un tramo del track A (inicio y fin propios).

Todo lo demás funciona igual que S1Vave.

## Atajos: mantener SAVE

| Mantener **SAVE** + | Hace |
|---|---|
| Teclas blancas **1–8** | Ir al patrón 1–8 **del banco elegido** |
| Teclas negras **1–8** | Copiar el patrón actual a ese lugar del banco elegido |
| Teclas blancas **9 / 10 / 11 / 12** | Elegir banco **A / B / C / D** |
| Teclas blancas **13 / 14 / 15 / 16** | Variación **1 / 2 / 3 / 4** |

- Tocando (PLAY), el patrón y la variación cambian **al terminar la vuelta actual**, para no romper el ritmo. En la pantalla se ve `A1>B3` (patrón en cola) o `V2>3` (variación en cola) y la tecla parpadea.
- Detenido, el cambio es inmediato.
- Luces: el banco y la variación activos encendidos; los demás tenues.
- Los patrones se nombran con banco + número: `A1` … `D8`.

## Página LOOP (presionar SEQ hasta llegar a LOOP)

El ciclo de SEQ queda: TRACKS → STEPS → STEP+ → TRACK → PATTERNS → **LOOP**.

| Perilla | Hace |
|---|---|
| **K1 START** | Primer step de la variación activa |
| **K2 END** | Último step (girando hasta el final aparece **FIN** = hasta el último step del track) |
| **K3 VAR** | Cambia de variación (1–4) |

Ejemplo con un track A de 64 steps:

| Variación | Default | Ejemplo editado |
|---|---|---|
| 1 | 1 – FIN (todo el track, igual que antes) | 1 – 16 |
| 2 | 17 – 32 | 17 – 28 (un fill de 12) |
| 3 | 33 – 48 | 33 – 48 |
| 4 | 49 – 64 | 49 – 64 |

- Las variaciones son del **track A**; el track B sigue tocando su largo completo.
- Si una variación empieza después del final del track, toca el track completo (la pantalla dice `PAST END`).
- Las variaciones (inicio, fin y la activa) se guardan con el patrón.

## Memoria

- **Banco A** = los mismos 8 lugares de siempre (`0xF4000`): tus patrones guardados no se pierden.
- **Bancos B–D** = 24 sectores libres de la zona de datos (`0x99000`–`0xB0FFF`, dentro de `FL_DATA`, después de los programas). S1Vave no usaba esa zona.
- Un patrón guardado con S1Vave original se carga sin problema; sus variaciones quedan con los valores default.
- En el SMK-37 sigue habiendo un solo banco (8 patrones); las variaciones sí funcionan.

## Probar sin el aparato

```
sh tests/run_host.sh tests/seq_banks.txt
```
