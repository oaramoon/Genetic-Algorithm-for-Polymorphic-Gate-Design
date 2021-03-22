#include "Devices.h"
#include <vector>
#include<set>
#include<fstream>
using namespace std;

typedef struct element_struct {
	string device_type; // Transistor, Resistance, Capacitance
	string device_model; // type of Transistor..
	int Terminal_Num;
} element;



class Generation
{
	public:
	Generation(); // checked
	~Generation(); // checked
	friend class Individual;
	void Evolve();
	string WhatGateIsIt(std::vector<int>&); //checked
	bool Resulted_Functions(std::vector<double>& ); //checked
	void Evaluate_Individuals(); //checked
	friend void Resurrection(Generation G, int Gen, int Ind_id);
	void Test();

	private:
	int Number_of_Components;
	int total_number_of_terminals;
	int number_of_inputs;
	int activation_signal; // 1 for temerature, 2 for external signal and 3 for vdd
	int total_control_signal_variations;
	int population_size;
	int max_number_of_generation;
	int current_generation;
	int number_of_ancestors;
	double high_thresh; //Voltage >high_thresh * vdd can be set as high
	double low_thresh; //Voltage <low_thresh * vdd can be set as low 
	double VDD;
	double Trigger_Low,Trigger_High;
	bool SilentMode;
	string LibFileAddress;
	std:: vector <element> Devices_info;

	bool Debug;

};



class Individual 
{
public:
	Individual(Generation, int, std::ifstream&, std::ifstream&); // checked
	~Individual();
	void Mutate(Generation); //checked
	void Undo_Mutation(); /// checked
	void Simulation_Code(Generation); //checked
	void Simulation_Code(Generation,int);
	void Define_Hspice_Nodes_Based_On_Connection_Mapping(Generation); //checked
	void Store_Genes(Generation); //checked
	void Set_ID(int); // checked
	void DFS(int,int); //checked
	
	void Test();
	string GetMeHspiceNodeName(int,Generation);
private:
	Device**  components;
	int Number_of_Components;
	std::vector <int> Connection_Mappings, org_Connection_Mappings;
	std::vector <int> Hspice_Node_Indecies_For_Terminals;
	std::vector<set<int>> Terminals_Adj_list;
	int Individual_ID;

};

