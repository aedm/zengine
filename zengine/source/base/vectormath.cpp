#include <include/base/vectormath.h>
#include <cmath>

Vec2::Vec2() = default;

Vec2::Vec2(float x, float y) {
  this->x = x;
  this->y = y;
}

Vec2 Vec2::operator- (const Vec2& op) const {
  return {x - op.x, y - op.y};
}

Vec2 Vec2::operator+ (const Vec2& op) const {
  return {x + op.x, y + op.y};
}

Vec2 Vec2::operator/ (const Vec2& op) const {
  return {x / op.x, y / op.y};
}

Vec2 Vec2::operator* (const Vec2& op) const {
  return {x * op.x, y * op.y};
}

Vec2 Vec2::operator* (float f) const {
  return {x * f, y * f};
}

Vec2 Vec2::operator/ (float f) const {
  return *this * (1.0f / f);
}

Vec2& Vec2::operator*=(float f) {
  x *= f;
  y *= f;
  return *this;
}

bool Vec2::operator==(const Vec2& op) const {
  return x == op.x && y == op.y;
}

Vec2& Vec2::operator+=(const Vec2& op) {
  x += op.x;
  y += op.y;
  return *this;
}

Vec2& Vec2::operator-=(const Vec2& op) {
  x -= op.x;
  y -= op.y;
  return *this;
}

float& Vec2::operator[](int n) {
  float* a[]{ &x, &y };
  return *(a[n]);
}

float Vec2::operator[](int n) const {
  float a[]{ x, y };
  return a[n];
}

float Vec2::Length() const {
  return sqrtf(x*x + y*y);
}

Vec2 Vec2::Dot(const Vec2& op) const {
  return {x * op.x, y * op.y};
}

Vec3::Vec3() = default;

Vec3::Vec3(float x, float y, float z) {
  this->x = x;
  this->y = y;
  this->z = z;
}

float Vec3::Dot(const Vec3& op) const {
  return x * op.x + y * op.y + z * op.z;
}

Vec3 Vec3::operator* (float f) const {
  return {x * f, y * f, z * f};
}

Vec3 Vec3::operator- (const Vec3& op) const {
  return {x - op.x, y - op.y, z - op.z};
}

Vec3 Vec3::operator+ (const Vec3& op) const {
  return {x + op.x, y + op.y, z + op.z};
}

Vec3 Vec3::operator- () const {
  return {-x, -y, -z};
}

Vec3 Vec3::operator/(float f) const {
  return {x / f, y / f, z / f};
}

bool Vec3::operator==(const Vec3& op) const {
  return x == op.x && y == op.y && z == op.z;
}

float& Vec3::operator[](int n) {
  float* a[]{ &x, &y, &z };
  return *(a[n]);
}

float Vec3::operator[](int n) const {
  float a[]{ x, y, z };
  return a[n];
}

Vec3 Vec3::Cross(const Vec3& op) const {
  return {y * op.z - z * op.y, z * op.x - x * op.z, x * op.y - y * op.x};
}

float Vec3::LengthSquared() const {
  return x*x + y*y + z*z;
}

float Vec3::Length() const {
  return sqrtf(x*x + y*y + z*z);
}

Vec3 Vec3::Normal() const {
  return *this / Length();
}

Vec3& Vec3::Normalize() {
  *this /= Length();
  return *this;
}

Vec3& Vec3::operator/=(float f) {
  x /= f;
  y /= f;
  z /= f;
  return *this;
}

Vec3& Vec3::operator*=(float f) {
  x *= f;
  y *= f;
  z *= f;
  return *this;
}

Vec3& Vec3::operator+=(const Vec3& op) {
  x += op.x;
  y += op.y;
  z += op.z;
  return *this;
}

Vec4::Vec4() = default;

Vec4::Vec4(float x, float y, float z, float w) {
  this->x = x;
  this->y = y;
  this->z = z;
  this->w = w;
}

Vec4::Vec4(Vec3 v, float w) {
  this->x = v.x;
  this->y = v.y;
  this->z = v.z;
  this->w = w;
}

float Vec4::Dot(const Vec4& v) const {
  return x*v.x + y*v.y + z*v.z + w*v.w;
}

Vec4 Vec4::operator-(const Vec4& v) const {
  return {x - v.x, y - v.y, z - v.z, w - v.w};
}

bool Vec4::operator == (const Vec4& v) const {
  return x == v.x && y == v.y && z == v.z && w == v.w;
}

float& Vec4::operator[](int n) {
  float* a[]{ &x, &y, &z, &w };
  return *(a[n]);
}

