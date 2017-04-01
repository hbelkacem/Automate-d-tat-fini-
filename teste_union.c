

#include "automate.h"
#include "table.h"
#include "ensemble.h"
#include "outils.h"
#include "fifo.h"
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 
#include <assert.h>
#include <math.h>


Automate * teste_union(Automate * aut1 , Automate * aut2){
   Automate* automate = creer_automate();
   automate = copier_automate(creer_union_des_automates(aut1 , aut2));
return automate;
}

int main(){
Automate * a = creer_automate();
Automate * b = creer_automate();
print_automate(teste_union(a , b));
return 0;
}
