:name "Default FS"
:returns void

:output vec4 FragColor
:input vec2 vTexCoord
:input vec3 vNormal
:input vec3 vTangent
:input vec3 vSkyLightCoord
:input vec3 vModelSpacePos
:input vec3 vViewSpacePos

:param vec3 Ambient
:param vec3 Diffuse
:param vec3 Specular
:param vec3 Emissive
:param float SpecularPower
:param float SampleSpread
:param float Spread
:param float MaxSpread
:param vec3 LightDir
:param sampler2D NormalMap
:param float NormalMapIntensity
:param float NormalMapScale


:global sampler2D gSkylightTexture
:global sampler2D gSkylightColorTexture
:global mat4 gCamera
:global mat4 gView
:global float gSkylightTextureSizeRecip;


const int poissonCount = 198;
vec3 poissonDisk[198] = vec3[](
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
  vec3(0.1630971351567112, 14.98077605915136, 14.98166385986346),
  vec3(-14.80365472793265, 2.421795126818701, 15.000442824801056),
  vec3(-9.315581488771858, 11.763307478884066, 15.00518114908043),
  vec3(-12.874121440126723, 7.765212440216818, 15.034677485627306),
  vec3(-7.597575305651912, 12.990651587584154, 15.04925842674829),
  vec3(9.964299862829339, -11.31394309381215, 15.076225657849527),
  vec3(10.530813871078436, 10.903228159249867, 15.15844401909237),
  vec3(14.38551529489169, -4.796713598091692, 15.164152190006552),
  vec3(15.133405087925098, -1.4110958706267738, 15.199050664805917),
  vec3(12.037777955858342, -9.320081727074879, 15.22406061187041),
  vec3(-6.597464114696695, -13.823538576905987, 15.317204429396334),
  vec3(-8.61076887690023, -12.823735744185296, 15.446473354396765),
  vec3(15.164911412147262, 3.0331521906443655, 15.465269165128843),
  vec3(-15.489890883183163, -0.5050564916268421, 15.498122519603957),
  vec3(7.97007673827266, -13.30563389601182, 15.510061785455711),
  vec3(13.47181325521909, -7.857239968016826, 15.59570364871357),
  vec3(5.610081465230287, 14.677466548176973, 15.713084939609232),
  vec3(-1.7300091866701521, 15.771963130511622, 15.866560836431477),
  vec3(6.388827873453117, -14.608689575322138, 15.944620807806537),
  vec3(-14.198805434237524, -7.525283065737403, 16.069721869989138),
  vec3(4.491638578173482, -15.435896033330994, 16.076122152706475),
  vec3(-15.113115334188247, 5.838489125823255, 16.2016730733845),
  vec3(-10.56896159577976, -12.373757824584182, 16.27307075860379),
  vec3(-3.8743894937143857, 15.827538766836362, 16.294842047860186),
  vec3(13.615498429436421, 8.963467354084717, 16.301090285306934),
  vec3(-1.919396831892339, -16.215392256761177, 16.328595476614293),
  vec3(-16.349038823800303, -2.392449797654802, 16.523162121592467),
  vec3(-6.640645662734581, 15.264450027111259, 16.646369257233616),
  vec3(-12.827628340437665, 10.757002598477676, 16.74100217263157),
  vec3(15.835221254675794, 5.62895323551712, 16.805931890620485),
  vec3(3.4095075071290886, 16.517099453875957, 16.865328808250347),
  vec3(-16.28722931384734, -4.60987035398114, 16.927041779423902),
  vec3(0.19188092700890635, -16.938876274288113, 16.93996303791099),
  vec3(-13.4750424822691, -10.27404501659524, 16.94499250226988),
  vec3(-16.565538633051144, 3.806536880388972, 16.997258403191715),
  vec3(13.043537783379108, 10.9406153740182, 17.024421948149715),
  vec3(-14.787884107525443, 8.470736176839466, 17.04215032662312),
  vec3(17.02746205919391, 1.518312455383132, 17.095020821556773),
  vec3(12.647180597729033, -11.529509808732643, 17.113759771050777),
  vec3(1.0878079884122371, 17.09800546159282, 17.132574733072417),
  vec3(-11.682312938291506, 12.580841031226868, 17.16840110904841),
  vec3(9.285714677661844, 14.54153163280563, 17.253423987801128),
  vec3(16.382366643269812, -5.527221789298165, 17.289653482433124),
  vec3(-9.0836754792788, 14.737470864830328, 17.312024945238893),
  vec3(-5.691995495710087, -16.507483882343067, 17.461266759573892),
  vec3(11.495515626500776, 13.349976880258957, 17.617285892627507),
  vec3(-12.609454632227237, -12.365837219266155, 17.661038368555374),
  vec3(7.197668621585084, 16.178227871043212, 17.707102830038288),
  vec3(2.1413316125151525, -17.68763222434103, 17.816779584939134),
  vec3(-3.4710222128667425, -17.513695297877465, 17.85434171816787),
  vec3(-17.72966100361741, 2.157330111473282, 17.860429796425986),
  vec3(17.60845440569112, -3.162444851796515, 17.890185130343323),
  vec3(17.545182844924483, 3.706251554055296, 17.932365757026307),
  vec3(-17.975170498014272, -1.0295174585195426, 18.004628866768655),
  vec3(-7.748292932501325, -16.294070875486693, 18.04252723603968),
  vec3(4.156762114387156, -17.60291415469556, 18.087046691297772),
  vec3(16.090427102398678, 8.505379028424791, 18.200091119408555),
  vec3(15.11365387216427, 10.310822256869535, 18.295780633259195),
  vec3(-3.68970328911821, 17.92071694864856, 18.29661188076387),
  vec3(-10.388801630530057, -15.22033545041672, 18.427854203398613),
  vec3(18.429757646167182, 0.071121600417829, 18.429894877033448),
  vec3(-17.19532437402358, -6.760901352621152, 18.476714194568363),
  vec3(-0.8819044119912256, 18.47182445428266, 18.492864950075326),
  vec3(11.599421307939686, -14.406693764552433, 18.49592927929492),
  vec3(-11.213837710657781, 14.734527682906986, 18.516383616660217),
  vec3(16.776331136696214, -7.98388561275646, 18.579228075613436),
  vec3(7.911062775944984, -16.917123920316698, 18.675491853771717),
  vec3(10.050748021452499, -15.812019147897072, 18.735994377779765),
  vec3(17.960667365292274, 5.844413463181056, 18.88763460402829),
  vec3(15.676290881073449, -10.588862356336797, 18.917454950110837),
  vec3(2.9473858984622936, 18.765428380632486, 18.995483303751488),
  vec3(5.151889829793166, 18.333411729193358, 19.043527888772363),
  vec3(-8.842552585098478, 16.980614232765667, 19.1450253575762),
  vec3(-5.84260502727351, 18.231837091446508, 19.145127767496522),
  vec3(-16.776354041407284, 9.253524226028247, 19.159169228448665),
  vec3(-18.830088902372623, -3.5400607384871243, 19.15996550371202),
  vec3(9.607773469318452, 16.582137718444372, 19.164461963519745),
  vec3(-13.761091046827314, 13.52491323607822, 19.294841404958856),
  vec3(19.222475924290606, -1.7822331051774079, 19.304919981733214),
  vec3(15.035120879500148, 12.329639831510974, 19.44414766031063),
  vec3(-1.6637656410238861, -19.380494517881864, 19.451778424244416),
  vec3(-5.18877370234039, -18.760326057777004, 19.464665584802788),
  vec3(18.443853341673268, -6.4686365090675455, 19.54530594223569),
  vec3(-18.765699683078743, 5.588475807775868, 19.580156956715076),
  vec3(-17.133813309687827, -9.502734064136279, 19.59258822169577),
  vec3(-12.498466189853886, -15.37825974898528, 19.816723493205973),
  vec3(0.32822401115304345, -19.932742209637592, 19.935444389262194),
  vec3(-7.792330659470071, -18.3523521120526, 19.938135448212776),
  vec3(3.291754510336414, -19.67864349048788, 19.952058976006487),
  vec3(6.741023209158563, -18.96157295551112, 20.12418055110779));


