:name "madd"
:returns float

:param float X
:param float Multiply
:param float Add

SHADER
{
  return X * Multiply + Add;
}