float Vec4::operator[](int n) const {
  float a[]{ x, y, z, w };
  return a[n];
}

Vec3 Vec4::XYZ() const
{
  return {x, y, z};
}

Vec4 Vec4::operator*(const Matrix& m) const {
  return {
    x*m.m[0] + y*m.m[1] + z*m.m[2] + w*m.m[3],
    x*m.m[4] + y*m.m[5] + z*m.m[6] + w*m.m[7],
    x*m.m[8] + y*m.m[9] + z*m.m[10] + w*m.m[11],
    x*m.m[12] + y*m.m[13] + z*m.m[14] + w*m.m[15]
  };
}

Matrix::Matrix() = default;

float& Matrix::operator() (UINT y, UINT x) {
  return m[y * 4 + x];
}

Matrix Matrix::Translate(const Vec3& translateVector) {
  Matrix matrix;
  matrix.m[0] = 1;	matrix.m[1] = 0;	matrix.m[2] = 0;	matrix.m[3] = translateVector.x;
  matrix.m[4] = 0;	matrix.m[5] = 1;	matrix.m[6] = 0;	matrix.m[7] = translateVector.y;
  matrix.m[8] = 0;	matrix.m[9] = 0;	matrix.m[10] = 1;	matrix.m[11] = translateVector.z;
  matrix.m[12] = 0;	matrix.m[13] = 0;	matrix.m[14] = 0;	matrix.m[15] = 1;

  return matrix;
}

Matrix Matrix::Rotate(float angle, const Vec3& axis) {
  Matrix rot;
  const float sn = sinf(angle);
  const float cs = cosf(angle);
  const float nc = 1 - cs;
  rot.m[0] = nc * (axis.x * axis.x) + cs;
  rot.m[1] = nc * (axis.y * axis.x) - sn * axis.z;
  rot.m[2] = nc * (axis.z * axis.x) + sn * axis.y;
  rot.m[3] = 0;
  rot.m[4] = nc * (axis.x * axis.y) + sn * axis.z;
  rot.m[5] = nc * (axis.y * axis.y) + cs;
  rot.m[6] = nc * (axis.z * axis.y) - sn * axis.x;
  rot.m[7] = 0;
  rot.m[8] = nc * (axis.x * axis.z) - sn * axis.y;
  rot.m[9] = nc * (axis.y * axis.z) + sn * axis.x;
  rot.m[10] = nc * (axis.z * axis.z) + cs;
  rot.m[11] = 0;
  rot.m[12] = 0;
  rot.m[13] = 0;
  rot.m[14] = 0;
  rot.m[15] = 1;
  return rot;
}

Matrix Matrix::Rotate(const Quaternion& q) {
  /// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

  Matrix rot;

  const float sqw = q.s * q.s;
  const float sqx = q.vx * q.vx;
  const float sqy = q.vy * q.vy;
  const float sqz = q.vz * q.vz;

  rot.m[0] = sqx - sqy - sqz + sqw;
  rot.m[5] = -sqx + sqy - sqz + sqw;
  rot.m[10] = -sqx - sqy + sqz + sqw;

  float tmp1 = q.vx * q.vy;
  float tmp2 = q.vz * q.s;
  rot.m[1] = 2.0f * (tmp1 - tmp2);
  rot.m[4] = 2.0f * (tmp1 + tmp2);

  tmp1 = q.vx * q.vz;
  tmp2 = q.vy * q.s;
  rot.m[2] = 2.0f * (tmp1 + tmp2);
  rot.m[8] = 2.0f * (tmp1 - tmp2);

  tmp1 = q.vy * q.vz;
  tmp2 = q.vx * q.s;
  rot.m[6] = 2.0f * (tmp1 - tmp2);
  rot.m[9] = 2.0f * (tmp1 + tmp2);

  rot.m[3] = rot.m[7] = rot.m[11] = rot.m[12] = rot.m[13] = rot.m[14] = 0;
  rot.m[15] = 1;
  return rot;
}

Matrix Matrix::Scale(const Vec3& scaleVector) {
  Matrix matrix;
  matrix.m[0] = scaleVector.x;	matrix.m[1] = 0;	matrix.m[2] = 0;	matrix.m[3] = 0;
  matrix.m[4] = 0;	matrix.m[5] = scaleVector.y;	matrix.m[6] = 0;	matrix.m[7] = 0;
  matrix.m[8] = 0;	matrix.m[9] = 0;	matrix.m[10] = scaleVector.z;	matrix.m[11] = 0;
  matrix.m[12] = 0;	matrix.m[13] = 0;	matrix.m[14] = 0;				matrix.m[15] = 1;

  return matrix;
}

