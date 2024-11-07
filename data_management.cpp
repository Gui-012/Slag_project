#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <thread>
#include <time.h>

//Molar mass ratio between Si and SiO2
#define Si_SiO2_ratio 60/28

//Molar mass ratio between Ca and CaO
#define Ca_CaO_ratio  56/40

//Number of components in steel composition vectors
#define STEEL_COMP_NUM   8

//Number of components in slag composition vectors
#define SLAG_COMP_NUM   7

//Number of additions present in both tapping and laddle additions
#define SHARED_ADDITIONS_ID 11

//Upper index of tapping only additions in additions vector
#define TAPPING_ADDITIONS 16

//Upper index of laddle only additions in additions vector
#define LADDLE_ADDITIONS_ID 21

//Number of materials in the composition table
#define COMPOSITION_MATERIALS 21

//Number of elements in the composition table
#define COMPOSITION_ELEMENTS 13

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

//Paramers to be found
struct parameters
{
    double tapping_slag_mass {};
    double deoxidized_Mn {};
    double deoxidized_Fe {};
    double laddle_MgO {};
    double return_slag {};
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
    parameters results {};
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

//Checks if data headings are the same as template and moves cursor to first data line
bool check_data_headings(std::ifstream &data_file)
{
    std::string line {};
    std::getline(data_file, line, ',');
    line = line.substr(3, 13);
    //First line of the csv file
    if(line.compare("VAZAMENTO FEA") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \"VAZAMENTO FEA\"" << std::endl;
        return false;
    }
    //skips for empty lines
    for(int i {0}; i < 5; i++)
    {
        std::getline(data_file, line);
    }

    //Gets line can compares to template
    std::getline(data_file, line);
    line = line.substr(0, 126);
    if(line.compare(",,,,,,Adicões,,,,,,,,,,,,,,,,,Composição Química Aço,,,,,,,,Composição Química escória,,,,,,,,,Observações corridas") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \",,,,,,Adicões,,,,,,,,,,,,,,,,,Composição Química Aço,,,,,,,,Composição Química escória,,,,,,,,,Observações corridas\"" << line << std::endl;
        return false;
    }

    std::getline(data_file, line);
    line = line.substr(0, 109);
    if(line.compare("Informações de corrida,,,,,,Fundentes/Formadores de escória,,,,,,Ligas/Desoxidantes,,,,,,Resíduos,,Outros") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \"Informações de corrida,,,,,,Fundentes/Formadores de escória,,,,,,Ligas/Desoxidantes,,,,,,Resíduos,,Outros\"" << std::endl;
        return false;
    }

    std::getline(data_file, line);
    line = line.substr(0, 353);
    if(line.compare("Contagem,Data,N° Corrida,Peso Aço,Tipo de aço,T°Vazamento,Cal Calcítica,Cal Dolomítica,Fluorita,Alumina,TS XXXX,CaC2,Carbono,FeSi,FeSiMn,SiC,Resíduo CaSi,Alumínio metálico,Areia,Escória de retorno,Aditivo de vazamento extra 1,Aditivo de vazamento extra 2,Aditivo de vazamento extra 3,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \"Contagem,Data,N° Corrida,Peso Aço,Tipo de aço,T°Vazamento,Cal Calcítica,Cal Dolomítica,Fluorita,Alumina,TS XXXX,CaC2,Carbono,FeSi,FeSiMn,SiC,Resíduo CaSi,Alumínio metálico,Areia,Escória de retorno,Aditivo de vazamento extra 1,Aditivo de vazamento extra 2,Aditivo de vazamento extra 3,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2\"" << std::endl;
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
        return 1;
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
        return 1;
    }
    return number;
}

//Reads steel composition from file and returns in a steel_composition struct
steel_composition read_steel_composition(std::ifstream &data_file)
{
    std::string cell {};
    steel_composition steel {};
    //C
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

//Reads slag composition from file and returns in a steel_composition struct
slag_composition read_slag_composition(std::ifstream &data_file)
{
    std::string cell {};
    slag_composition slag {};
    //CaO
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

//Reads data from the tapping section of the data table
void read_tapping_data(std::vector<heat> &heats, std::ifstream &data_file)
{
    int heat_count {0};
    std::string cell {};
    std::getline(data_file, cell, ',');

    //Reads multiple heats (Checking the order )
    while(string_to_int(cell) == heat_count + 1)
    {
        heats.push_back({});
        //date cell
        std::getline(data_file, cell, ',');
        heats.at(heat_count).date = cell;
        //heat number
        std::getline(data_file, cell, ',');
        heats.at(heat_count).heat_number = string_to_int(cell);
        //Steel weight
        std::getline(data_file, cell, ',');
        heats.at(heat_count).steel_weight = string_to_double(cell);
        //Steel type
        std::getline(data_file, cell, ',');
        heats.at(heat_count).steel_type = cell;
        //tapping temperature
        std::getline(data_file, cell, ',');
        heats.at(heat_count).tapping.tapping_temperature = string_to_double(cell);
        //tapping additions
        for(int i {0}; i <= TAPPING_ADDITIONS; i++)
        {
            std::getline(data_file, cell, ',');
            heats.at(heat_count ).tapping.additions.push_back(string_to_double(cell));
        }
        //laddle only additions
        for(int i {TAPPING_ADDITIONS + 1}; i <= LADDLE_ADDITIONS_ID; i++)
        {
            heats.at(heat_count).tapping.additions.push_back(0);
        }
        //tapping steel
        heats.at(heat_count).tapping.steel = read_steel_composition(data_file);
        //Tapping slag
        heats.at(heat_count).tapping.slag = read_slag_composition(data_file);
        //Ignore rest of line
        std::getline(data_file, cell);
        //Get first cell in next line
        std::getline(data_file, cell, ',');
        heat_count++;
    }
}

//Check if headings on laddle data section are the same as template and moves cursor to first data line
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
        std::cout << "Should be: \"FORNO PANELA\"" << std::endl;
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
        std::cout << "Should be: \"Informações de corrida,,,,,,,Adicões,,,,,,,Ligas e desoxidades,,,,,,,Outros,,,Composição Química Aço (CFP),,,,,,,,Composição Química Aço (FFP),,,,,,,,Composição Química escória (CFP),,,,,,,,,Composição Química escória (FFP),,,,,,,,,Observações\"" << std::endl;
        return false;
    }

    std::getline(data_file, line);
    line = line.substr(0, 63);
    if(line.compare(",,,,,,,Fundentes/Formadores de escória,,,,,,Ligas/Desoxidantes") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \",,,,,,,Fundentes/Formadores de escória,,,,,,Ligas/Desoxidantes\"" << std::endl;
        return false;
    }
    
    std::getline(data_file, line);
    line = line.substr(0, 400);
    if(line.compare("Contagem,Data,N° Corrida,Peso Aço,Tipo de aço,T°iFP,T°fFP,Cal Calcítica,Cal Dolomítica,Fluorita,Alumina,TS ***,CaC2,Carbono,FeSi,FeSiMn,SiC,CaSi,Alumínio metálico,FeS,Mn Eletrolítico,Aditivo de FP extra 1,Aditivo de FP extra 2,Aditivo de FP extra 3,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2,Basicidade,Fechamento,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \"Contagem,Data,N° Corrida,Peso Aço,Tipo de aço,T°iFP,T°fFP,Cal Calcítica,Cal Dolomítica,Fluorita,Alumina,TS ***,CaC2,Carbono,FeSi,FeSiMn,SiC,CaSi,Alumínio metálico,FeS,Mn Eletrolítico,Aditivo de FP extra 1,Aditivo de FP extra 2,Aditivo de FP extra 3,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,C,Si,Mn,P,S,Nb,ppm O2 ,Mn/S,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2,Basicidade,Fechamento,CaO,MgO,SiO2,Al2O3,FeO,MnO,CaF2\"" << std::endl;
        return false;
    }

    return true;
}

//Reads data from the laddle data section of the data table
void read_laddle_data(std::vector<heat> &heats, std::ifstream &data_file)
{
    int heat_count {0};
    std::string cell {};
    std::getline(data_file, cell, ',');
    while(string_to_int(cell) == heat_count + 1)
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
        heats.at(heat_count).laddle.initial_temperature = string_to_double(cell);
        //laddle final temperature
        std::getline(data_file, cell, ',');
        heats.at(heat_count).laddle.final_temperature = string_to_double(cell);
        //Shared additions
        for(int i {0}; i <= SHARED_ADDITIONS_ID; i++)
        {
            std::getline(data_file, cell, ',');
            heats.at(heat_count).laddle.additions.push_back(string_to_double(cell));
        }
        //Skips tapping only additions
        for(int i {SHARED_ADDITIONS_ID + 1}; i <= TAPPING_ADDITIONS; i++)
        {
            heats.at(heat_count).laddle.additions.push_back(0);
        }
        //Reads laddle only additions
        for(int i {TAPPING_ADDITIONS + 1}; i <= LADDLE_ADDITIONS_ID; i++)
        {
            std::getline(data_file, cell, ',');
            heats.at(heat_count).laddle.additions.push_back(string_to_double(cell));
        }
        //laddle initial steel
        heats.at(heat_count).laddle.initial_steel = read_steel_composition(data_file);
        //laddle final steel
        heats.at(heat_count).laddle.final_steel = read_steel_composition(data_file);
        //Laddle initial slag
        heats.at(heat_count).laddle.initial_slag = read_slag_composition(data_file);
        //Ignore two lines
        std::getline(data_file, cell, ',');
        std::getline(data_file, cell, ',');
        //Laddle final slag
        heats.at(heat_count).laddle.final_slag = read_slag_composition(data_file);
        //Ignore rest of line
        std::getline(data_file, cell);
        //Get first cell in next line
        std::getline(data_file, cell, ',');
        heat_count++;
    }
}

//Check if headings on composition table are the same as template and moves cursor to first data line
bool check_compositions_headings(std::ifstream &comp_file)
{
    std::string line {};
    //First line of the csv file
    std::getline(comp_file, line, ',');
    line = line.substr(3, 130);
    if(line.compare("Elementos de adição") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \"Elementos de adição\"" << std::endl;
        return false;
    }
    //skip rest of line
    std::getline(comp_file, line);

    std::getline(comp_file, line);
    line = line.substr(0, 130);
    if(line.compare("Nome,Tipo,C,Si,Mn,P,S,Ca,CaO,SiO2,MgO,CaF2,Al2O3,FeO,Fe2O3,CaC2,Rendimento Fe,Rendimento Si,Rendimento Mn,Rendimento Ca,Fechamento") != 0)
    {
        std::cout << "Wrong line:" << line << std::endl;
        std::cout << "Should be: \"Nome,Tipo,C,Si,Mn,P,S,Ca,CaO,SiO2,MgO,CaF2,Al2O3,FeO,Fe2O3,CaC2,Rendimento Fe,Rendimento Si,Rendimento Mn,Rendimento Ca,Fechamento\"" << std::endl;
        return false;
    }
    return true;
}

//Reads data from the composition table
std::vector<std::vector<double>> read_composition_data(std::ifstream &comp_file)
{
    std::vector<std::vector<double>> compositions {};
    std::string cell {};
    //For each material in the composition table
    for(int material_id {0}; material_id <= COMPOSITION_MATERIALS; material_id++)
    {
        std::vector<double> comp {};
        std::getline(comp_file, cell, ',');
        std::getline(comp_file, cell, ',');
        //Reads all the elements composition percentage in the material 
        for(int element_id {0}; element_id <= COMPOSITION_ELEMENTS; element_id++)
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

//Calculates final slag mass in a heat
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
    //Calculates error for each slag element
    errors.push_back(calculated_slag.CaO - measured_slag.CaO);
    errors.push_back(calculated_slag.MgO - measured_slag.MgO);
    errors.push_back(calculated_slag.SiO2 - measured_slag.SiO2);
    errors.push_back(calculated_slag.Al2O3 - measured_slag.Al2O3);
    errors.push_back(calculated_slag.FeO - measured_slag.FeO);
    errors.push_back(calculated_slag.MnO - measured_slag.MnO);
    errors.push_back(calculated_slag.CaF2 - measured_slag.CaF2);

    double MSE {0};
    //Sums squares of errors
    for(double x : errors)
    {
        MSE += x*x;
    }
    
    //Returns average of squared errors
    return MSE /= errors.size();
}

//Returns mean squared error of the slag compositioni using heat, composition, and parameters
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
    
    return error / SLAG_COMP_NUM;
}

//Returns relative error of the slag compositioni using heat, composition, and parameters
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
    
    return error / SLAG_COMP_NUM;
}

//Returns absolute error of the slag compositioni using heat, composition, and parameters
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

//Calculates the next point (Parameters) in minimization process
parameters get_next_point(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters point, double const &delta)
{
    double derivative {0};
    parameters point2 {point};
    double point_MSE {get_slag_MSE(heat, compositions, point)};
    
    //Increase tapping slag mass by small step
    point2.tapping_slag_mass += delta * point.tapping_slag_mass;
    //Calculated derivative
    derivative = (get_slag_MSE(heat, compositions, point2) - point_MSE) / (delta * point.tapping_slag_mass);
    //Moves point in the direction of the derivative
    point.tapping_slag_mass -= derivative*point.tapping_slag_mass;
    //Resets point 2
    point2.tapping_slag_mass = point.tapping_slag_mass;

    point2.laddle_MgO += delta * point.laddle_MgO;
    derivative = (abs(get_calculated_slag(heat, compositions, point2).MgO - heat.laddle.final_slag.MgO) - abs(get_calculated_slag(heat, compositions, point).MgO - heat.laddle.final_slag.MgO)) / (delta * point.laddle_MgO);
    point.laddle_MgO -= derivative*point.laddle_MgO;
    point2.laddle_MgO = point.laddle_MgO;

    point2.deoxidized_Mn += delta * point.deoxidized_Mn;
    //Use slag MnO error for optimization
    derivative = (abs(get_calculated_slag(heat, compositions, point2).MnO - heat.laddle.final_slag.MnO) - abs(get_calculated_slag(heat, compositions, point).MnO - heat.laddle.final_slag.MnO)) / (delta * point.deoxidized_Mn);
    point.deoxidized_Mn -= derivative*point.deoxidized_Mn*delta;
    //Checks that deoxidization is at maximum 100%
    point.deoxidized_Mn = point.deoxidized_Mn <= 1 ? point.deoxidized_Mn : 1;
    point2.deoxidized_Mn = point.deoxidized_Mn;

    point2.deoxidized_Fe += delta * point.deoxidized_Fe;
    //Use slag FeO error for optimization
    derivative = (abs(get_calculated_slag(heat, compositions, point2).FeO - heat.laddle.final_slag.FeO) - abs(get_calculated_slag(heat, compositions, point).FeO - heat.laddle.final_slag.FeO)) / (delta * point.deoxidized_Fe);
    point.deoxidized_Fe -= derivative*point.deoxidized_Fe*delta;
    //Checks that deoxidization is at maximum 100%
    point.deoxidized_Fe = point.deoxidized_Fe <= 1 ? point.deoxidized_Fe : 1;
    point2.deoxidized_Fe = point.deoxidized_Fe;

    return point2;
}

//Minimizes the MSE of the slag composition using modified gradient algorithm and stores result in heat struct
parameters minimize(heat const &heat, std::vector<std::vector<double>> const &compositions, parameters start_point, double step_size, int max_iterations)
{
    parameters point {start_point};
    int n {0};
    double original_step_size {step_size};
    //Ensures maximum iterations of algorithm
    while(n < max_iterations)
    {
        n += 1;
        step_size = original_step_size;

        //Sets initial error in starting point
        double start {get_slag_MSE(heat, compositions, point)};
        
        //Finds next point
        parameters point2 {get_next_point(heat, compositions, point, step_size)};
        double end {get_slag_MSE(heat, compositions, point2)};
        int i {0};
        bool done {false};

        //If the starting point has higher error
        while(start < end)
        {
            if(i > 150)
            {
                return point;
            }

            //Decreases step size to find lower error
            step_size /= 2;
            point2 = get_next_point(heat, compositions, point2, step_size);
            end = get_slag_MSE(heat, compositions, point2);
            i++;
        }
        point = point2;
    }
    return point;
}

std::vector<heat> heats{};

//Minimizes the slag error in a heat
void solve_heat(int id, std::vector<std::vector<double>> compositions)
{
    heats.at(id).results = {heats.at(id).steel_weight*5, 0.50, 0.50, 10, heats.at(id).steel_weight*2.5};
    double best_error {get_slag_MSE(heats.at(id), compositions, minimize(heats.at(id), compositions, heats.at(id).results, 0.01, 500))};
    for(float i {1}; i <= 25; i += 0.5)
    {   
        parameters temp {minimize(heats.at(id), compositions, {heats.at(id).steel_weight*(5 + i), 0.50, 0.50, 10, 200}, 0.01, 500)};
        double error {get_slag_MSE(heats.at(id), compositions, temp)};
        if(error < best_error)
        {
            best_error = error;
            heats.at(id).results = temp;
        }
    }
    for(float i {20.0}; i <= heats.at(id).steel_weight*5; i += 20.0)
    {   
        parameters temp {heats.at(id).results};
        temp.return_slag = i;
        temp = minimize(heats.at(id), compositions, temp, 0.01, 500);
        double error {get_slag_MSE(heats.at(id), compositions, temp)};
        if(error < best_error)
        {
            best_error = error;
            heats.at(id).results = temp;
        }
    }

    heats.at(id).results = minimize(heats.at(id), compositions, heats.at(id).results, 0.01, 20000);
    return;
}

//Sums all elements in a vector
double add_vector(std::vector<double> vec)
{
    double sum {0};
    for(auto x : vec)
    {
        sum += x;
    }
    return sum;
}

int main()
{
    //Opens data table file
    std::string file_name  {read_data_file_name()};
    std::ifstream data_file (file_name);
    if(!data_file.is_open())
    {
        std::cout << "Could not open file:\"" << file_name << "\"" << std::endl;
        return 1;
    }

    //Opens Compositions table file
    file_name = read_comp_file_name();
    std::ifstream compositions_file (file_name);
    if(!compositions_file.is_open())
    {
        std::cout << "Could not open file:\"" << file_name << "\"" << std::endl;
        return 1;
    }

    //Reads data from data file
    if(!check_data_headings(data_file))
    {
        std::cout << "Invalid Data Table Format" << std::endl;
        return 1;
    }
    read_tapping_data(heats, data_file);
    if(!check_laddle_headings(data_file))
    {
        std::cout << "Invalid Data Table Format" << std::endl;
        return 1;
    }
    read_laddle_data(heats, data_file);

    //Reads material compositions from compositions file
    if(!check_compositions_headings(compositions_file))
    {
        std::cout << "Invalid Composition Table Format" << std::endl;
        return 1;
    }
    std::vector<std::vector<double>> compositions {read_composition_data(compositions_file)};
    std::ofstream results_file;
    data_file.close();
    compositions_file.close();

    //Opens file to stores results
    file_name = read_return_file_name();
    results_file.open(file_name);
    if(!results_file.is_open())
    {
        std::cout << "Could not open " << file_name << " file" << std::endl;
        return 1;
    }
    std::cout << "Running..." << std::endl;

    //Initiates clock to measure program runtime
    clock_t t;
    t = clock();
    
    //Sets each heat solving as its own thread for multithreading optimization
    std::vector<std::thread> threads {};
    for(int i {0}; i < heats.size(); i++)
    {
        threads.push_back(std::thread(solve_heat, i, compositions));
    }

    //Waits for all threads to finish
    for(auto &th : threads)
    {
        th.join();
    }

    //Vectores use to store and calculate errors from all heats
    std::vector<double> ovr_MSE {};
    std::vector<double> ovr_abs_error {};
    std::vector<double> ovr_rel_error {};
    std::vector<double> ovr_stdev {};
    for(auto test_heat : heats)
    {
        //5 a 15% da escoria final fica de retorno
        //std::cout << "Heat Number: " << test_heat.heat_number << std::endl;
        results_file << "Heat Number: " << test_heat.heat_number << std::endl;
        results_file << "Result Tapping Mass: " << test_heat.results.tapping_slag_mass << std::endl;
        results_file << "Result Deoxidized Mn: " << test_heat.results.deoxidized_Mn << std::endl;
        results_file << "Result Deoxidized Fe: " << test_heat.results.deoxidized_Fe << std::endl;
        results_file << "Result Laddle MgO: " << test_heat.results.laddle_MgO << std::endl;
        results_file << "Result Return Slag: " << test_heat.results.return_slag << std::endl << std::endl;;

        slag_composition final_calculated_slag {get_calculated_slag(test_heat, compositions, test_heat.results)};

        results_file << "Calculated/Measured CaO: " << final_calculated_slag.CaO*100 << " / " << test_heat.laddle.final_slag.CaO*100 << std::endl;
        results_file << "Calculated/Measured MgO: " << final_calculated_slag.MgO*100 << " / " << test_heat.laddle.final_slag.MgO*100 << std::endl;
        results_file << "Calculated/Measured SiO2: " << final_calculated_slag.SiO2*100 << " / " << test_heat.laddle.final_slag.SiO2*100 << std::endl;
        results_file << "Calculated/Measured Al2O3: " << final_calculated_slag.Al2O3*100 << " / " << test_heat.laddle.final_slag.Al2O3*100 << std::endl;
        results_file << "Calculated/Measured FeO: " << final_calculated_slag.FeO*100 << " / " << test_heat.laddle.final_slag.FeO*100 << std::endl;
        results_file << "Calculated/Measured MnO: " << final_calculated_slag.MnO*100 << " / " << test_heat.laddle.final_slag.MnO*100 << std::endl;
        results_file << "Calculated/Measured CaF2: " << final_calculated_slag.CaF2*100 << " / " << test_heat.laddle.final_slag.CaF2*100 << std::endl << std::endl;

        results_file << "Final MSE: " << get_slag_MSE(test_heat, compositions, test_heat.results)*100 << std::endl;
        results_file << "Final Absolute Error: " << get_slag_absolute_error(test_heat, compositions, test_heat.results)*100 << std::endl;
        results_file << "Final Relative Error: " << get_slag_relative_error(test_heat, compositions, test_heat.results)*100 << std::endl;
        results_file << "Final Standard Deviation: " << find_std_dev_slag_error(get_calculated_slag(test_heat, compositions, test_heat.results), test_heat.laddle.final_slag)*100 << std::endl << std::endl;
        results_file << "---------------------------------------------------------------------------------" << std::endl << std::endl;

        ovr_MSE.push_back(get_slag_MSE(test_heat, compositions, test_heat.results)*100);
        ovr_abs_error.push_back(get_slag_absolute_error(test_heat, compositions, test_heat.results)*100);
        ovr_rel_error.push_back(get_slag_relative_error(test_heat, compositions, test_heat.results)*100);
        ovr_stdev.push_back(find_std_dev_slag_error(get_calculated_slag(test_heat, compositions, test_heat.results), test_heat.laddle.final_slag)*100);
    }
    std::cout << "Done" << std::endl;
    t = clock() - t;
    results_file << "Runtime: " << ((float)t)/CLOCKS_PER_SEC << std::endl;
    results_file << "Overall MSE: " << add_vector(ovr_MSE)/ovr_MSE.size() << std::endl;
    results_file << "Overall Absolute Error: " << add_vector(ovr_abs_error)/ovr_abs_error.size() << std::endl;
    results_file << "Overall Relative Error: " << add_vector(ovr_rel_error)/ovr_rel_error.size() << std::endl;
    results_file << "Overall Standard Deviation: " << add_vector(ovr_stdev)/ovr_stdev.size() << std::endl;
    results_file.close();
}