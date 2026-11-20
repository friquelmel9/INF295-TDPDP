/* 
 * Función auxiliar que leer un archivo especifico
 *
 * Parámetros
 * - filepath (string): Dirección de archivo
 * - inst (TDPDP): Instancia a asignar
 * */
void read_file(const std::string &filepath, TDPDP &inst){
	inst.isInstance = false;
	std::ifstream file(filepath);
	if (!file.is_open()){ 
		throw std::runtime_error("Error: No se pudo abrir el archivo de instancia en " + filepath);
	}
	
	file >> inst.c;
	file >> inst.SL;
	file >> inst.SR;
	file >> inst.E;

	// Lectura de los Cdrones
	std::string line;
	std::getline(file >> std::ws, line);
	std::stringstream ss(line);
	Client cdron;
	while(ss >> cdron){inst.Cdrones.insert(cdron);}

	// Tau
	int num_nodes = inst.c + 2;

	inst.tauTruck.assign(num_nodes, std::vector<Time>(num_nodes, 0.0));
	for(int i=0; i<num_nodes; ++i){
		for(int j=0; j<num_nodes; ++j){
			file >> inst.tauTruck[i][j];
		}
	}
	inst.tauDrone.assign(num_nodes, std::vector<Time>(num_nodes, 0.0));
	for(int i=0; i<num_nodes; ++i){
		for(int j=0; j<num_nodes; ++j){
			file >> inst.tauDrone[i][j];
		}
	}

	// Conjunto F
	file >> inst.size_F;
	
	inst.F.resize(inst.size_F);
	inst.in_F.assign(static_cast<size_t>(num_nodes) * num_nodes * num_nodes, false);
	for(int i=0; i<inst.size_F; ++i){
		file >> inst.F[i].i >> inst.F[i].j >> inst.F[i].k;
		if(inst.F[i].i >= 0 && inst.F[i].i < num_nodes &&
		   inst.F[i].j >= 0 && inst.F[i].j < num_nodes &&
		   inst.F[i].k >= 0 && inst.F[i].k < num_nodes){
			inst.in_F[(static_cast<size_t>(inst.F[i].i) * num_nodes + inst.F[i].j) * num_nodes + inst.F[i].k] = true;
		}
	}

	file.close();
	inst.isInstance = true;
}

/*
 * Función auxiliar para registrar resultados en archivo CSV/texto.
 * Si el directorio no existe, lo crea automáticamente.
 * Si el archivo no existe o está vacío, escribe la cabecera CSV.
 * Escribe en modo append para registrar en la última línea.
 *
 * Parámetros:
 * - filepath (string): Ruta del archivo
 * - inst (string): Ruta o nombre del archivo de la instancia
 * - tdpdp (TDPDP): Objeto con datos de la instancia (c)
 * - seed (int): Semilla utilizada
 * - n (int): Tamaño de la población
 * - g (int): Cantidad de generaciones
 * - p_mut (Prob): Probabilidad de mutación
 * - p_cro (Prob): Probabilidad de cruzamiento
 * - runtime (double): Tiempo de ejecución en segundos
 * - eval (Time): Evaluación/fitness del mejor individuo
 */
void write_result(const std::string &filepath, const std::string &inst, const TDPDP &tdpdp, int seed, int n, int g, Prob p_mut, Prob p_cro, double runtime, Time eval, const std::string &init_type){
	if (filepath.empty()) return;
	std::filesystem::path p(filepath);
	if (p.has_parent_path()) {
		std::filesystem::create_directories(p.parent_path());
	}

	std::string instance_name = std::filesystem::path(inst).filename().string();
	if (instance_name.empty()) instance_name = inst;

	bool write_header = false;
	std::error_code ec;
	if (!std::filesystem::exists(filepath, ec) || std::filesystem::file_size(filepath, ec) == 0) {
		write_header = true;
	}

	std::ofstream file(filepath, std::ios::app);
	if (!file.is_open()){
		std::cerr << "Error: No se pudo abrir o crear el archivo en " << filepath << std::endl;
		return;
	}

	if (write_header) {
		file << "instance,c,seed,n,g,p_mut,p_cro,runtime,eval,init_type\n";
	}
	file << instance_name << "," << tdpdp.c << "," << seed << "," << n << "," << g << "," << p_mut << "," << p_cro << "," << runtime << "," << eval << "," << init_type << "\n";

	file.close();
}

/*
 * Función auxiliar para registrar el avance de cada iteración/generación del evolutivo.
 * Si el directorio no existe, lo crea automáticamente.
 * Si el archivo no existe o está vacío, escribe la cabecera CSV.
 * Escribe en modo append.
 *
 * Parámetros:
 * - filepath (string): Ruta del archivo (ej. results_per_iteration.csv)
 * - inst (string): Ruta o nombre de la instancia
 * - c (int): Cantidad de clientes
 * - seed (int): Semilla utilizada
 * - n (int): Tamaño de la población
 * - g (int): Cantidad total de generaciones
 * - p_mut (Prob): Probabilidad de mutación
 * - p_cro (Prob): Probabilidad de cruzamiento
 * - i (int): Iteración/generación actual
 * - runtime (double): Tiempo transcurrido acumulado en segundos
 * - eval (Time): Evaluación/fitness del mejor individuo histórico hasta esa generación
 * - init_type (string): Tipo de inicialización ("grasp" o "random-start")
 */
void write_step(const std::string &filepath, const std::string &inst, int c, int seed, int n, int g, Prob p_mut, Prob p_cro, int i, double runtime, Time eval, const std::string &init_type){
	if (filepath.empty()) return;
	std::filesystem::path p(filepath);
	if (p.has_parent_path()) {
		std::filesystem::create_directories(p.parent_path());
	}

	std::string instance_name = std::filesystem::path(inst).filename().string();
	if (instance_name.empty()) instance_name = inst;

	bool write_header = false;
	std::error_code ec;
	if (!std::filesystem::exists(filepath, ec) || std::filesystem::file_size(filepath, ec) == 0) {
		write_header = true;
	}

	std::ofstream file(filepath, std::ios::app);
	if (!file.is_open()){
		std::cerr << "Error: No se pudo abrir o crear el archivo en " << filepath << std::endl;
		return;
	}

	if (write_header) {
		file << "instance,c,seed,n,g,p_mut,p_cro,i,runtime,eval,init_type\n";
	}
	file << instance_name << "," << c << "," << seed << "," << n << "," << g << "," << p_mut << "," << p_cro << "," << i << "," << runtime << "," << eval << "," << init_type << "\n";

	file.close();
}