Matrix& Matrix::operator*=(const Matrix& other) {
  Vec4 col[4];
  for (UINT x = 0; x < 4; x++) {
    col[x] = Vec4(other.m[x + 0 * 4], other.m[x + 1 * 4],
      other.m[x + 2 * 4], other.m[x + 3 * 4]);
  }

  for (UINT y = 0; y < 4; y++) {
    Vec4 row(m[y * 4 + 0], m[y * 4 + 1], m[y * 4 + 2], m[y * 4 + 3]);
    for (UINT x = 0; x < 4; x++) {
      m[y * 4 + x] = row.Dot(col[x]);
    }
  }

  return *this;
}

Matrix Matrix::operator * (const Matrix& other) const {
  Matrix matrix = *this;
  matrix *= other;
  return matrix;
}

Vec4 Matrix::operator * (const Vec4& v) const {
  Vec4 ret;
  ret.x = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
  ret.y = m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w;
  ret.z = m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w;
  ret.w = m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w;
  return ret;
}

void Matrix::LoadIdentity() {
  m[0] = 1;	m[1] = 0;	m[2] = 0;	m[3] = 0;
  m[4] = 0;	m[5] = 1;	m[6] = 0;	m[7] = 0;
  m[8] = 0;	m[9] = 0;	m[10] = 1;	m[11] = 0;
  m[12] = 0;	m[13] = 0;	m[14] = 0;	m[15] = 1;
}

Matrix Matrix::Ortho(float x1, float y1, float x2, float y2) {
  const float rx = 1.0f / (x2 - x1);
  const float ry = 1.0f / (y2 - y1);
  Matrix m;
  m.m[0] = 2.0f*rx;	m.m[1] = 0;		      m.m[2] = 0;	    m.m[3] = -(x1 + x2) * rx;
  m.m[4] = 0;		    m.m[5] = -2.0f*ry;	m.m[6] = 0;	    m.m[7] = (y1 + y2) * ry;
  m.m[8] = 0;		    m.m[9] = 0;		      m.m[10] = -1;	  m.m[11] = 0;
  m.m[12] = 0;		  m.m[13] = 0;		    m.m[14] = 0;	  m.m[15] = 1;
  return m;
}

Matrix Matrix::Ortho(float x1, float y1, float x2, float y2, float near, float far) {
  const float rx = 1.0f / (x2 - x1);
  const float ry = 1.0f / (y2 - y1);
  const float rz = 1.0f / (far - near);
  Matrix m;
  m.m[0] = 2.0f*rx;	m.m[1] = 0;		      m.m[2] = 0;	      m.m[3] = -(x1 + x2) * rx;
  m.m[4] = 0;		    m.m[5] = 2.0f*ry;	  m.m[6] = 0;	      m.m[7] = -(y1 + y2) * ry;
  m.m[8] = 0;		    m.m[9] = 0;		      m.m[10] = -2 * rz;	m.m[11] = -(far + near) * rz;
  m.m[12] = 0;		  m.m[13] = 0;		    m.m[14] = 0;	    m.m[15] = 1;
  return m;
}


Matrix Matrix::Projection(float fovY, float zFar, float zNear, float aspectRatio) {
  /// Create projection matrix
  /// https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/

  const float f = 1.0f / tanf(fovY / 2.0f);
  const float a = zNear - zFar;

  Matrix m;
  m.m[0] = f / aspectRatio;	m.m[1] = 0;		m.m[2] = 0;	    m.m[3] = 0;
  m.m[4] = 0;		            m.m[5] = f;	  m.m[6] = 0;	    m.m[7] = 0;
  m.m[8] = 0;		            m.m[9] = 0;		m.m[10] = (zFar + zNear) / a;
  m.m[11] = 2.0f * zFar * zNear / a;
  m.m[12] = 0;	            m.m[13] = 0;  m.m[14] = -1;	  m.m[15] = 0;
  return m;
}

Matrix Matrix::LookAt(const Vec3& position, const Vec3& target, const Vec3& up) {
  /// Generate view matrix for position camera 	
  /// http://wiki.delphigl.com/index.php/gluLookAt
  const float epsilon = 0.001f;

  const Vec3 f = -(target - position).Normal();
  Vec3 s = -f.Cross(up.Normal());
  float sl = s.Length();
  if (fabs(sl) < epsilon) {
    s = -f.Cross(Vec3(up.y, up.z, up.x).Normal());
    sl = s.Length();
  }
  s /= sl;
  const Vec3 u = s.Cross(f);

  Matrix m;
  m.m[0] = s.x;	  m.m[1] = s.y;	  m.m[2] = s.z;	  m.m[3] = 0;
  m.m[4] = u.x;	  m.m[5] = u.y;	  m.m[6] = u.z;	  m.m[7] = 0;
  m.m[8] = -f.x;	m.m[9] = -f.y;	m.m[10] = -f.z;	m.m[11] = 0;
  m.m[12] = 0;		m.m[13] = 0;		m.m[14] = 0;		m.m[15] = 1;
  return Translate(-position) * m;
}

