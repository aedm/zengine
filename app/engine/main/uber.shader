:name "UberShader"
:returns void

const int poissonCount = 109;
//const int poissonCount = 8;
vec3 poissonDisk[109] = vec3[](
  vec3(0, 0, 0),
  vec3(0.22047275640740338, 2.0245790958599343, 2.0365482443858056),
  vec3(1.900583171653274, 0.8678589504857577, 2.0893529022905297),
  vec3(-1.9111342677894392, -0.8784366771723029, 2.1033509420257457),
  vec3(-1.8592811322756226, 1.5845650570325738, 2.442902525440744),
  vec3(1.338684649466444, -2.255601218936519, 2.622939848640263),
  vec3(-0.6305880632128513, -2.663458632498145, 2.737088451712042),
  vec3(3.5146841875215813, -1.1116698498293829, 3.6863009634366426),
  vec3(-1.1738619097095722, 3.500926129302922, 3.692483655468626),
  vec3(-4.083103788019985, 1.525758148890592, 4.358861602832693),
  vec3(1.0685811395354463, -4.252083865605272, 4.3842995851004005),
  vec3(3.770769770633212, 2.3828847512799847, 4.46058789858511),
  vec3(-3.167988111232795, -3.3698237131258253, 4.625133568931545),
  vec3(-4.309915714281189, -1.6789864375563148, 4.62540473058369),
  vec3(3.024621039868947, -3.534616527615796, 4.652079796404255),
  vec3(1.6822410973868038, 4.563538856957873, 4.863725116482379),
  vec3(4.956773480354919, 0.5193108096226275, 4.983902793247549),
  vec3(-3.5280571142989494, 4.259692348380241, 5.531018523256328),
  vec3(-0.4289491118892883, -5.62638268864341, 5.642710297335569),
  vec3(5.112339386236091, -3.0807058634209383, 5.968815847132239),
  vec3(-0.1590035320584562, 5.987561939534661, 5.989672787637997),
  vec3(-6.145121025794879, 1.236293969728706, 6.2682481764248825),
  vec3(2.9732809186343445, -5.558916628830428, 6.304121945790908),
  vec3(-5.477394176265758, -3.35390663057834, 6.422658067251237),
  vec3(5.3223214840677, 3.777534294406916, 6.5266278678341205),
  vec3(-2.242253219597181, 6.172579880055434, 6.567224823048094),
  vec3(-3.6593925061339867, -5.490016569526658, 6.597835663884546),
  vec3(-5.67466277091653, 3.786984076002412, 6.822246400968079),
  vec3(-6.736625737315819, -1.1921316508606026, 6.841294044086216),
  vec3(3.2290475531019673, 6.053733984583982, 6.861081784696992),
  vec3(6.972018676218653, -1.0731175965286255, 7.054121192432199),
  vec3(0.7725370650438954, -7.467570494794941, 7.507424499227322),
  vec3(-4.637118641160582, 6.187442683052019, 7.7322258275513995),
  vec3(-1.6007859801086823, -7.679177696615017, 7.844251771220888),
  vec3(7.471644340306785, 2.8904492718352195, 8.011252470187864),
  vec3(8.06987601755634, 0.8879420156237288, 8.11857992273531),
  vec3(0.1713703607995889, 8.170770887149235, 8.172567815004417),
  vec3(4.960034088225001, -6.671618239686888, 8.313388484395324),
  vec3(-7.6222768497517155, -3.797043027214272, 8.51567026867395),
  vec3(2.858803854134326, 8.151075691258889, 8.637869783751364),
  vec3(-8.285834025675266, 2.4662015063836407, 8.645067690372755),
  vec3(-3.799685797761061, 8.127748355499776, 8.972062499338511),
  vec3(-1.8617690793099158, 8.784188713039185, 8.97931820635395),
  vec3(-7.439776743889347, 5.091381962988933, 9.01512330985915),
  vec3(6.2645878102140955, 6.528727750341901, 9.04816812785701),
  vec3(-9.077460042766655, -0.38171232726658033, 9.085482107671142),
  vec3(8.822309705612017, -2.192801419724784, 9.090738507298681),
  vec3(-6.023159207614224, -7.1188622891083, 9.325054805820514),
  vec3(8.18413994791409, -4.5742404719489755, 9.375703844632639),
  vec3(7.222399402592796, -6.4145574213993815, 9.659689438227435),
  vec3(1.2442620080778397, -9.69825334767933, 9.777745442611975),
  vec3(3.8634311307967266, -8.988835190525666, 9.78392856601283),
  vec3(9.043145154057456, 4.347757125880349, 10.034015462565135),
  vec3(-8.150624497670508, -6.175452217556691, 10.22589310492499),
  vec3(1.5862422107175043, 10.358905884984786, 10.479651496353988),
  vec3(-6.922561054435798, 7.874879316871709, 10.485016729013125),
  vec3(-4.3545730222179895, -9.564199781285705, 10.50886405194083),
  vec3(-0.9553311728857494, -10.561957536499069, 10.605074476527582),
  vec3(-10.475685988669538, 1.8984352157374076, 10.646316414683493),
  vec3(-9.763686262636504, 4.387127163068186, 10.704039152587564),
  vec3(-10.66766425221022, -1.8413045819552387, 10.825408221467363),
  vec3(-1.8732716888775762, 10.860957937191294, 11.021322703459365),
  vec3(6.362345948563014, 9.0220801153568, 11.039808674841824),
  vec3(-4.97605097405399, 9.892628296613303, 11.073625328289642),
  vec3(11.252568624407594, 2.228405349978324, 11.471098075198128),
  vec3(3.5464970204031587, -11.040782849410354, 11.596401469575046),
  vec3(11.65144486965313, -0.6679322824704421, 11.67057415402226),
  vec3(-9.068295448639983, 7.368523761807523, 11.68456780425133),
  vec3(6.87084435029044, -9.657900728220595, 11.852575608789929),
  vec3(-10.849542372244915, -5.0102749466790275, 11.950540771381744),
  vec3(9.731252088627546, 7.0239652625702504, 12.001389720453691),
  vec3(4.0032498659397575, 11.544858317875335, 12.219237417652625),
  vec3(0.7517389228341926, -12.280630065325518, 12.303616793832656),
  vec3(-9.61320899844725, -7.717559281101709, 12.327794145959215),
  vec3(10.883834296053415, -6.067180642212424, 12.460679352635061),
  vec3(-8.000528570284361, -9.712113020800148, 12.583067858544357),
  vec3(11.323676181616072, 5.536539437864164, 12.604717815688076),
  vec3(-3.233057055473896, -12.188648784559692, 12.610147386818998),
  vec3(-11.66036084317467, 5.028751721780516, 12.698517979368757),
  vec3(0.48675275969128506, 12.700675062987017, 12.709999028506939),
  vec3(6.092224731680588, 11.177294451287985, 12.729772717220593),
  vec3(-6.306165140838548, -11.097846313585977, 12.764400165440644),
  vec3(-1.2471000320625727, -12.725162945262063, 12.786126484336103),
  vec3(8.207299409123667, 9.854727073611425, 12.82479665672605),
  vec3(-12.824184389911018, 0.4209497737767265, 12.831091301151282),
  vec3(12.06047066842276, -4.3959988034249236, 12.836656816461172),
  vec3(-8.228061312405876, 9.943178219988653, 12.906114290334138),
  vec3(-12.60544095526738, 3.1095635025242956, 12.983317251494833),
  vec3(9.588858614682973, -8.933479662500119, 13.105467119209568),
  vec3(13.079591508915996, -2.1214265700926873, 13.250515640245954),
  vec3(-12.720124627775114, -3.733643324811659, 13.256759144792579),
  vec3(-5.348892863781776, 12.240278184618496, 13.357958859985423),
  vec3(12.886834193005463, 3.773217913624702, 13.427869113962688),
  vec3(5.259334716542089, -12.642913277625025, 13.693204804068769),
  vec3(-10.621906137640657, 9.047871859840296, 13.953095541453807),
  vec3(-11.888954352829106, -7.594152371806615, 14.10738763378501),
  vec3(-3.1812149703773436, 13.888549388345545, 14.248225601816753),
  vec3(-9.962195949854516, -10.330721973375827, 14.35162585334804),
  vec3(-14.215210461638275, -2.1527487531733662, 14.377292362018647),
  vec3(11.307922894679344, 8.975630694774289, 14.437141911091254),
  vec3(14.445382693660449, 1.122842499621406, 14.488956361493427),
  vec3(3.5676622494525425, 14.098907673340833, 14.54329438289543),
  vec3(-3.8778536668627517, -14.098613341695934, 14.622197072268158),
  vec3(2.4619396705506027, -14.433524815615343, 14.641987076364147),
  vec3(7.757137238651293, 12.45605832044074, 14.67400991622575),
  vec3(12.927683578129766, 7.032771676212672, 14.716823031686738),
  vec3(-13.69562353817854, -5.543522330611857, 14.775004024686503),
  vec3(-0.3131644053003022, -14.880361100053678, 14.883656083527924),
  vec3(0.1630971351567112, 14.98077605915136, 14.98166385986346));