SHADER
{
  vec3 color = Ambient;
  vec3 light = (vec4(normalize(LightDir), 0) * gCamera).xyz;
  vec3 normal = normalize(vNormal);
  vec3 tangent = normalize(vTangent);
  vec3 binormal = cross(normal, tangent);
  mat3 normalSpace = mat3(tangent, binormal, normal);

  vec3 n = normalize(texture(NormalMap, vTexCoord * NormalMapScale).xyz * 2.0 - 1.0);
  vec3 np = normalSpace * n;
  normal = normalize(mix(normal, np, NormalMapIntensity));


  float cosa = dot(light, normal);
  if (cosa < 0) cosa = 0; 
  vec3 diffuse = Diffuse * cosa;


  vec3 pointToEye = normalize(vViewSpacePos);
  vec3 lightMirror = reflect(light, normal);
  float coss = dot(pointToEye, lightMirror);
  vec3 specular = Specular * pow(max(coss, 0.0), SpecularPower);

  
  // -------------
  // Soft shadows

  vec3 s = vSkyLightCoord * 0.5 + vec3(0.5, 0.5, 0.500);

  // calculate penumbra
  float distance = 0;
  float distCount = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 p = s.xy + poissonDisk[i].xy * gSkylightTextureSizeRecip * SampleSpread;
    float shadowZ = texture(gSkylightTexture, p).z;
    if (shadowZ < s.z) {
      distance += s.z - shadowZ;
      distCount += 1.0;
    }
  }
  if (distCount > 0) distance /= distCount;
  float spread = min(distance * Spread, MaxSpread);
  
  float shadow = 0;
  for (int i=0; i<poissonCount; i++) {
    vec2 p = s.xy + poissonDisk[i].xy * gSkylightTextureSizeRecip * spread;
    shadow += (texture(gSkylightTexture, p).z < s.z) ? 0 : 1;
  }
  shadow /= poissonCount;


  // -------------
  // Final color

  color += shadow * (diffuse + specular) + Emissive;
  FragColor = vec4(color, 1);
}
