
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

class Device 
{
public:
	Device();
	~Device();
	virtual string Simulation_Code(string,string,string);
	virtual string Simulation_Code(string,string);
	virtual void Mutate_Feature();
	virtual void Undo_Feature_Mutation();
	virtual string Device_Type();
	virtual double  Get_Features(string);
	virtual int Get_Terminal_Numbers();
	virtual int Get_Device_Index();
	virtual void Store_Feature_Genes(std::ofstream&);
};

class Transistor : public Device
{

public:
	Transistor(bool,int, int, string,double, double);
	~Transistor();
	string Simulation_Code(string,string,string); // prints one line of its Hspice code
	void Mutate_Feature();
	void Undo_Feature_Mutation();
	string Device_Type(); 
	double  Get_Features(string);
	int Get_Terminal_Numbers();
	int Get_Device_Index();
	void Store_Feature_Genes(std::ofstream&);
	
private:
	int Index; 
	int Number_of_Terminals;
	string Device_type;
	bool Pmos;
	double length;
	double width;
	double org_width, org_length;
	// Other parameters to be added
};

class Capacitor : public Device
{
	
public:
	Capacitor(int,double,string);
	~Capacitor();
	string Simulation_Code(string,string);
	string Device_Type();
	void Mutate_Feature();
	void Undo_Feature_Mutation();
	double  Get_Features(string);
	int Get_Terminal_Numbers();
	int Get_Device_Index();
	void Store_Feature_Genes(std::ofstream& FeatureFile);
private:
	int Index; 
	int Number_of_Terminals;
	string Device_type;
	double capacitance;
	double org_capacitance;
	
};

class Resistor : public Device
{
public:
	Resistor(int,double, string);
	~Resistor();
	string Simulation_Code(string,string);
	string Device_Type();
	void Mutate_Feature();
	void Undo_Feature_Mutation();
	double  Get_Features(string);
	int Get_Terminal_Numbers();
	int Get_Device_Index();
	void Store_Feature_Genes(std::ofstream& FeatureFile);
private:
	int Index;
	int Number_of_Terminals;
	string Device_type;
	double resistance;
	double org_resistance;

};