:global float gSkylightTextureSizeRecip
:global sampler2D gSkylightTexture

const float Pi = 3.1415926535;

float Noise(vec2 co) {
  return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

mat2 Rotate2D(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return mat2(c, -s, s, c);
}  
  
float CalculateSoftShadow(vec3 lightProjectionCoord, float sampleSpread, float spread) {
  vec3 s = lightProjectionCoord * 0.5 + vec3(0.5, 0.5, 0.500);

  // Calculate penumbra
  float distance = 0;
  float distCount = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 sampleCoord = poissonDisk[i].xy;
    vec2 p = s.xy + sampleCoord * gSkylightTextureSizeRecip * sampleSpread;
    float shadowZ = texture(gSkylightTexture, p).z;
    if (shadowZ < s.z) {
      distance += s.z - shadowZ;
      distCount += 1.0;
    }
  }
  if (distCount > 0) distance /= distCount;
  float sp = distance * spread;
  
  float shadow = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 sampleCoord = poissonDisk[i].xy;
    vec2 p = s.xy + sampleCoord * gSkylightTextureSizeRecip * sp;
    shadow += (texture(gSkylightTexture, p).z < s.z) ? 0 : 1.0;
  }
  shadow /= poissonCount;
  return shadow;
}  

float CalculateSoftShadow2(vec3 lightProjectionCoord, float sampleSpread, float spread, float bias, vec3 skyNormal) {
  vec3 s = lightProjectionCoord * 0.5 + vec3(0.5, 0.5, 0.5 - bias * 0.001);
  if (skyNormal.z < 0) return 0;
  //return skyNormal.z;

  // Calculate penumbra
  float distance = 0;
  float distCount = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 sampleCoord = poissonDisk[i].xy;
    vec2 p = s.xy + sampleCoord * gSkylightTextureSizeRecip * sampleSpread;
    float shadowZ = texture(gSkylightTexture, p).z;
    if (shadowZ < s.z) {
      distance += s.z - shadowZ;
      distCount += 1.0;
    }
  }
  if (distCount > 0) distance /= distCount;
  float sp = distance * spread;
  
  float shadow = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 sampleCoord = poissonDisk[i].xy;
    vec2 p = s.xy + sampleCoord * gSkylightTextureSizeRecip * sp;
    shadow += (texture(gSkylightTexture, p).z <= s.z) ? 0 : 1.0;
  }
  shadow /= poissonCount;
  return shadow;
}  

