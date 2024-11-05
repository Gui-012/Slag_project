#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <thread>

//Id of substances in addition vector 
#define     calcitic_lime_id    0
#define     dolomitic_lime_id   1
#define     fluorite_id         2
#define     Al_oxide_id         3
#define     TS_id               4
#define     CaC2_id             5
#define     Carbon_id           6
#define     FeSi_id             7
#define     FeSiMn_id           8
#define     SiC_id              9
#define     CaSi_id             10
#define     Metal_Al_id         11
#define     Sand_id             12
#define     return_slag_id      13
#define     Extra1t_id           14
#define     Extra2t_id           15
#define     Extra3t_id           16
#define     FeS_id              17
#define     Metal_Mn_id         18
#define     Extra1l_id           19
#define     Extra2l_id           20
#define     Extra3l_id           21

// Ids of elements in composition table

#define    C_id       0
#define    Si_id      1
#define    Mn_id      2
#define    P_id       3
#define    S_id       4
#define    Ca_id      5
#define    CaO_id     6
#define    SiO2_id    7
#define    MgO_id     8
#define    CaF2_id    9
#define    Al2O3_id   10
#define    FeO_id     11
#define    Fe2O3_id   12

//Molar mass ratio between Si and SiO2
#define Si_SiO2_ratio 60/28

//Molar mass ratio between Ca and CaO
#define Ca_CaO_ratio  56/40


//Chemical composition of steel (in decimal percentages)
struct steel_composition
{
    double C {};
    double Si {};
    double Mn {};
    double P {};
    double S {};
    double Nb {};
    double O2_ppm {};
    double Mn_S {};
};

//Chemical composition of slag (in decimal percentages)
struct slag_composition
{
    double CaO {};
    double MgO {};
    double SiO2 {};
    double Al2O3 {};
    double FeO {};
    double MnO {};
    double CaF2 {};
};

//Data from the tapping procedure
struct tapping_data
{
    float tapping_temperature {};
    std::vector<double> additions {};
    steel_composition steel {};
    slag_composition slag {};
    std::string observations {};
};

//Data from the laddle stage
struct laddle_data
{
    float initial_temperature {};
    float final_temperature {};
    std::vector<double> additions {};
    steel_composition initial_steel {};
    steel_composition final_steel {};
    slag_composition initial_slag {};
    slag_composition final_slag {};
    std::string observations {};
};

//Data from a heat
struct heat
{
    std::string date {};
    int heat_number {};
    double steel_weight {};
    std::string steel_type {};
    tapping_data tapping {};
    laddle_data laddle {};
};

//Paramers to be found
struct parameters
{
    double tapping_slag_mass {};
    double deoxidized_Mn {};
    double deoxidized_Fe {};
    double laddle_MgO {};
    double return_slag {};
};

//Promps user to enter a file name with a csv extension and returns the user input
std::string read_data_file_name()
{
    std::cout << "Enter data file name (csv extension):";
    std::string file_name;
    std::cin >> file_name;
    return file_name;
}

//Promps user to enter a file name with a csv extension and returns the user input
std::string read_comp_file_name()
{
    std::cout << "Enter compositions file name (csv extension):";
    std::string file_name;
    std::cin >> file_name;
    return file_name;
}

//Promps user to enter a file name and returns the user input
std::string read_return_file_name()
{
    std::cout << "Enter return file name:";
    std::string file_name;
    std::cin >> file_name;
    return file_name;
}

