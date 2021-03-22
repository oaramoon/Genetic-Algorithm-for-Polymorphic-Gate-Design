#include "Population.h"
#include <iomanip>
#include <math.h>
#include <cstdlib>
Individual:: Individual(Generation G, int ID, std::ifstream& FeatureFile,std::ifstream& ConnectionFile){

    Individual_ID = ID;
    Number_of_Components = G.Number_of_Components;
    components = new Device*[G.Number_of_Components];
    double width, length, capacitance, resistance ;
    for(int i=0;i <G.Number_of_Components;i++){
        if (G.Devices_info[i].device_type.compare("pmos") == 0 ){
            FeatureFile  >> length >> width ;
            Transistor * T = new Transistor(1,i+1,G.Devices_info[i].Terminal_Num,G.Devices_info[i].device_model,width,length);
            components[i] = T;
        }
        else if(G.Devices_info[i].device_type.compare("nmos") == 0 ){
            FeatureFile  >> length >> width ;
            Transistor * T = new Transistor(0,i+1,G.Devices_info[i].Terminal_Num,G.Devices_info[i].device_model,width,length);
            components[i] = T;
        }
        else if(G.Devices_info[i].device_type.compare("Capacitor") == 0){
            FeatureFile >> capacitance ;
            Capacitor * C = new Capacitor(i+1,capacitance,"");
            components[i] = C;
        }
        else if(G.Devices_info[i].device_type.compare("Resistor") == 0){
            FeatureFile >> resistance;
            Resistor * R = new Resistor(i+1,resistance,"");
            components[i] = R;
        }
    }
    int read_connection_index;
    for (int i = 0; i < G.total_number_of_terminals; i++){
        ConnectionFile >> read_connection_index;
        Connection_Mappings.push_back(read_connection_index);
        org_Connection_Mappings.push_back(read_connection_index);
    }
}
void Individual::Test(){

    cout << components[0]->Device_Type().compare("Transistor") << endl;
}
Individual:: ~Individual(){ 
    for (int i=0; i < Number_of_Components;i++){
        delete components[i];
    }
    delete[] components;
}
void Individual::Undo_Mutation(){
    Connection_Mappings = org_Connection_Mappings; // needs to be checked
    for (int Dev = 0 ; Dev < Number_of_Components ; Dev++){
        components[Dev]->Undo_Feature_Mutation();
    }
    Hspice_Node_Indecies_For_Terminals.clear();
    Hspice_Node_Indecies_For_Terminals.shrink_to_fit();
    Terminals_Adj_list.clear();
    Terminals_Adj_list.shrink_to_fit();
}

