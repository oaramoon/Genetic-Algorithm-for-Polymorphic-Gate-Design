#include "Devices.h"
#include <stdlib.h>
#include <cstdlib>
using namespace std;
Device:: Device(){
	//printf("Device Contructor\n");
}
Device:: ~Device(){
}
string Device:: Simulation_Code(string,string){
}
string Device:: Simulation_Code(string,string,string){
}
void Device:: Mutate_Feature(){
}
int Device::Get_Terminal_Numbers(){	
}
int Device::Get_Device_Index(){	
}
double Device::Get_Features(string){	
}
void Device::Store_Feature_Genes(std::ofstream&){
}
string Device::Device_Type(){
}
void Device::Undo_Feature_Mutation(){
}
///////////////////////////////////////////////////////

Transistor:: Transistor(bool pmos,int Tran_index, int Num_Terminals, string Dev_type, double W, double L){
	//printf("Transistor Contructor\n");
	Number_of_Terminals = 3;
	Pmos = pmos;
	Number_of_Terminals = Num_Terminals;
	Index = Tran_index;
	Device_type = Dev_type;
	length = L;
	width = W;
	org_length = L;
	org_width = W;
}
Transistor:: ~Transistor(){
	//// left blank intentionally
}
string Transistor:: Simulation_Code(string S_NetName, string G_NetName, string D_NetName){
	std::stringstream temp;
	temp << "M"<< Index<<" " << D_NetName << " " << G_NetName << " " << S_NetName << " ";
	if (Pmos){
		temp << "vdd ";
	}
	else{
		temp << "vss ";
	}
	temp << Device_type << " ";
	temp << "l="<<Get_Features("Length")<<"u ";
	temp << "w="<<Get_Features("Width")<<"u\n";
	string output = temp.str();
	return output;
}

void Transistor:: Mutate_Feature(){
	
	double new_width = (rand() % 10 + 1)*0.4; //Step Size must be checked
	while(width == new_width){
		new_width = (rand() % 10 + 1)*0.4; //Step Size must be checked
	}
	width = new_width;
	
	double new_length = (rand() % 10 + 1)*0.4; /////// Step Size must be checked
	while(length == new_length){
		new_length = (rand() % 10 + 1)*0.4; //Step Size must be checked
	}
	length = new_length;
}

double Transistor:: Get_Features(string Feature){

	if (Feature.compare("Width") == 0 || Feature.compare("width") == 0){
		return width;
	}
	if (Feature.compare("Length") == 0 || Feature.compare("length") == 0){
		return length;
	}
}
int Transistor::Get_Terminal_Numbers(){
	return Number_of_Terminals;
}
int Transistor::Get_Device_Index(){
	return Index;
}
void Transistor::Store_Feature_Genes(std::ofstream& FeatureFile){
	FeatureFile << length ;
	FeatureFile << " ";
	FeatureFile << width  << "\n";
}

string Transistor::Device_Type(){
	return "Transistor";
}

void Transistor::Undo_Feature_Mutation(){
	length = org_length;
	width = org_width;
}
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Capacitor:: Capacitor(int index,double cap , string Dev_name){
	//printf("Capacitor Contructor\n");
	Number_of_Terminals = 2;
	Index = index;
	capacitance = cap;
	org_capacitance = cap;
	Device_type = Dev_name;
}
Capacitor:: ~Capacitor(){
	// intentionally left blank
}
string Capacitor::Simulation_Code(string PositiveNet , string NegativeNet){
	std::stringstream temp;
	temp << "C"<< Index<<" " << PositiveNet << " " << NegativeNet << " " << Device_type << " " << capacitance <<"u\n";
	string output = temp.str();
	return output;
}

int Capacitor::Get_Terminal_Numbers(){
	return Number_of_Terminals;
}
int Capacitor::Get_Device_Index(){
	return Index;
}

void Capacitor::Mutate_Feature(){
	double new_capacitance = (rand() % 10 + 1)*0.1; //Step Size must be checked
	while(capacitance == new_capacitance){
		new_capacitance = (rand() % 10 + 1)*0.1; //Step Size must be checked
	}
	capacitance = new_capacitance;
	
}

double Capacitor::Get_Features(string Feature){
	if (Feature.compare("Capacitance") == 0 || Feature.compare("capacitance") == 0){
		return capacitance;
	}
}
void Capacitor::Store_Feature_Genes(std::ofstream& FeatureFile){
	FeatureFile << capacitance << "\n"; 
}

string Capacitor::Device_Type(){
	return "Capacitor";	
}

void Capacitor::Undo_Feature_Mutation(){
	capacitance = org_capacitance;
}
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Resistor:: Resistor(int index,double res, string Dev_name){
	//printf("Resistor Contructor\n");
	Number_of_Terminals = 2;
	Index = index;
	resistance = res;
	org_resistance = res;
	Device_type = Dev_name;
}
Resistor:: ~Resistor(){
 /// intentionally left blank
}
string Resistor:: Simulation_Code(string PositiveNet , string NegativeNet){
	std::stringstream temp;
	temp << "R"<< Index<<" " << PositiveNet << " " << NegativeNet << " " << Device_type << " " << resistance << "u\n";
	string output = temp.str();
	return output;
}

int Resistor::Get_Terminal_Numbers(){
	return Number_of_Terminals;
}
int Resistor::Get_Device_Index(){
	return Index;
}

void Resistor::Mutate_Feature(){
	double new_resistance = (rand() % 10 + 1)*0.2; //Step Size must be checked
	while(resistance == new_resistance){
		new_resistance = (rand() % 10 + 1)*0.2; //Step Size must be checked
	}
	resistance = new_resistance;	
}

double Resistor::Get_Features(string Feature){
	if (Feature.compare("resistance") == 0 || Feature.compare("Resistance") == 0){
		return resistance;
	}
}

void Resistor::Store_Feature_Genes(std::ofstream& FeatureFile){
	FeatureFile << resistance << "\n"; 
}

string Resistor::Device_Type(){
	return "Resistor";
}

void Resistor::Undo_Feature_Mutation(){
	resistance = org_resistance;
}