#ifdef FRAGMENT_SHADER

float CalculateSoftShadow3(vec3 lightProjectionCoord, mat4 skylightProjection, vec3 skyNormal, vec3 skyTangent, vec3 skyBinormal, float sampleSpread, float spread) {
  //if (skyNormal.z < 0) return 0;
  
  vec3 s = lightProjectionCoord * 0.5 + vec3(0.5, 0.5, 0.5 - 0.0000 /*- 0.001 / skyNormal.z*/);

  vec3 sdx = (vec4(skyTangent, 0) * skylightProjection).xyz; 
  vec3 sdy = (vec4(skyBinormal, 0) * skylightProjection).xyz; 
  
  float rand = Noise(gl_FragCoord.xy + vec2(10.2341234, 0.934367) * gl_FragCoord.z);
  float sinrand = sin(rand);
  float cosrand = cos(rand);
  mat2 rotrand = mat2(cosrand, sinrand, sinrand, -cosrand);
  
  // Calculate penumbra
  float distance = 0;
  float distCount = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 pd = rotrand * poissonDisk[i].xy * sampleSpread * 0.01;
	vec3 sMod = s + pd.x * sdx + pd.y * sdy;
	vec3 pmod = pd.x * sdx + pd.y * sdy;
    float shadowZ = texture(gSkylightTexture, sMod.xy).z;
    if (shadowZ < sMod.z) {
      distance += sMod.z - shadowZ;
      distCount += 1.0;
    }
  }
  if (distCount > 0) distance /= distCount;
  float sp = spread * distance * 0.1;
  
  float shadow = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 pd = rotrand * poissonDisk[i].xy * sp;
	vec3 sMod = s + pd.x * sdx + pd.y * sdy;
    shadow += (texture(gSkylightTexture, sMod.xy).z < sMod.z) ? 0 : 1.0;
  }
  shadow /= poissonCount;
  return shadow;
} 