void Individual::Define_Hspice_Nodes_Based_On_Connection_Mapping(Generation G){

    for (int i=0;i< G.total_number_of_terminals + (3 + G.number_of_inputs);i++){
        Hspice_Node_Indecies_For_Terminals.push_back(-1);
    }
    for (int i=0; i < G.total_number_of_terminals + (3 + G.number_of_inputs); i++){
        set<int> temp;
        Terminals_Adj_list.push_back(temp);
    }
    //convert the directed graph to undirected and create its adjacency list 
    for( int i=0; i < G.total_number_of_terminals + (3+G.number_of_inputs); i++){
        if ((3+G.number_of_inputs) <= i){
            Terminals_Adj_list[i].insert(Connection_Mappings[i-(3+G.number_of_inputs)]);
            Terminals_Adj_list[Connection_Mappings[i-(3+G.number_of_inputs)]].insert(i);
        }
    }
    int label = 0;
    for(int V=0; V< G.total_number_of_terminals + (3+G.number_of_inputs); V++){
        if (Hspice_Node_Indecies_For_Terminals[V] == -1){
            DFS(V, label);
            label++;
        } 
    }

}
string Individual::GetMeHspiceNodeName(int C , Generation G){
    if(C == 0){
        return "vss";
    }
    else if(C == 1){
        return "vdd";
    }
    else if(C == 2){
        return "out";
    }
    else if (C < 3 + G.number_of_inputs){
        char input_signal_name;
        input_signal_name = 97 + (C-3);
        return string(1,input_signal_name);
    }
    else if(3 + G.number_of_inputs <= C){
        return string("Node") + to_string(C-(3 + G.number_of_inputs));
    }
}
void Individual:: Mutate(Generation G){
    //CHANGING TOPOLOGY
    bool valid_target = false;
    int Targeted_terminal;
    int Index_of_terminal_losing_its_connection;
    while (valid_target == false){
        Targeted_terminal = rand()%G.total_number_of_terminals;
        Index_of_terminal_losing_its_connection = Connection_Mappings[Targeted_terminal];
        if (Index_of_terminal_losing_its_connection < 3 + G.number_of_inputs){ //we want to remove connection to one of the special signals.
            int special_signal_count = 0;
            for(int i=0; i < G.total_number_of_terminals; i++){// we have to make sure at least one other terminal is connected to them after mutation.
                if ( Connection_Mappings[i] == Index_of_terminal_losing_its_connection  &&  i!=Targeted_terminal){
                    special_signal_count++;
                    if (special_signal_count > 1){
                    valid_target = true;
                    break;
                    }
                }   
            }
        }
        else{
            valid_target = true;
        }
    }
    
    int new_terminal_to_connect_to = (rand()%(G.total_number_of_terminals+3+G.number_of_inputs));
    while(new_terminal_to_connect_to == Targeted_terminal+3+G.number_of_inputs || new_terminal_to_connect_to == Index_of_terminal_losing_its_connection ){ // make sure we dont connect it to itself or to the same terminal it was connected to
        new_terminal_to_connect_to = (rand()%(G.total_number_of_terminals+3+G.number_of_inputs));
    }
    Connection_Mappings[Targeted_terminal] = new_terminal_to_connect_to;
    int target_device_for_feature_mutation = rand()%Number_of_Components;
    components[target_device_for_feature_mutation]->Mutate_Feature();
    
}
void Individual:: Simulation_Code(Generation G){
    this->Define_Hspice_Nodes_Based_On_Connection_Mapping(G);
    std::vector<string> Spice_Header;
    std::vector<string> Spice_Measurements;
    Spice_Header.push_back(string("\n.lib "  + G.LibFileAddress +  " typical"));
    Spice_Header.push_back(".GLOBAL VSS VDD");
    stringstream vdd_s_string;
    vdd_s_string << fixed << setprecision(1) << G.VDD;
    Spice_Header.push_back(string(".param vp ="+ vdd_s_string.str()));
    Spice_Header.push_back("vvdd VDD 0 vp\nvvss VSS 0 0\n.option post nomod\n.option posttop\n.param c_load = 1f");

    switch(G.number_of_inputs){  
        case 2:
        Spice_Measurements.push_back("va a 0  pwl  0 0 ,4.9n 0, 5n vp , 9.9n vp, 10n 0, 14.9n 0,15n vp,20n vp");
        Spice_Measurements.push_back("vb b 0  pwl  0 0 ,9.9n 0, 10n vp ,  20n vp");
        break;
        case 3:
        Spice_Measurements.push_back("va a 0  pwl  0 0 ,4.9n 0, 5n vp , 9.9n vp, 10n 0, 14.9n 0,15n vp,20n vp");
        Spice_Measurements.push_back("vb b 0  pwl  0 0 ,9.9n 0, 10n vp ,  20n vp");
        Spice_Measurements.push_back("vc c 0 vp");
        break;
    }

    stringstream T_Low,T_High,T_Step;
    switch(G.activation_signal){
        case 3:
            T_Low << fixed << setprecision(1) << G.Trigger_Low;
            T_High << fixed << setprecision(1) << G.Trigger_High;
            T_Step << fixed << setprecision(1) << (G.Trigger_High-G.Trigger_Low)/(G.total_control_signal_variations-1);
            Spice_Measurements.push_back(".tran 0.1n 20n sweep vp "+T_Low.str()+" "+T_High.str()+" "+T_Step.str()); 
        break;
        case 2:
            T_Low << fixed << setprecision(1) << G.Trigger_Low;
            T_High << fixed << setprecision(1) << G.Trigger_High;
            Spice_Measurements.push_back(".tran 0.1n 20n sweep vc "+T_Low.str()+" "+T_High.str()+" "+T_High.str());
        break;

    }
    
    Spice_Measurements.push_back(".measure tran charge1 avg  v(out) from = 1n   to = 4.8n"); //output voltage when input equals"00"
    Spice_Measurements.push_back(".measure tran charge2 avg  v(out) from = 6n   to = 9.8n");  //output voltage when input equals"01"
    Spice_Measurements.push_back(".measure tran charge3 avg  v(out) from = 11n  to = 14.8n"); //output voltage when input equals"10"
    Spice_Measurements.push_back(".measure tran charge4 avg  v(out) from = 16n  to = 19.8n");  //output voltage when input equals"11"
    Spice_Measurements.push_back(".end");

    ofstream SPFILE("./Gen"+to_string(G.current_generation)+"/G"+to_string(G.current_generation)+"_Ind"+to_string(Individual_ID)+".sp");
    
    for (int i=0;i < Spice_Header.size(); i++){
        SPFILE << Spice_Header[i] << "\n";
    }
    /// ADD connection SPICE codes
    int current_index = 3 + G.number_of_inputs ;
    for(int Dev=0; Dev < Number_of_Components; Dev++){
        
        if(components[Dev]->Device_Type().compare("Transistor") == 0){
            
            string Source,Gate,Drain;
            Drain = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            Gate = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            Source = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            SPFILE << components[Dev]->Simulation_Code(Source,Gate,Drain);

        }
        else if(components[Dev]->Device_Type().compare("Capacitor") == 0){
            string P,N;
            P = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            N = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            SPFILE << components[Dev]->Simulation_Code(P,N);

        }
        else if(components[Dev]->Device_Type().compare("Resistor") == 0) {
            string P,N;
            P = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            N = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            SPFILE << components[Dev]->Simulation_Code(P,N);

        }
    }
    for(int i = 0 ; i < Spice_Measurements.size(); i++){
        SPFILE << Spice_Measurements[i] << "\n";
    }
}

