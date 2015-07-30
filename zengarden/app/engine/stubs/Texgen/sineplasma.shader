:name "sine plasma"
:returns float

:param float FreqX
:param float FreqY
:param float PhaseX
:param float PhaseY
:param vec2 Coord

SHADER
{
  const float Pi2 = 2.0 * 3.1415926535;
  return 0.5 + 0.5 * (sin(FreqX * Pi2 * (Coord.x + PhaseX)) + cos(FreqY * Pi2 * (Coord.y + PhaseY)));
}