#endif

vec3 ApplyNormalMap(vec3 normal, vec3 tangent, sampler2D normalMap, vec2 texCoord, float intensity) {
  vec3 normalNorm = normalize(normal);

  vec3 tangentNorm = normalize(tangent);
  vec3 binormal = cross(tangent, normal);
  mat3 normalSpace = mat3(tangent, binormal, normal);

  vec3 n = normalize(texture(normalMap, texCoord).xyz * 2.0 - 1.0);
  vec3 np = normalSpace * n;
  return normalize(mix(normal, np, intensity));
}

float CalculateLight(float specularFactor, float specularPower, vec3 normal, vec3 lightDir, vec3 viewSpacePos) {
  // Diffuse
  float cosa = dot(lightDir, normal);
  float diffuse = max(cosa, 0.0);

  // Specular
  vec3 pointToEye = normalize(viewSpacePos);
  vec3 lightMirror = reflect(lightDir, normal);
  float coss = dot(pointToEye, lightMirror);
  float specular = specularFactor * pow(max(coss, 0.0), specularPower);

  return diffuse + specular;
}

vec3 SolidMaterial(vec3 color, vec3 lightColor, vec3 emissiveColor, 
				   float ambientFactor, float light, float shadow) 
{
  vec3 lightAffectedColor = color * lightColor * 
	(ambientFactor + (1.0 - ambientFactor) * shadow * light);
  return lightAffectedColor + emissiveColor;
}

vec3 TexturedMaterial(vec3 color, vec3 lightColor, vec3 emissiveColor, 
				   float ambientFactor, float light, float shadow, 
				   sampler2D tex, vec2 texCoord) 
{
  vec3 lightAffectedColor = color * lightColor * 
	(ambientFactor + (1.0 - ambientFactor) * shadow * light);
  lightAffectedColor *= texture(tex, texCoord).rgb;
  return lightAffectedColor + emissiveColor;
}

mat4 Translate(vec3 translateVector) {
  mat4 matrix;
  matrix[0] = vec4(1.0, 0.0, 0.0, translateVector.x);
  matrix[1] = vec4(0.0, 1.0, 0.0, translateVector.y);
  matrix[2] = vec4(0.0, 0.0, 1.0, translateVector.z);
  matrix[3] = vec4(0.0, 0.0, 0.0, 1.0);
  return matrix;
}

mat4 RotateX(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  mat4 matrix;
  matrix[0] = vec4(1, 0, 0, 0);
  matrix[1] = vec4(0, c, -s, 0);
  matrix[2] = vec4(0, s, c, 0);
  matrix[3] = vec4(0, 0, 0, 1);
  return matrix;
}

mat4 RotateY(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  mat4 matrix;
  matrix[0] = vec4(c, 0, -s, 0);
  matrix[1] = vec4(0, 1, 0, 0);
  matrix[2] = vec4(s, 0, c, 0);
  matrix[3] = vec4(0, 0, 0, 1);
  return matrix;
}

mat4 RotateZ(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  mat4 matrix;
  matrix[0] = vec4(c, -s, 0, 0);
  matrix[1] = vec4(s, c, 0, 0);
  matrix[2] = vec4(0, 0, 1, 0);
  matrix[3] = vec4(0, 0, 0, 1);
  return matrix;
}

mat4 Rotate(vec3 angle) {
  return RotateX(angle.x) * RotateY(angle.y) * RotateZ(angle.z);
}
  
SHADER 
{}