void Individual::Simulation_Code(Generation G,int Gen){ // called by Resurrection function

    this->Define_Hspice_Nodes_Based_On_Connection_Mapping(G);
    std::vector<string> Spice_Header;
    std::vector<string> Spice_Measurements;
    Spice_Header.push_back(string("\n.lib "  + G.LibFileAddress +  " typical"));
    Spice_Header.push_back(".GLOBAL VSS VDD");
    stringstream vdd_s_string;
    vdd_s_string << fixed << setprecision(1) << G.VDD;
    Spice_Header.push_back(string(".param vp ="+ vdd_s_string.str()));
    Spice_Header.push_back("vvdd VDD 0 vp\nvvss VSS 0 0\n.option post nomod\n.option posttop\n.param c_load = 1f");

    switch(G.number_of_inputs){  
        case 2:
        Spice_Measurements.push_back("va a 0  pwl  0 0 ,4.9n 0, 5n vp , 9.9n vp, 10n 0, 14.9n 0,15n vp,20n vp");
        Spice_Measurements.push_back("vb b 0  pwl  0 0 ,9.9n 0, 10n vp ,  20n vp");
        break;
        case 3:
        Spice_Measurements.push_back("va a 0  pwl  0 0 ,4.9n 0, 5n vp , 9.9n vp, 10n 0, 14.9n 0,15n vp,20n vp");
        Spice_Measurements.push_back("vb b 0  pwl  0 0 ,9.9n 0, 10n vp ,  20n vp");
        Spice_Measurements.push_back("vc c 0 vp");
        break;
    }

    stringstream T_Low,T_High,T_Step;
    switch(G.activation_signal){
        case 3:
            T_Low << fixed << setprecision(1) << G.Trigger_Low;
            T_High << fixed << setprecision(1) << G.Trigger_High;
            T_Step << fixed << setprecision(1) << (G.Trigger_High-G.Trigger_Low)/(G.total_control_signal_variations-1);
            Spice_Measurements.push_back(".tran 0.1n 20n sweep vp "+T_Low.str()+" "+T_High.str()+" "+T_Step.str()); 
        break;
        case 2:
            T_Low << fixed << setprecision(1) << G.Trigger_Low;
            T_High << fixed << setprecision(1) << G.Trigger_High;
            Spice_Measurements.push_back(".tran 0.1n 20n sweep vc "+T_Low.str()+" "+T_High.str()+" "+T_High.str());
        break;

    }
    
    Spice_Measurements.push_back(".measure tran charge1 avg  v(out) from = 1n   to = 4.8n"); //output voltage when input equals"00"
    Spice_Measurements.push_back(".measure tran charge2 avg  v(out) from = 6n   to = 9.8n");  //output voltage when input equals"01"
    Spice_Measurements.push_back(".measure tran charge3 avg  v(out) from = 11n  to = 14.8n"); //output voltage when input equals"10"
    Spice_Measurements.push_back(".measure tran charge4 avg  v(out) from = 16n  to = 19.8n");  //output voltage when input equals"11"
    Spice_Measurements.push_back(".end");

    system("mkdir Resurrected");
    ofstream SPFILE("./Resurrected/G"+to_string(Gen)+"_Ind"+to_string(Individual_ID)+".sp");
    
    for (int i=0;i < Spice_Header.size(); i++){
        SPFILE << Spice_Header[i] << "\n";
    }
    /// ADD connection SPICE codes
    int current_index = 3 + G.number_of_inputs ;
    for(int Dev=0; Dev < Number_of_Components; Dev++){
        
        if(components[Dev]->Device_Type().compare("Transistor") == 0){
            
            string Source,Gate,Drain;
            Drain = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            Gate = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            Source = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            SPFILE << components[Dev]->Simulation_Code(Source,Gate,Drain);

        }
        else if(components[Dev]->Device_Type().compare("Capacitor") == 0){
            string P,N;
            P = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            N = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            SPFILE << components[Dev]->Simulation_Code(P,N);

        }
        else if(components[Dev]->Device_Type().compare("Resistor") == 0) {
            string P,N;
            P = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            N = GetMeHspiceNodeName(Hspice_Node_Indecies_For_Terminals[current_index],G);
            current_index++;
            SPFILE << components[Dev]->Simulation_Code(P,N);

        }
    }
    for(int i = 0 ; i < Spice_Measurements.size(); i++){
        SPFILE << Spice_Measurements[i] << "\n";
    }
}

