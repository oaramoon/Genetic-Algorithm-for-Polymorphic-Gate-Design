#include "Population.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include<string>
#include <time.h>
using namespace std;

int main(){
	srand(time(NULL));
	Generation Go = Generation();
	for(int Gen = 0 ; Gen < 4 ; Gen++){
		Go.Evolve();
		Go.Evaluate_Individuals();
	}
	/*int Gen, Ind_id;
    cout <<"Please Enter the Generation of the individual" << endl;
    cin >> Gen;
    cout <<"Please Enter the Id of the individual" << endl;
    cin >> Ind_id;
    Resurrection(Go,Gen,Ind_id);*/
}