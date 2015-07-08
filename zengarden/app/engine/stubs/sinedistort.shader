:name "sine distort"
:returns vec2

:param float FreqX
:param float FreqY
:param float PhaseX
:param float PhaseY
:param float AmpX
:param float AmpY
:param vec2 Coord

SHADER
{
  const float Pi = 3.1415926535;
  vec2 freq = (Coord.yx + vec2(PhaseX, PhaseY)) * vec2(FreqX, FreqY) * 2.0 * Pi;
  return Coord + vec2(AmpX, AmpY) * sin(freq);
}