Quaternion Quaternion::FromEuler(float roll, float pitch, float yaw) {
  const float rr = 0.5f*roll;
  const float pp = 0.5f*pitch;
  const float yy = 0.5f*yaw;
  const float sr = sinf(rr); const float cr = cosf(rr);
  const float sp = sinf(pp); const float cp = cosf(pp);
  const float sy = sinf(yy); const float cy = cosf(yy);

  const float vx = sr*cp*cy - cr*sp*sy;
  const float vy = cr*sp*cy + sr*cp*sy;
  const float vz = cr*cp*sy - sr*sp*cy;
  const float s = cr*cp*cy + sr*sp*sy;
  return Quaternion(s, vx, vy, vz);
}

void Quaternion::ToEuler(float& oRoll, float& oPitch, float& oYaw) const
{
  const float a = 2.0f  *  (s * vy - vx * vz);
  if (a < 1.0f) {
    if (-1.0f < a) {
      oRoll = atan2f(2.0f * (vy * vz + s * vx), 1.0f - 2.0f * (vx * vx + vy * vy));
      oPitch = asinf(a);
      oYaw = atan2f(2.0f * (vx * vy + s * vz), 1.0f - 2.0f * (vy * vy + vz * vz));
    }
    else {
      oRoll = -atan2f(2.0f * (vx * vy - s * vz), 1.0f - 2.0f * (vx * vx + vz * vz));
      oPitch = Pi / -2.0f;
      oYaw = 0.0f;
    }
  }
  else {
    oRoll = atan2f(2.0f * (vx * vy - s * vz), 1.0f - 2.0f * (vx * vx + vz * vz));
    oPitch = Pi / 2.0f;
    oYaw = 0.0f;
  }
}

Quaternion::Quaternion(Quaternion& q1, Quaternion& q2, float ratio) {
  const float cosom = q1.s * q2.s + q1.vx * q2.vx + q1.vy * q2.vy + q1.vz * q2.vz;
  float sclp, sclq;
  if (cosom < 0.9999f) {
    float omega = acosf(cosom);
    float sinom = sinf(omega);
    sclp = sinf((1.0f - ratio) * omega) / sinom;
    sclq = sinf(ratio * omega) / sinom;
  }
  else {
    sclp = 1.0f - ratio;
    sclq = ratio;
  }

  s = q1.s * sclp + q2.s * sclq;
  vx = q1.vx * sclp + q2.vx* sclq;
  vy = q1.vy * sclp + q2.vy * sclq;
  vz = q1.vz * sclp + q2.vz * sclq;
}

Quaternion::Quaternion() = default;

Quaternion::Quaternion(float _S, float _Vx, float _Vy, float _Vz)
  : s(_S), vx(_Vx), vy(_Vy), vz(_Vz) {}

Quaternion& Quaternion::operator *= (Quaternion& q) {
  const float bs = s*q.s - vx*q.vx - vy*q.vy - vz*q.vz;
  const float bx = s*q.vx + vx*q.s + vy*q.vz - vz*q.vy;
  const float by = s*q.vy + vy*q.s + vz*q.vx - vx*q.vz;
  const float bz = s*q.vz + vz*q.s + vx*q.vy - vy*q.vx;

  s = bs;
  vx = bx;
  vy = by;
  vz = bz;
  return *this;
}

Quaternion Quaternion::operator *(const Quaternion &q) const
{
  const float bs = s*q.s - vx*q.vx - vy*q.vy - vz*q.vz;
  const float bx = s*q.vx + vx*q.s + vy*q.vz - vz*q.vy;
  const float by = s*q.vy + vy*q.s + vz*q.vx - vx*q.vz;
  const float bz = s*q.vz + vz*q.s + vx*q.vy - vy*q.vx;
  return Quaternion(bs, bx, by, bz);
}

Quaternion& Quaternion::operator = (const Quaternion& q) = default;

Quaternion Quaternion::Conjugate() const
{
  return Quaternion(s, -vx, -vy, -vz);
}