void Individual::Set_ID(int id){
    Individual_ID = id;
}

void Individual::Store_Genes(Generation G){
    // writing genes to a text file
    ofstream ConnectionFile;
    ofstream FeatureFile;
    string ConnectionFileName = "Connection_";
    string FeatureFileName = "Feature_";
    ConnectionFileName = ConnectionFileName + std::to_string(G.current_generation)+"g.txt";
    FeatureFileName = FeatureFileName + std::to_string(G.current_generation)+"g.txt";
    ConnectionFile.open(ConnectionFileName, std::ofstream::out | std::ofstream::app );
    FeatureFile.open(FeatureFileName, std::ofstream::out | std::ofstream::app);
    
    for( int i = 0 ; i < G.total_number_of_terminals; i++ ){
        ConnectionFile << Connection_Mappings[i] << endl;
    }
    ConnectionFile <<"\n";

    for (int i = 0 ; i < Number_of_Components ; i++){
        components[i]->Store_Feature_Genes(FeatureFile);
    }
    FeatureFile << "\n";
}

void Individual::DFS(int v, int label){
    Hspice_Node_Indecies_For_Terminals[v] = label;
    set<int>::iterator it;
    for (it = Terminals_Adj_list[v].begin(); it != Terminals_Adj_list[v].end(); ++it)
    {
        if (Hspice_Node_Indecies_For_Terminals[*it] == -1){
            DFS(*it, label);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Generation:: Generation(){
    Debug = 1;
    cout <<"////////////////////////////////////////////////////////////////" <<endl;
    ifstream CONF("Config.txt");
    string string_from_conf;
    CONF >> string_from_conf >> VDD;
    cout << "VDD :"<< VDD << endl;
    CONF >> string_from_conf >> high_thresh;
    cout << "high Threshold :"<< high_thresh << endl;
    CONF >> string_from_conf >> low_thresh;
    cout << "Low Threshold :"<< low_thresh << endl;
    CONF >> string_from_conf >> SilentMode;
    cout << "Silent Mode : "<< SilentMode << endl;
    CONF >> string_from_conf >> string_from_conf;
    if (string_from_conf.compare("Temperature") == 0){
        cout << "Activation Signal:"<< string_from_conf;
        activation_signal = 1;
        CONF >> string_from_conf >> Trigger_Low;
        CONF >> string_from_conf >> Trigger_High;
        cout << " with Trigger Low  "<< Trigger_Low <<"  and Trigger High of "<< Trigger_High << endl;
    }
    else if (string_from_conf.compare("External") == 0){
        cout << "Activation Signal:"<< string_from_conf << endl;
        activation_signal = 2;
        CONF >> string_from_conf >> Trigger_Low;
        CONF >> string_from_conf >> Trigger_High;
        cout << " with Trigger Low  "<< Trigger_Low <<"  and Trigger High of "<< Trigger_High << endl;
    }
    else if(string_from_conf.compare("VDD") == 0){
        cout << "Activation Signal:"<< string_from_conf;
        activation_signal = 3;
        CONF >> string_from_conf >> Trigger_Low;
        CONF >> string_from_conf >> Trigger_High;
        cout << " with Trigger Low  "<< Trigger_Low <<"  and Trigger High of  "<< Trigger_High << endl;
    }
    else{
        cout << "Activation Signal can be either Temperature, VDD, External." << endl;
        exit(0);
    }
    CONF >> string_from_conf >> total_control_signal_variations;
    cout << "Total number of variations for trigger signal is "<< total_control_signal_variations << endl;
    CONF >> string_from_conf >> population_size;
    cout << "Population size : "<< population_size << endl;
    CONF >> string_from_conf >> LibFileAddress;
    cout << "library files address : "<< LibFileAddress << endl;
    CONF >> string_from_conf >> number_of_inputs;
    cout << "number of inputs : "<< number_of_inputs << endl;
    CONF >> string_from_conf >> Number_of_Components;
    cout << "number of components : "<< Number_of_Components << endl;
    cout <<"////////////////////////////////////////////////////////////////" <<endl;

    current_generation = 0; 
    number_of_ancestors = 0; 

    total_number_of_terminals = 0;
    string component,component_model;
    int component_terminals; 
    for (int c=0;c<Number_of_Components;c++){
        CONF>> component >> component_terminals >> component_model;
        total_number_of_terminals += component_terminals;
        element E;
        E.device_type = component;
        if (component_model.compare("None") == 0){
            E.device_model = "";
        }
        else{
            E.device_model = component_model;
        }
        E.Terminal_Num = component_terminals;
        Devices_info.push_back(E);
    }

    ////////////////// Generating the initial connection file //////////////
    ofstream ConnectionFile("Connection_0g.txt");
    int * connections = new int[total_number_of_terminals];
    for (int i=0; i < total_number_of_terminals; i++){
        connections[i] = -1;
    }
    for(int individual = 0; individual < population_size; individual++){
        int random = (rand()%total_number_of_terminals);
        connections[random]  = 0;//connecting one terminal to VSS
        
        random = (rand()%total_number_of_terminals);
        while(connections[random] != -1){
            random = (rand()%total_number_of_terminals);
        }
        connections[random] = 1;//connecting one terminal to VDD

        random = (rand()%total_number_of_terminals);
        while(connections[random] != -1){
            random = (rand()%total_number_of_terminals);
        }
        connections[random] = 2;//connecting one terminal to output

        for(int input=0; input < number_of_inputs;input++){
            random = (rand()%total_number_of_terminals);
            while(connections[random] != -1){
                random = (rand()%total_number_of_terminals);
            }
            connections[random] = 3+input;//connecting one terminal to inputs
        }

        for(int i = 0; i < total_number_of_terminals; i++){
            if (connections[i] == -1){
                random = (rand()%(total_number_of_terminals+3+number_of_inputs));
                while(random == i+3+number_of_inputs){
                    random = (rand()%(total_number_of_terminals+3+number_of_inputs));
                }
                connections[i] = random;
            }
        }

        for( int i = 0 ; i < total_number_of_terminals; i++ ){
            ConnectionFile << connections[i] << "\n";
            connections[i] = -1;
        }
        ConnectionFile <<"\n";
    }
    delete[] connections;
    ///////// generating the initial feature files ///////////
    ofstream FeatureFile("Feature_0g.txt");
    for(int individual = 0; individual < population_size; individual++){
        for(int Dev=0; Dev < Number_of_Components; Dev++){
            if (Devices_info[Dev].device_type.compare("pmos") == 0 || Devices_info[Dev].device_type.compare("nmos") == 0 ){
                FeatureFile << (rand() % 10 + 1)*0.40 << " " << (rand() % 10 + 1)*0.40 << "\n"; 
            }
            else if(Devices_info[Dev].device_type.compare("Capacitor")){
                FeatureFile << (rand() % 10 + 1)*0.10 << "\n"; // Ask Bill
            }
            else{
                FeatureFile << (rand() % 10 + 1)*0.20 << "\n"; // Ask Bill
            }
        }
        FeatureFile <<"\n";
    }   
}
Generation:: ~Generation(){}

void Generation:: Test(){
    number_of_ancestors = population_size;
    current_generation++;
    string mkdir_Command = "mkdir Gen"+to_string(current_generation); 
    system(mkdir_Command.c_str());
    ifstream FeatureFile("Feature_0g.txt");
    ifstream ConnectionFile("Connection_0g.txt");
    Individual Ind(*this,0,FeatureFile,ConnectionFile);
    Ind.Mutate(*this);
    Ind.Store_Genes(*this);
    Ind.Simulation_Code(*this);
    Ind.Undo_Mutation();
    Ind.Set_ID(1);
    Ind.Simulation_Code(*this);
    Ind.Store_Genes(*this);


}
void Generation:: Evolve(){
    
    if (current_generation == 0 ){
        number_of_ancestors = population_size;
        current_generation++;
        string mkdir_Command = "mkdir Gen"+to_string(current_generation); 
        system(mkdir_Command.c_str());

        ifstream FeatureFile("Feature_0g.txt");
        ifstream ConnectionFile("Connection_0g.txt");
        for (int i = 0;i< population_size;i++){
            Individual Ind(*this,i,FeatureFile,ConnectionFile);
            Ind.Mutate(*this);
            Ind.Store_Genes(*this);
            Ind.Simulation_Code(*this);
        } 
    }
    else{
        ifstream ConnectionFile, FeatureFile , SurvivalFile;
        string SurvivalFileName = "Survived_"+ std::to_string(current_generation)+"g.txt";
        string ConnectionFileName = "Connection_"+ std::to_string(current_generation)+"g.txt";
        string FeatureFileName = "Feature_" + std::to_string(current_generation)+"g.txt";

        current_generation++;
        string mkdir_Command = "mkdir Gen"+to_string(current_generation); 
        system(mkdir_Command.c_str());

        ConnectionFile.open(ConnectionFileName, std::ifstream::in);
        FeatureFile.open(FeatureFileName, std::ifstream::in );
        SurvivalFile.open(SurvivalFileName, std::ifstream::in);

        int subpopulation = (population_size/number_of_ancestors);
        int survived_ind;
        int current_line_on_connection_file = 0, current_line_on_feature_file = 0;

        string nothing_important_string;
        int nothing_important_int;
        for( int ancestor = 0 ; ancestor < number_of_ancestors; ancestor++){
            SurvivalFile >> survived_ind >> nothing_important_string ; 
            for (int j = 0; j < survived_ind*((Number_of_Components+1)) - current_line_on_feature_file; j++){
                std::getline(FeatureFile,nothing_important_string);
            }
            for (int j = 0; j < survived_ind*((total_number_of_terminals+1))-current_line_on_connection_file; j++){
                std::getline(ConnectionFile,nothing_important_string);
            }            
            current_line_on_connection_file = survived_ind*((total_number_of_terminals+1)) + (total_number_of_terminals);
            current_line_on_feature_file = survived_ind*((Number_of_Components+1)) + (Number_of_Components);

            Individual Ind(*this,ancestor*subpopulation,FeatureFile,ConnectionFile);
            for (int j= 0; j < subpopulation; j++){
                Ind.Set_ID(ancestor*subpopulation+j);
                Ind.Mutate(*this);
                Ind.Store_Genes(*this);
                Ind.Simulation_Code(*this);
                Ind.Undo_Mutation();
            }
        }
    }
}
string Generation::WhatGateIsIt(std::vector<int>& Binary_Outputs){

    if (Binary_Outputs[0] == 2 || Binary_Outputs[1] == 2 || Binary_Outputs[2] == 2 || Binary_Outputs[3] == 2 )
    {
        return "unstable";
    }
    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 1 )
    {
        return "and";
    }
    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 1 )
    {
        return "or";
    }
    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 1 )
    {
        return "xor";
    }
    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 0 )
    {
        return "xnor";
    }
    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 0 )
    {
        return "nand";
    }
    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 0 )
    {
        return "nor";
    }

    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 0 )
    {
        return "low";
    }

    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 0 )
    {
        return "inv2and";
    }

    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 1 )
    {
        return "buf1";
    }

    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 0 )
    {
        return "inv1and";
    }

    if (Binary_Outputs[0] == 0 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 1 )
    {
        return "buf2";
    }

    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 0 )
    {
       return "inv2";
    }

    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 0 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 1 )
    {
        return "inv2or";
    }

    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 0 )
    {
        return "inv1";
    }

    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 0 && Binary_Outputs[3] == 1 )
    {
        return "inv1or";
    }

    if (Binary_Outputs[0] == 1 && Binary_Outputs[1] == 1 && Binary_Outputs[2] == 1 && Binary_Outputs[3] == 1 )
    {
        return "high";
    }

}
void Generation:: Evaluate_Individuals(){
    cout <<"Evaluating Individuals" << endl;
    int num_survived_individuauls = 0;
    string SurvivalFileName = "Survived_"+ std::to_string(current_generation)+"g.txt";
    ofstream SurvivalFile(SurvivalFileName);
    // Run the HSpice Simulations
    ofstream simulation("simulate.sh", std::ifstream::trunc);
    simulation << "#!/bin/bash"<<endl;
    for (int i = 0 ; i < number_of_ancestors*((int)(population_size/number_of_ancestors)); i++){
        simulation <<"hspice -i  "<< "./Gen" + to_string(current_generation)+"/G"+ to_string(current_generation)+"_Ind"+to_string(i)+".sp";
        simulation <<" -o  "<< "./Gen" + to_string(current_generation) + "/G" + to_string(current_generation)+"_Ind"+to_string(i)<<".lis"<<endl;
    }
    simulation.close();
    system("chmod +x simulate.sh");
    system("simulate.sh");
    cout <<"finished the simulations" << endl;
    // pars the hpsice simulation result of each individual
    for (int individual = 0 ; individual < number_of_ancestors*((int)(population_size/number_of_ancestors)); individual++){
        cout << "Analyzing Individual " << individual << endl;
        string Buffer;
        string nonesense;
        double read_charge_val;
        char possible_m_u_n;

        std::vector<double>Individual_Simulation_Results;
        std::vector<string> Resulted_Functionality;

        ifstream lisfile("./Gen" + to_string(current_generation)+"/G"+to_string(current_generation)+"_Ind"+to_string(individual)+".lis");
        if(lisfile.fail()) cout <<" couldnt open file " << "./Gen" + to_string(current_generation)+"/G"+to_string(current_generation)+"_Ind"+to_string(individual)+".lis" <<endl;

        while (std::getline(lisfile, Buffer)) {
        if( Buffer.find("charge") != std::string::npos){
            Buffer.replace(8,1,"  ");
            stringstream to_extract_charge_val (Buffer);
            to_extract_charge_val >> nonesense >> read_charge_val >> possible_m_u_n ;
            switch(possible_m_u_n){
            case 'm': // if the value is reported in Milli V
                    read_charge_val = read_charge_val / 1000.000 ;
                    break;
            case 'u': // if the value is reported in Micro V 
                    read_charge_val = read_charge_val / 1000000.000 ;
                    break;
            case 'n': //if the value is reported in Nano V
                    read_charge_val = read_charge_val / 1000000000.000 ;
                    break;
            case 'p':
                read_charge_val = read_charge_val / 1000000000000.000 ;
                break;
            case 'f':
                char check; // checking to see if the f character is coming from the word "from" in the .lis file or it's for femto
                to_extract_charge_val >> check;
                if (check == 'r'){
                    break;
                }
                read_charge_val = read_charge_val / 1000000000000000.000 ;
                break;
            case 'a':
                read_charge_val = read_charge_val / 1000000000000000000.000 ;
                break;
            default:
                    break;
            }
            Individual_Simulation_Results.push_back(read_charge_val);
            }
        }
        
        double current_VDD; //VDD varies if we are searching for Vdd triggered polymorphic gates
        for (int cont_sig_value = 0; cont_sig_value < total_control_signal_variations; cont_sig_value++){            
            std::vector<int> Binary_Outputs;
            
            /// Setting the VDD value for evaluating binary values of the outputs
            if (activation_signal == 1 || activation_signal == 2){ // if the trigger signal is temperature or is an external signal the VDD should be constant
                current_VDD = VDD;
            }
            else{ // if the VDD signal is in fact the trigger signal, it varies.
                current_VDD = Trigger_Low + cont_sig_value*((Trigger_High-Trigger_Low)/(total_control_signal_variations-1));
            }  

            for(int i = cont_sig_value*pow(2,number_of_inputs); i < (cont_sig_value+1)*(pow(2,number_of_inputs)); i++){ ////
                if (Individual_Simulation_Results[i] > high_thresh*current_VDD){
                    Binary_Outputs.push_back(1);
                }
                else if (Individual_Simulation_Results[i] < low_thresh*current_VDD && Individual_Simulation_Results[i] >= 0) 
                {
                    Binary_Outputs.push_back(0);   
                }
                else{
                    Binary_Outputs.push_back(2);
                }
            }
            Resulted_Functionality.push_back(this->WhatGateIsIt(Binary_Outputs));
        }
        
        for (int i= 0 ; i < total_control_signal_variations; i++){
            if(Resulted_Functionality[i].compare("nand") == 0 || Resulted_Functionality[i].compare("nor") == 0 ||
                Resulted_Functionality[i].compare("and") == 0 || Resulted_Functionality[i].compare("or") == 0 ){
                    num_survived_individuauls++;
                    SurvivalFile << individual << " [" ;
                    cout << "Individual " << individual <<" of Generation " << current_generation <<" survived with functionalities [";
                    for(int j=0; j< total_control_signal_variations; j++){
                        if ( j != total_control_signal_variations-1){
                            SurvivalFile << Resulted_Functionality[j] <<",";
                            cout << Resulted_Functionality[j] <<",";
                        }
                        else{
                            SurvivalFile << Resulted_Functionality[j] <<"]"<<endl;
                            cout << Resulted_Functionality[j] <<"]"<<endl;
                        }
                    }
                    break;
            }
        }

    }

    number_of_ancestors = num_survived_individuauls;
    if(number_of_ancestors == 0){
        cout << "##############    No Individual Survived   ############" << endl;
        string delete_command_failed_feature_file = "rm -rf Feature_" + std::to_string(current_generation)+"g.txt";
        string delete_command_failed_connection_file = "rm -rf Connection_"+ std::to_string(current_generation)+"g.txt";
        system(delete_command_failed_feature_file.c_str());
        system(delete_command_failed_connection_file.c_str());
        cout << "Deleted Feature and Connection Files of failed Generation " << current_generation << endl;
        system(("rm -rf ./Gen"+to_string(current_generation)).c_str());
        current_generation--;
    }

    if (SilentMode){
        system(("rm -rf ./Gen*"));
    }

}

void Resurrection(Generation G, int Gen, int Ind_id){
    ifstream ConnectionFile, FeatureFile;
    string ConnectionFileName = "Connection_"+ std::to_string(Gen)+"g.txt";
    string FeatureFileName = "Feature_" + std::to_string(Gen)+"g.txt";

    ConnectionFile.open(ConnectionFileName, std::ifstream::in);
    FeatureFile.open(FeatureFileName, std::ifstream::in );
    
    string nothing_important_string;

    for (int j = 0; j < Ind_id*((G.Number_of_Components+1)); j++){
        std::getline(FeatureFile,nothing_important_string);
    }
    for (int j = 0; j < Ind_id*((G.total_number_of_terminals+1)); j++){
        std::getline(ConnectionFile,nothing_important_string);
    }            
    

    Individual Ind(G,Ind_id,FeatureFile,ConnectionFile);
    Ind.Simulation_Code(G,Gen);

}