//check if data headings are the same as template and moves cursor to first data line
bool check_data_headings(std::ifstream &data_file)
{
    std::string line {};
    std::getline(data_file, line, ',');
    line = line.substr(3, 13);
    //First line of the csv file
    if(line.compare("VAZAMENTO FEA") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }
    //skips for empty lines
    for(int i {0}; i < 5; i++)
    {
        std::getline(data_file, line);
    }

    std::getline(data_file, line);
    line = line.substr(0, 126);
    if(line.compare(",,,,,,Adicões,,,,,,,,,,,,,,,,,Composição Química Aço,,,,,,,,Composição Química escória,,,,,,,,,Observações corridas") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: ,,,,,,Adicões,,,,,,,,,,,,,,,,,Composição Química Aço,,,,,,,,Composição Química escória,,,,,,,,,Observações corridas" << line << std::endl;
        return false;
    }

    std::getline(data_file, line);
    line = line.substr(0, 109);
    if(line.compare("Informações de corrida,,,,,,Fundentes/Formadores de escória,,,,,,Ligas/Desoxidantes,,,,,,Resíduos,,Outros") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }

    std::getline(data_file, line);
    line = line.substr(0, 353);
    if(line.compare("Contagem,Data,N° Corrida,Peso Aço,Tipo de aço,T°Vazamento,Cal Calcítica,Cal Dolomítica,Fluorita,Alumina,TS XXXX,CaC2,Carbono,FeSi,FeSiMn,SiC,Resíduo CaSi,Alumínio metálico,Areia,Escória de retorno,Aditivo de vazamento extra 1,Aditivo de vazamento extra 2,Aditivo de vazamento extra 3,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }

    return true;
}

//Transforms string to double using stod, but treats invalid arguments (such as empty cells) as 0
double string_to_double(std::string const &str)
{
    double number {0};
    try
    {
        number = std::stod(str);
    }
    catch(const std::invalid_argument& e)
    {
        number = 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return number;
}

//Transforms string to int using stoi, but treats invalid arguments (such as empty cells) as 0
int string_to_int(std::string const &str)
{
    int number {0};
    try
    {
        number = std::stoi(str);
    }
    catch(const std::invalid_argument& e)
    {
        number = 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return number;
}

//reads steel composition from file and returns in a steel_composition struct
steel_composition read_steel_composition(std::ifstream &data_file)
{
    std::string cell {};
    steel_composition steel {};
    std::getline(data_file, cell, ',');
    steel.C = string_to_double(cell);
    //Si
    std::getline(data_file, cell, ',');
    steel.Si = string_to_double(cell);
    //Mn
    std::getline(data_file, cell, ',');
    steel.Mn = string_to_double(cell);
    //P
    std::getline(data_file, cell, ',');
    steel.P = string_to_double(cell);
    //S
    std::getline(data_file, cell, ',');
    steel.S = string_to_double(cell);
    //Nb
    std::getline(data_file, cell, ',');
    steel.Nb = string_to_double(cell);
    //O2
    std::getline(data_file, cell, ',');
    steel.O2_ppm = string_to_double(cell);
    //Mn/S
    std::getline(data_file, cell, ',');
    steel.Mn_S = string_to_double(cell);
    return steel;
}

//reads slag composition from file and returns in a steel_composition struct
slag_composition read_slag_composition(std::ifstream &data_file)
{
    std::string cell {};
    slag_composition slag {};
    std::getline(data_file, cell, ',');
    slag.CaO = string_to_double(cell)/100.0;
    //MgO
    std::getline(data_file, cell, ',');
    slag.MgO = string_to_double(cell)/100.0;
    //SiO2
    std::getline(data_file, cell, ',');
    slag.SiO2 = string_to_double(cell)/100.0;
    //Al2O3
    std::getline(data_file, cell, ',');
    slag.Al2O3 = string_to_double(cell)/100.0;
    //FeO
    std::getline(data_file, cell, ',');
    slag.FeO = string_to_double(cell)/100.0;
    //MnO
    std::getline(data_file, cell, ',');
    slag.MnO = string_to_double(cell)/100.0;
    //CaF2
    std::getline(data_file, cell, ',');
    slag.CaF2 = string_to_double(cell)/100.0;
    return slag;
}

void read_tapping_data(std::vector<heat> &heats, std::ifstream &data_file)
{
    int heat_count {1};
    std::string cell {};
    std::getline(data_file, cell, ',');
    while(string_to_int(cell) == heat_count)
    {
        heats.push_back({});
        //date cell
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).date = cell;
        //heat number
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).heat_number = string_to_int(cell);
        //Steel weight
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).steel_weight = string_to_double(cell);
        //Steel type
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).steel_type = cell;
        //tapping temperature
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).tapping.tapping_temperature = string_to_double(cell);
        //tapping additions
        for(int i {0}; i < 17; i++)
        {
            std::getline(data_file, cell, ',');
            heats.at(heat_count - 1).tapping.additions.push_back(string_to_double(cell));
        }
        //laddle only additions
        for(int i {17}; i <= 21; i++)
        {
            heats.at(heat_count - 1).tapping.additions.push_back(0);
        }
        //tapping steel
        heats.at(heat_count - 1).tapping.steel = read_steel_composition(data_file);
        //Tapping slag
        heats.at(heat_count - 1).tapping.slag = read_slag_composition(data_file);
        //Ignore rest of line
        std::getline(data_file, cell);
        //Get first cell in next line
        std::getline(data_file, cell, ',');
        heat_count++;
    }
}

