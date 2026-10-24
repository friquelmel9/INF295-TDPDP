/* 
 * Función auxiliar que leer un archivo especifico
 *
 * Parámetros
 * - filepath (string): Dirección de archivo
 * - inst (TDPDP): Instancia a asignar
 * */
void read_file(std::string filepath, TDPDP &inst){
	inst.is_instance = false;
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
	for(int i=0; i<inst.size_F; ++i){
		file >> inst.F[i].i >> inst.F[i].j >> inst.F[i].k;
	}

	file.close();
	inst.is_instance = true;
}