//check if headings on laddle data section are the same as template and moves cursor to first data line
bool check_laddle_headings(std::ifstream &data_file)
{
    std::string line {};
    //skips for empty lines
    for(int i {0}; i < 4; i++)
    {
        std::getline(data_file, line);
    }
    //First line of the csv file
    std::getline(data_file, line, ',');
    if(line.compare("FORNO PANELA") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }

    //skips for empty lines
    for(int i {0}; i < 5; i++)
    {
        std::getline(data_file, line);
    }

    std::getline(data_file, line);
    line = line.substr(0, 264);
    if(line.compare("Informações de corrida,,,,,,,Adicões,,,,,,,Ligas e desoxidades,,,,,,,Outros,,,Composição Química Aço (CFP),,,,,,,,Composição Química Aço (FFP),,,,,,,,Composição Química escória (CFP),,,,,,,,,Composição Química escória (FFP),,,,,,,,,Observações") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }

    std::getline(data_file, line);
    line = line.substr(0, 63);
    if(line.compare(",,,,,,,Fundentes/Formadores de escória,,,,,,Ligas/Desoxidantes") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }
    
    std::getline(data_file, line);
    line = line.substr(0, 400);
    if(line.compare("Contagem,Data,N° Corrida,Peso Aço,Tipo de aço,T°iFP,T°fFP,Cal Calcítica,Cal Dolomítica,Fluorita,Alumina,TS ***,CaC2,Carbono,FeSi,FeSiMn,SiC,CaSi,Alumínio metálico,FeS,Mn Eletrolítico,Aditivo de FP extra 1,Aditivo de FP extra 2,Aditivo de FP extra 3,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2,Basicidade,Fechamento,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }

    return true;
}

void read_laddle_data(std::vector<heat> &heats, std::ifstream &data_file)
{
    int heat_count {1};
    std::string cell {};
    std::getline(data_file, cell, ',');
    while(string_to_int(cell) == heat_count)
    {
        //date cell
        std::getline(data_file, cell, ',');
        //heat number
        std::getline(data_file, cell, ',');
        //Steel weight
        std::getline(data_file, cell, ',');
        //Steel type
        std::getline(data_file, cell, ',');
        //laddle initial temperature
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).laddle.initial_temperature = string_to_double(cell);
        //laddle final temperature
        std::getline(data_file, cell, ',');
        heats.at(heat_count - 1).laddle.final_temperature = string_to_double(cell);
        //tapping additions
        for(int i {0}; i <= 11; i++)
        {
            std::getline(data_file, cell, ',');
            heats.at(heat_count - 1).laddle.additions.push_back(string_to_double(cell));
        }
        //Skips tapping only additions
        for(int i {12}; i <= 16; i++)
        {
            heats.at(heat_count - 1).laddle.additions.push_back(0);
        }
        for(int i {17}; i <= 21; i++)
        {
            std::getline(data_file, cell, ',');
            heats.at(heat_count - 1).laddle.additions.push_back(string_to_double(cell));
        }
        //laddle initial steel
        heats.at(heat_count - 1).laddle.initial_steel = read_steel_composition(data_file);
        //laddle final steel
        heats.at(heat_count - 1).laddle.final_steel = read_steel_composition(data_file);
        //Laddle initial slag
        heats.at(heat_count - 1).laddle.initial_slag = read_slag_composition(data_file);
        //Ignore two lines
        std::getline(data_file, cell, ',');
        std::getline(data_file, cell, ',');
        //Laddle final slag
        heats.at(heat_count - 1).laddle.final_slag = read_slag_composition(data_file);
        //Ignore rest of line
        std::getline(data_file, cell);
        //Get first cell in next line
        std::getline(data_file, cell, ',');
        heat_count++;
    }
}

bool check_compositions_headings(std::ifstream &comp_file)
{
    std::string line {};
    //First line of the csv file
    std::getline(comp_file, line, ',');
    line = line.substr(3, 130);
    if(line.compare("Elementos de adição") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }
    //skip rest of line
    std::getline(comp_file, line);

    std::getline(comp_file, line);
    line = line.substr(0, 130);
    if(line.compare("Nome,Tipo,C,Si,Mn,P,S,Ca,CaO,SiO2,MgO,CaF2,Al2O3,FeO,Fe2O3,CaC2,Rendimento Fe,Rendimento Si,Rendimento Mn,Rendimento Ca,Fechamento") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        return false;
    }
    return true;
}

std::vector<std::vector<double>> read_composition_data(std::ifstream &comp_file)
{
    std::vector<std::vector<double>> compositions {};
    std::string cell {};
    for(int material_id {0}; material_id <= 21; material_id++)
    {
        std::vector<double> comp {};
        std::getline(comp_file, cell, ',');
        std::getline(comp_file, cell, ',');
        for(int element_id {0}; element_id <= 13; element_id++)
        {
            std::getline(comp_file, cell, ',');
            comp.push_back(string_to_double(cell)/100.0);
        }
        compositions.push_back(comp);
        std::getline(comp_file, cell);
    }
    return compositions;
}

//Calculates oxidized Si by subtracting all added Si to the Si leftover at the end (in Kg)
double get_oxidized_Si(heat heat, std::vector<std::vector<double>> compositions, double return_slag)
{
    double SiO2 {0};
    for(int i {0}; i < heat.tapping.additions.size(); i++)
    {
        SiO2 += (heat.tapping.additions.at(i) + heat.laddle.additions.at(i))*compositions.at(i).at(Si_id); //Calculates sum of Si added within all subtances
    }
    SiO2 += return_slag*compositions.at(return_slag_id).at(Si_id);
    SiO2 -= heat.laddle.final_steel.Si*heat.steel_weight*10; //Subtracts Si leftover at the end
    if(SiO2 < 0)
    {
        return 0;
    }
    return SiO2;
}

//Calculates oxidized by summing all added Ca (in Kg)
double get_oxidized_Ca(heat heat, std::vector<std::vector<double>> compositions, double return_slag)
{
    double CaO {0};
    for(int i {0}; i < heat.tapping.additions.size(); i++)
    {
        if(i == CaC2_id) // If the current substance is CaC2
        {
            CaO += (heat.tapping.additions.at(i) + heat.laddle.additions.at(i))*compositions.at(i).at(Ca_id)*40/60; //Adjust for molar mass
        }
        else
        {
            CaO += (heat.tapping.additions.at(i) + heat.laddle.additions.at(i))*compositions.at(i).at(Ca_id); //Calculates sum of Ca added within all subtance
        }
    }
    return CaO;
}

//Calculates added mass from all additions and return_slag parameter
double get_added_substance(int substance_id, heat heat, double return_slag, std::vector<std::vector<double>> compositions, double additional_mass)
{
    double added {0};
    for(int i {0}; i < heat.tapping.additions.size(); i++)
    {
        added += (heat.tapping.additions.at(i) + heat.laddle.additions.at(i))*compositions.at(i).at(substance_id); //Calculates sum of subtances added
    }
    added += return_slag*compositions.at(return_slag_id).at(substance_id);
    added += additional_mass; //incorporates additional mass (Needed for oxidized Si and Ca)
    return added;
}

double get_final_slag_mass(parameters parameters, slag_composition tapping_slag, double added_mass)
{
    double slag_mass {parameters.tapping_slag_mass};  //Start slag mass calculation with slag mass from tapping
    slag_mass += added_mass; //include added mass
    slag_mass += parameters.laddle_MgO; //include masss incorporated from laddle lining
    slag_mass -= parameters.tapping_slag_mass*tapping_slag.FeO*parameters.deoxidized_Fe; //Removes deoxidized FeO mass
    slag_mass -= parameters.tapping_slag_mass*tapping_slag.MnO*parameters.deoxidized_Mn; //Removes deoxidized MnO mass
    return slag_mass;
}

//Calculates the simulated slag composition
slag_composition get_calculated_slag(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters const &parameters)
{
    double oxidized_Si {get_oxidized_Si(heat, compositions, parameters.return_slag)};
    double oxidized_Ca {get_oxidized_Ca(heat, compositions, parameters.return_slag)};
    double added_SiO2 {get_added_substance(SiO2_id, heat, parameters.return_slag, compositions, oxidized_Si*Si_SiO2_ratio)};
    double added_CaO {get_added_substance(CaO_id, heat, parameters.return_slag, compositions, oxidized_Ca*Ca_CaO_ratio)};
    double added_MgO {get_added_substance(MgO_id, heat, parameters.return_slag, compositions, 0)};
    double added_CaF2 {get_added_substance(CaF2_id, heat, parameters.return_slag, compositions, 0)};
    double added_Al2O3 {get_added_substance(Al2O3_id, heat, parameters.return_slag, compositions, 0)};
    double total_added_mass {added_SiO2 + added_CaO + added_MgO + added_CaF2 + added_Al2O3};
    double final_slag_mass {get_final_slag_mass(parameters, heat.tapping.slag, total_added_mass)};
    slag_composition final_slag {};
    //Calculating percentage composition of final slag, including the parameters as needed
    final_slag.CaO = (parameters.tapping_slag_mass*heat.tapping.slag.CaO + added_CaO) / final_slag_mass; //Adds CaO from tapping and added Cao
    final_slag.MgO = (parameters.tapping_slag_mass*heat.tapping.slag.MgO + added_MgO + parameters.laddle_MgO) / final_slag_mass; //Adds MgO from tapping, added MgO and MgO from laddle lining
    final_slag.SiO2 = (parameters.tapping_slag_mass*heat.tapping.slag.SiO2 + added_SiO2) / final_slag_mass; //Adds SiO2 from tapping and added SiO2
    final_slag.Al2O3 = (parameters.tapping_slag_mass*heat.tapping.slag.Al2O3 + added_Al2O3) / final_slag_mass; //Adds Al2O3 from tapping and added Al2O3
    final_slag.FeO = (parameters.tapping_slag_mass*heat.tapping.slag.FeO*(1 - parameters.deoxidized_Fe)) / final_slag_mass; //FeO from tapping slag without desoxidized FeO
    final_slag.MnO = (parameters.tapping_slag_mass*heat.tapping.slag.MnO*(1 - parameters.deoxidized_Mn)) / final_slag_mass; //MnO from tapping slag without desoxidized MnO
    final_slag.CaF2 = (parameters.tapping_slag_mass*heat.tapping.slag.CaF2 + added_CaF2) / final_slag_mass; //Adds CaF2 from tapping and added CaF2

    return final_slag;
};

//Calculates mean square error of the slag composition
double find_slag_MSE(slag_composition const &calculated_slag, slag_composition const &measured_slag)
{
    std::vector<double> errors {};
    errors.push_back(calculated_slag.CaO - measured_slag.CaO);
    errors.push_back(calculated_slag.MgO - measured_slag.MgO);
    errors.push_back(calculated_slag.SiO2 - measured_slag.SiO2);
    errors.push_back(calculated_slag.Al2O3 - measured_slag.Al2O3);
    errors.push_back(calculated_slag.FeO - measured_slag.FeO);
    errors.push_back(calculated_slag.MnO - measured_slag.MnO);
    errors.push_back(calculated_slag.CaF2 - measured_slag.CaF2);

    double average_error {0};

    for(double x : errors)
    {
        average_error += x*x;
    }
    
    average_error /= errors.size();
    
    return average_error;
}

double get_slag_MSE(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters const &parameters)
{
    return  find_slag_MSE(get_calculated_slag(heat, compositions, parameters), heat.laddle.final_slag);
}

//Calculates the average relativeerror of the slag composition
double find_slag_relative_error(slag_composition const &calculated_slag, slag_composition const &measured_slag)
{
    double error {};
    error += abs(calculated_slag.CaO - measured_slag.CaO)/measured_slag.CaO;
    error += abs(calculated_slag.MgO - measured_slag.MgO)/measured_slag.MgO;
    error += abs(calculated_slag.SiO2 - measured_slag.SiO2)/measured_slag.SiO2;
    error += abs(calculated_slag.Al2O3 - measured_slag.Al2O3)/measured_slag.Al2O3;
    error += abs(calculated_slag.FeO - measured_slag.FeO)/measured_slag.FeO;
    error += abs(calculated_slag.MnO - measured_slag.MnO)/measured_slag.MnO;
    error += (measured_slag.CaF2 > 0 ? abs(calculated_slag.CaF2 - measured_slag.CaF2)/measured_slag.CaF2 : 0);
    
    return error/7;
}

double get_slag_relative_error(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters const &parameters)
{
    return  find_slag_relative_error(get_calculated_slag(heat, compositions, parameters), heat.laddle.final_slag);
}

//Calculates the average absolute error of the slag composition
double find_slag_absolute_error(slag_composition const &calculated_slag, slag_composition const &measured_slag)
{
    double error {};
    error += abs(calculated_slag.CaO - measured_slag.CaO);
    error += abs(calculated_slag.MgO - measured_slag.MgO);
    error += abs(calculated_slag.SiO2 - measured_slag.SiO2);
    error += abs(calculated_slag.Al2O3 - measured_slag.Al2O3);
    error += abs(calculated_slag.FeO - measured_slag.FeO);
    error += abs(calculated_slag.MnO - measured_slag.MnO);
    error += abs(calculated_slag.CaF2 - measured_slag.CaF2);
    
    return error/7;
}

double get_slag_absolute_error(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters const &parameters)
{
    return  find_slag_absolute_error(get_calculated_slag(heat, compositions, parameters), heat.laddle.final_slag);
}

//Calculates standard deviation of errors in the slag composition
double find_std_dev_slag_error(slag_composition const &calculated_slag, slag_composition const &measured_slag)
{
    std::vector<double> errors {};
    errors.push_back(calculated_slag.CaO - measured_slag.CaO);
    errors.push_back(calculated_slag.MgO - measured_slag.MgO);
    errors.push_back(calculated_slag.SiO2 - measured_slag.SiO2);
    errors.push_back(calculated_slag.Al2O3 - measured_slag.Al2O3);
    errors.push_back(calculated_slag.FeO - measured_slag.FeO);
    errors.push_back(calculated_slag.MnO - measured_slag.MnO);
    errors.push_back(calculated_slag.CaF2 - measured_slag.CaF2);

    double average_error {0};

    for(double x : errors)
    {
        average_error += x*x;
    }
    
    average_error /= errors.size();
    
    double std_dev {0};

    for(double x : errors)
    {
        std_dev += (x - average_error)*(x - average_error);
    }
    std_dev /= errors.size() - 1;
    std_dev = sqrt(std_dev);
    return std_dev;
}

parameters get_approximate_gradient(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters point, double const &delta)
{
    parameters gradient {0};
    parameters point2 {point};
    double point_MSE {get_slag_MSE(heat, compositions, point)};
    
    point2.tapping_slag_mass += delta * point.tapping_slag_mass;
    gradient.tapping_slag_mass = (get_slag_MSE(heat, compositions, point2) - point_MSE) / (delta * point.tapping_slag_mass);
    point.tapping_slag_mass -= gradient.tapping_slag_mass*point.tapping_slag_mass;
    point2.tapping_slag_mass = point.tapping_slag_mass;

    
    /*point2.return_slag += delta * point.return_slag;
    gradient.return_slag = (get_slag_MSE(heat, compositions, point2) - point_MSE) / (delta * point.return_slag);
    point.return_slag -= gradient.return_slag*point.return_slag*delta;
    point_MSE = get_slag_MSE(heat, compositions, point);
    point2.return_slag = point.return_slag;*/

    point2.laddle_MgO += delta * point.laddle_MgO;
    gradient.laddle_MgO = (abs(get_calculated_slag(heat, compositions, point2).MgO - heat.laddle.final_slag.MgO) - abs(get_calculated_slag(heat, compositions, point).MgO - heat.laddle.final_slag.MgO)) / (delta * point.laddle_MgO);
    point.laddle_MgO -= gradient.laddle_MgO*point.laddle_MgO;
    point2.laddle_MgO = point.laddle_MgO;

    point2.deoxidized_Mn += delta * point.deoxidized_Mn;
    gradient.deoxidized_Mn = (abs(get_calculated_slag(heat, compositions, point2).MnO - heat.laddle.final_slag.MnO) - abs(get_calculated_slag(heat, compositions, point).MnO - heat.laddle.final_slag.MnO)) / (delta * point.deoxidized_Mn);
    point.deoxidized_Mn -= gradient.deoxidized_Mn*point.deoxidized_Mn*delta;
    point.deoxidized_Mn = point.deoxidized_Mn <= 1 ? point.deoxidized_Mn : 1;
    point2.deoxidized_Mn = point.deoxidized_Mn;

    point2.deoxidized_Fe += delta * point.deoxidized_Fe;
    gradient.deoxidized_Fe = (abs(get_calculated_slag(heat, compositions, point2).FeO - heat.laddle.final_slag.FeO) - abs(get_calculated_slag(heat, compositions, point).FeO - heat.laddle.final_slag.FeO)) / (delta * point.deoxidized_Fe);
    point.deoxidized_Fe -= gradient.deoxidized_Fe*point.deoxidized_Fe*delta;
    point.deoxidized_Fe = point.deoxidized_Fe <= 1 ? point.deoxidized_Fe : 1;
    point2.deoxidized_Fe = point.deoxidized_Fe;

    return point2;
}

parameters minimize(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters start_point, double step_size, double finish_size, int max_iterations, bool b)
{
    parameters point {start_point};
    int n {0};
    double original_step_size {step_size};
    while(finish_size < step_size && n < max_iterations)
    {
        n += 1;
        step_size = original_step_size;
        double start {get_slag_MSE(heat, compositions, point)};
        parameters point2 {get_approximate_gradient(heat, compositions, point, step_size)};
        double end {get_slag_MSE(heat, compositions, point2)};
        int i {0};
        bool done {false};
        while(start < end)
        {
            if(i > 150)
            {
                if(b)
                {
                    std::cout << "n: " << n << std::endl << "i: " << i << std::endl;
                }
                return point;
            }
            step_size /= 2;
            point2 = get_approximate_gradient(heat, compositions, point2, step_size);
            end = get_slag_MSE(heat, compositions, point2);
            i++;
        }
        point = point2;
    }
    if(b)
    {
        std::cout << "n: " << n << std::endl;
    }
    return point;
}

std::vector<std::pair<parameters, heat>> heat_pairs{};

void minimize_heat(int id, std::vector<std::vector<double>> compositions)
{
    heat_pairs.at(id).first = {heat_pairs.at(id).second.steel_weight*5, 0.50, 0.50, 10, heat_pairs.at(id).second.steel_weight*2.5};
    double best_error {get_slag_MSE(heat_pairs.at(id).second, compositions, minimize(heat_pairs.at(id).second, compositions, heat_pairs.at(id).first, 0.01, 0.000000001, 300, false))};
    for(float i {1}; i <= 25; i += 0.5)
    {   
        parameters temp {minimize(heat_pairs.at(id).second, compositions, {heat_pairs.at(id).second.steel_weight*(5 + i), 0.50, 0.50, 10, 200}, 0.01, 0.0000000000000001, 300, false)};
        double error {get_slag_MSE(heat_pairs.at(id).second, compositions, temp)};
        if(error < best_error)
        {
            best_error = error;
            heat_pairs.at(id).first = temp;
        }
    }
    for(float i {20.0}; i <= heat_pairs.at(id).second.steel_weight*5; i += 20.0)
    {   
        parameters temp {heat_pairs.at(id).first};
        temp.return_slag = i;
        temp = minimize(heat_pairs.at(id).second, compositions, temp, 0.01, 0.0000000000000001, 300, false);
        double error {get_slag_MSE(heat_pairs.at(id).second, compositions, temp)};
        if(error < best_error)
        {
            best_error = error;
            heat_pairs.at(id).first = temp;
        }
    }

    heat_pairs.at(id).first = minimize(heat_pairs.at(id).second, compositions, heat_pairs.at(id).first, 0.01, 0.0000000001, 10000, true);
    std::cout << heat_pairs.at(id).first.tapping_slag_mass << std::endl;
    return;
}

int main()
{
    std::string file_name  {read_data_file_name()};
    //std::string file_name  {"dados.csv"};
    std::ifstream data_file (file_name);
    if(!data_file.is_open())
    {
        std::cout << "Could not open file:\"" << file_name << "\"" << std::endl;
        return 1;
    }

    //file_name = "comp.csv";
    file_name = read_comp_file_name();

    std::ifstream compositions_file (file_name);
    if(!compositions_file.is_open())
    {
        std::cout << "Could not open file:\"" << file_name << "\"" << std::endl;
        return 1;
    }

    if(!check_data_headings(data_file))
    {
        std::cout << "Invalid Data Table Format" << std::endl;
        return 1;
    }
    std::vector<heat> heats {};
    read_tapping_data(heats, data_file);
    if(!check_laddle_headings(data_file))
    {
        std::cout << "Invalid Data Table Format" << std::endl;
        return 1;
    }
    read_laddle_data(heats, data_file);
    if(!check_compositions_headings(compositions_file))
    {
        std::cout << "Invalid Composition Table Format" << std::endl;
        return 1;
    }
    std::vector<std::vector<double>> compositions {read_composition_data(compositions_file)};
    std::ofstream results_file;
    data_file.close();
    compositions_file.close();
    file_name = read_return_file_name();
    results_file.open(file_name);
    if(!results_file.is_open())
    {
        std::cout << "Could not open " << file_name << " file" << std::endl;
        return 1;
    }
    std::cout << "Running..." << std::endl;

    
    std::vector<std::thread> threads {};
    parameters result {};
    for(auto test_heat : heats)
    {
        heat_pairs.push_back(std::make_pair(result, test_heat));
        //threads.push_back(std::thread(foo, heat_pairs.size() - 1, compositions));
        //threads.push_back(std::thread(minimize_heat, heat_pairs.size() - 1, compositions));
    }

    for(int i {0}; i < heat_pairs.size(); i++)
    {
        threads.push_back(std::thread(minimize_heat, i, compositions));
    }

    for(auto &th : threads)
    {
        th.join();
    }

    std::cout << "a: " << heat_pairs.at(0).first.tapping_slag_mass << std::endl;

    for(auto test_heat_pair : heat_pairs)
    {
        heat test_heat {test_heat_pair.second};
        parameters results {test_heat_pair.first};
        
        //5 a 15% da escoria final fica de retorno
        std::cout << "Heat Number: " << test_heat.heat_number << std::endl;
        results_file << "Heat Number: " << test_heat.heat_number << std::endl;
        results_file << "Result Tapping Mass: " << results.tapping_slag_mass << std::endl;
        results_file << "Result Deoxidized Mn: " << results.deoxidized_Mn << std::endl;
        results_file << "Result Deoxidized Fe: " << results.deoxidized_Fe << std::endl;
        results_file << "Result Laddle MgO: " << results.laddle_MgO << std::endl;
        results_file << "Result Return Slag: " << results.return_slag << std::endl << std::endl;;

        slag_composition final_calculated_slag {get_calculated_slag(test_heat, compositions, results)};

        results_file << "Calculated/Measured CaO: " << final_calculated_slag.CaO*100 << " / " << test_heat.laddle.final_slag.CaO*100 << std::endl;
        results_file << "Calculated/Measured MgO: " << final_calculated_slag.MgO*100 << " / " << test_heat.laddle.final_slag.MgO*100 << std::endl;
        results_file << "Calculated/Measured SiO2: " << final_calculated_slag.SiO2*100 << " / " << test_heat.laddle.final_slag.SiO2*100 << std::endl;
        results_file << "Calculated/Measured Al2O3: " << final_calculated_slag.Al2O3*100 << " / " << test_heat.laddle.final_slag.Al2O3*100 << std::endl;
        results_file << "Calculated/Measured FeO: " << final_calculated_slag.FeO*100 << " / " << test_heat.laddle.final_slag.FeO*100 << std::endl;
        results_file << "Calculated/Measured MnO: " << final_calculated_slag.MnO*100 << " / " << test_heat.laddle.final_slag.MnO*100 << std::endl;
        results_file << "Calculated/Measured CaF2: " << final_calculated_slag.CaF2*100 << " / " << test_heat.laddle.final_slag.CaF2*100 << std::endl << std::endl;

        results_file << "Final MSE: " << get_slag_MSE(test_heat, compositions, results)*100 << std::endl;
        results_file << "Final Absolute Error: " << get_slag_absolute_error(test_heat, compositions, results)*100 << std::endl;
        results_file << "Final Relative Error: " << get_slag_relative_error(test_heat, compositions, results)*100 << std::endl;
        results_file << "Final Standard Deviation: " << find_std_dev_slag_error(get_calculated_slag(test_heat, compositions, results), test_heat.laddle.final_slag)*100 << std::endl << std::endl;
        results_file << "---------------------------------------------------------------------------------" << std::endl << std::endl;
    }
    std::cout << "Done" << std::endl;
    results_file.close();
}
//Test branch
//Tested minimazation algorithm 
/*
parameters get_approximate_gradient(heat heat, std::vector<std::vector<double>> compositions, parameters point, double delta)
{
    parameters gradient {0};
    parameters point2 {point};
    double point_MSE {get_slag_MSE(heat, compositions, point)};
    point2.tapping_slag_mass += delta * point.tapping_slag_mass;
    gradient.tapping_slag_mass = (get_slag_MSE(heat, compositions, point2) - point_MSE) / (delta * point.tapping_slag_mass);
    point.tapping_slag_mass -= gradient.tapping_slag_mass*point.tapping_slag_mass;
    point2.tapping_slag_mass = point.tapping_slag_mass;
    point2.return_slag += delta * point.return_slag;
    gradient.return_slag = (get_slag_MSE(heat, compositions, point2) - point_MSE) / delta;
    point.return_slag -= gradient.return_slag*point.return_slag*delta;
    get_slag_MSE(heat, compositions, point);
    point2.return_slag = point.return_slag;
    point2.laddle_MgO += delta * point.laddle_MgO;
    gradient.laddle_MgO = (abs(get_calculated_slag(heat, compositions, point2).MgO - heat.laddle.final_slag.MgO) - abs(get_calculated_slag(heat, compositions, point).MgO - heat.laddle.final_slag.MgO)) / (delta * point.laddle_MgO);
    point.laddle_MgO -= gradient.laddle_MgO*point.laddle_MgO;
    point2.laddle_MgO = point.laddle_MgO;
    point2.deoxidized_Mn += delta * point.deoxidized_Mn;
    gradient.deoxidized_Mn = (abs(get_calculated_slag(heat, compositions, point2).MnO - heat.laddle.final_slag.MnO) - abs(get_calculated_slag(heat, compositions, point).MnO - heat.laddle.final_slag.MnO)) / (delta * point.deoxidized_Mn);
    point.deoxidized_Mn -= gradient.deoxidized_Mn*point.deoxidized_Mn;
    point2.deoxidized_Mn = point.deoxidized_Mn;
    point2.deoxidized_Fe += delta * point.deoxidized_Fe;
    gradient.deoxidized_Fe = (abs(get_calculated_slag(heat, compositions, point2).FeO - heat.laddle.final_slag.FeO) - abs(get_calculated_slag(heat, compositions, point).FeO - heat.laddle.final_slag.FeO)) / (delta * point.deoxidized_Fe);
    point.deoxidized_Fe -= gradient.deoxidized_Fe*point.deoxidized_Fe;
    point2.deoxidized_Fe = point.deoxidized_Fe;

    return point2;
}

parameters minimize(heat heat, std::vector<std::vector<double>> compositions, parameters start_point, double step_size, double finish_size, int max_iterations)
{
    parameters point {start_point};
    int n {0};
    double original_step_size {step_size};
    while(finish_size < step_size && n < max_iterations)
    {
        n += 1;
        step_size = original_step_size;
        double start {get_slag_MSE(heat, compositions, point)};
        parameters point2 {get_approximate_gradient(heat, compositions, point, step_size)};
        double end {get_slag_MSE(heat, compositions, point2)};
        int i {0};
        bool done {false};
        while(start < end)
        {
            if(i > 100)
            {
                std::cout << "i:" << i << std::endl;
                std::cout << "n:" << n << std::endl;
                return point;
            }
            step_size /= 2;
            point2 = get_approximate_gradient(heat, compositions, point2, step_size);
            end = get_slag_MSE(heat, compositions, point2);
            i++;
        }
        point = point2;
    }
    std::cout << "n:" << n << std::endl;
    return point;
}
*/

//Line search algorithm
/*
parameters get_approximate_gradient(heat heat, std::vector<std::vector<double>> compositions, parameters point, double delta)
{
    parameters gradient {0};
    parameters point2 {point};
    double point_MSE {get_slag_MSE(heat, compositions, point)};
    
    point2.tapping_slag_mass += delta;
    double point2_MSE {get_slag_MSE(heat, compositions, point2)};
    gradient.tapping_slag_mass = -(point2_MSE - point_MSE) / delta;
    point2.tapping_slag_mass = point.tapping_slag_mass;

    point2.laddle_MgO += delta;
    point2_MSE = get_slag_MSE(heat, compositions, point2);
    gradient.laddle_MgO = -pow(abs(get_calculated_slag(heat, compositions, point2).MgO - heat.laddle.final_slag.MgO) - abs(get_calculated_slag(heat, compositions, point).MgO - heat.laddle.final_slag.MgO), 2) / delta;
    point2.laddle_MgO = point.laddle_MgO;

    point2.deoxidized_Fe += delta;
    point2_MSE = get_slag_MSE(heat, compositions, point2);
    gradient.deoxidized_Fe = -pow(abs(get_calculated_slag(heat, compositions, point2).FeO - heat.laddle.final_slag.FeO) - abs(get_calculated_slag(heat, compositions, point).FeO - heat.laddle.final_slag.FeO), 2) / delta;
    point2.deoxidized_Fe = point.deoxidized_Fe;

    point2.deoxidized_Mn += delta;
    point2_MSE = get_slag_MSE(heat, compositions, point2);
    gradient.deoxidized_Mn = -pow(abs(get_calculated_slag(heat, compositions, point2).MnO - heat.laddle.final_slag.MnO) - abs(get_calculated_slag(heat, compositions, point).MnO - heat.laddle.final_slag.MnO), 2) / delta;
    point2.deoxidized_Mn = point.deoxidized_Mn;

    return gradient;
}

double get_gradient_magnitude(parameters gradient)
{
    return std::sqrt(gradient.tapping_slag_mass*gradient.tapping_slag_mass + gradient.laddle_MgO*gradient.laddle_MgO + gradient.deoxidized_Fe*gradient.deoxidized_Fe + gradient.deoxidized_Mn*gradient.deoxidized_Mn);
}

parameters minimize(heat heat, std::vector<std::vector<double>> compositions, parameters start_point, double delta, double finish_size, int max_iterations)
{
    parameters point {start_point};
    parameters point2 {};
    int n {0};
    double original_delta {delta};
    while(finish_size < delta && n < max_iterations)
    {
        n += 1;
        //std::cout << "n:" << n << std::endl;
        parameters gradient (get_approximate_gradient(heat, compositions, point, delta));\
        double gradient_magnitude {get_gradient_magnitude(gradient)};

        parameters vector_p {};
        vector_p.tapping_slag_mass = gradient.tapping_slag_mass/gradient_magnitude;
        vector_p.laddle_MgO = gradient.laddle_MgO/gradient_magnitude;
        vector_p.deoxidized_Fe = gradient.deoxidized_Fe/gradient_magnitude;
        vector_p.deoxidized_Mn = gradient.deoxidized_Mn/gradient_magnitude;

        double point_error {get_slag_MSE(heat, compositions, point)};

        double alpha {2};

        double point2_error {};
        double constant {0};

        do
        {
            alpha *= 0.5;
            point2.tapping_slag_mass = point.tapping_slag_mass + alpha*vector_p.tapping_slag_mass;
            point2.laddle_MgO = point.laddle_MgO + alpha*vector_p.laddle_MgO;
            point2.deoxidized_Fe = point.deoxidized_Fe + alpha*vector_p.deoxidized_Fe;
            point2.deoxidized_Mn = point.deoxidized_Mn + alpha*vector_p.deoxidized_Mn;
            point2.return_slag = point.return_slag;

            point2_error = get_slag_MSE(heat, compositions, point2);

            constant = 0;

            constant += vector_p.tapping_slag_mass*gradient.tapping_slag_mass;
            constant += vector_p.laddle_MgO*gradient.laddle_MgO;
            constant += vector_p.deoxidized_Fe*gradient.deoxidized_Fe;
            constant += vector_p.deoxidized_Mn*gradient.deoxidized_Mn;
            constant *= alpha*0.0001;
            constant += point_error;
            //std::cout << "point2_error:" << vector_p.laddle_MgO << std::endl;
            //std::cout << "constant:" << constant << std::endl;
        } while (point2_error > constant);

        point = point2;
    }
    return point2;
}
*/

/*
for(auto test_heat : heats)
    {
        parameters results {test_heat.steel_weight*5, 0.50, 0.50, 10, test_heat.steel_weight*2.5};
        double best_error {get_slag_MSE(test_heat, compositions, minimize(test_heat, compositions, results, 0.01, 0.000000001, 300, false))};
        for(float i {1}; i <= 25; i += 0.5)
        {   
            parameters temp {minimize(test_heat, compositions, {test_heat.steel_weight*(5 + i), 0.50, 0.50, 10, 200}, 0.01, 0.0000000000000001, 300, false)};
            double error {get_slag_MSE(test_heat, compositions, temp)};
            if(error < best_error)
            {
                best_error = error;
                results = temp;
            }
        }

        for(float i {20.0}; i <= test_heat.steel_weight*5; i += 20.0)
        {   
            parameters temp {results};
            temp.return_slag = i;
            temp = minimize(test_heat, compositions, temp, 0.01, 0.0000000000000001, 300, false);
            double error {get_slag_MSE(test_heat, compositions, temp)};
            if(error < best_error)
            {
                best_error = error;
                results = temp;
            }
        }

        results = minimize(test_heat, compositions, results, 0.01, 0.0000000001, 10000, true);
        
        //5 a 15% da escoria final fica de retorno
        std::cout << "Heat Number: " << test_heat.heat_number << std::endl;
        results_file << "Heat Number: " << test_heat.heat_number << std::endl;
        results_file << "Result Tapping Mass: " << results.tapping_slag_mass << std::endl;
        results_file << "Result Deoxidized Mn: " << results.deoxidized_Mn << std::endl;
        results_file << "Result Deoxidized Fe: " << results.deoxidized_Fe << std::endl;
        results_file << "Result Laddle MgO: " << results.laddle_MgO << std::endl;
        results_file << "Result Return Slag: " << results.return_slag << std::endl << std::endl;;

        slag_composition final_calculated_slag {get_calculated_slag(test_heat, compositions, results)};

        results_file << "Calculated/Measured CaO: " << final_calculated_slag.CaO*100 << " / " << test_heat.laddle.final_slag.CaO*100 << std::endl;
        results_file << "Calculated/Measured MgO: " << final_calculated_slag.MgO*100 << " / " << test_heat.laddle.final_slag.MgO*100 << std::endl;
        results_file << "Calculated/Measured SiO2: " << final_calculated_slag.SiO2*100 << " / " << test_heat.laddle.final_slag.SiO2*100 << std::endl;
        results_file << "Calculated/Measured Al2O3: " << final_calculated_slag.Al2O3*100 << " / " << test_heat.laddle.final_slag.Al2O3*100 << std::endl;
        results_file << "Calculated/Measured FeO: " << final_calculated_slag.FeO*100 << " / " << test_heat.laddle.final_slag.FeO*100 << std::endl;
        results_file << "Calculated/Measured MnO: " << final_calculated_slag.MnO*100 << " / " << test_heat.laddle.final_slag.MnO*100 << std::endl;
        results_file << "Calculated/Measured CaF2: " << final_calculated_slag.CaF2*100 << " / " << test_heat.laddle.final_slag.CaF2*100 << std::endl << std::endl;

        results_file << "Final MSE: " << get_slag_MSE(test_heat, compositions, results)*100 << std::endl;
        results_file << "Final Absolute Error: " << get_slag_absolute_error(test_heat, compositions, results)*100 << std::endl;
        results_file << "Final Relative Error: " << get_slag_relative_error(test_heat, compositions, results)*100 << std::endl;
        results_file << "Final Standard Deviation: " << find_std_dev_slag_error(get_calculated_slag(test_heat, compositions, results), test_heat.laddle.final_slag)*100 << std::endl << std::endl;
        results_file << "---------------------------------------------------------------------------------" << std::endl << std::endl;
        }
    std::cout << "Done" << std::endl;
    results_file.close();
}
*/