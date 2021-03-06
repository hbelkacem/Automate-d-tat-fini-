/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2014, 2015 Adrien Boussicault
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

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

 

void action_get_max_etat(const intptr_t element , void* data){
  int *max = (int*) data;
  if( element > *max) *max = element ;
  }


/* *  get_max_etat aplique avec " pour_tout_element " les etats de l'automate  */
/*  l'action_get_max_etat pour recuperer a chaque fois le nombre le plus garnd  */
/* *\/ */



int get_max_etat( const Automate* automate ){
  int max = INT_MIN;
  pour_tout_element(automate->etats, action_get_max_etat, &max);
  return max;
}

void action_get_min_etat( const intptr_t element, void* data ){
  int * min = (int*) data;
  if( *min > element ) *min = element;
}

int get_min_etat( const Automate* automate ){
  int min = INT_MAX;

  pour_tout_element( automate->etats, action_get_min_etat, &min );

  return min;
}


int comparer_cle(const Cle *a, const Cle *b) {
  if( a->origine < b->origine )
    return -1;
  if( a->origine > b->origine )
    return 1;
  if( a->lettre < b->lettre )
    return -1;
  if( a->lettre > b->lettre )
    return 1;
  return 0;
}

void print_cle( const Cle * a){
  printf( "(%d, %c)" , a->origine, (char) (a->lettre) );
}

void supprimer_cle( Cle* cle ){
  xfree( cle );
}

void initialiser_cle( Cle* cle, int origine, char lettre ){
  cle->origine = origine;
  cle->lettre = (int) lettre;
}

Cle * creer_cle( int origine, char lettre ){
  Cle * result = xmalloc( sizeof(Cle) );
  initialiser_cle( result, origine, lettre );
  return result;
}

Cle * copier_cle( const Cle* cle ){
  return creer_cle( cle->origine, cle->lettre );
}

Automate * creer_automate(){
  Automate * automate = xmalloc( sizeof(Automate) );
  automate->etats = creer_ensemble( NULL, NULL, NULL );
  automate->alphabet = creer_ensemble( NULL, NULL, NULL );
  automate->transitions = creer_table(
				      ( int(*)(const intptr_t, const intptr_t) ) comparer_cle , 
				      ( intptr_t (*)( const intptr_t ) ) copier_cle,
				      ( void(*)(intptr_t) ) supprimer_cle
				      );
  automate->initiaux = creer_ensemble( NULL, NULL, NULL );
  automate->finaux = creer_ensemble( NULL, NULL, NULL );
  automate->vide = creer_ensemble( NULL, NULL, NULL ); 
  return automate;
}

Automate * translater_automate_entier( const Automate* automate, int translation ){
  Automate * res = creer_automate();

  Ensemble_iterateur it;
  for( 
      it = premier_iterateur_ensemble( get_etats( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    ajouter_etat( res, get_element( it ) + translation );
  }

  for( 
      it = premier_iterateur_ensemble( get_initiaux( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    ajouter_etat_initial( res, get_element( it ) + translation );
  }

  for( 
      it = premier_iterateur_ensemble( get_finaux( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    ajouter_etat_final( res, get_element( it ) + translation );
  }

  // On ajoute les lettres
  for(
      it = premier_iterateur_ensemble( get_alphabet( automate ) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
      ){
    ajouter_lettre( res, (char) get_element( it ) );
  }

  Table_iterateur it1;
  Ensemble_iterateur it2;
  for(
      it1 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it1 );
      it1 = iterateur_suivant_table( it1 )
      ){
    Cle * cle = (Cle*) get_cle( it1 );
    Ensemble * fins = (Ensemble*) get_valeur( it1 );
    for(
	it2 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it2 );
	it2 = iterateur_suivant_ensemble( it2 )
	){
      int fin = get_element( it2 );
      ajouter_transition(
			 res, cle->origine + translation, cle->lettre, fin + translation
			 );
    }
  };

  return res;
}


void liberer_automate( Automate * automate ){
  assert( automate );
  liberer_ensemble( automate->vide );
  liberer_ensemble( automate->finaux );
  liberer_ensemble( automate->initiaux );
  pour_toute_valeur_table(
			  automate->transitions, ( void(*)(intptr_t) ) liberer_ensemble);
  liberer_table( automate->transitions );
  liberer_ensemble( automate->alphabet );
  liberer_ensemble( automate->etats );
  xfree(automate);
}

const Ensemble * get_etats( const Automate* automate ){
  return automate->etats;
}

const Ensemble * get_initiaux( const Automate* automate ){
  return automate->initiaux;
}

const Ensemble * get_finaux( const Automate* automate ){
  return automate->finaux;
}

const Ensemble * get_alphabet( const Automate* automate ){
  return automate->alphabet;
}

void ajouter_etat( Automate * automate, int etat ){
  ajouter_element( automate->etats, etat );
}

void ajouter_lettre( Automate * automate, char lettre ){
  ajouter_element( automate->alphabet, lettre );
}

void ajouter_transition(Automate * automate, int origine, char lettre, int fin){
  ajouter_etat( automate, origine );
  ajouter_etat( automate, fin );
  ajouter_lettre( automate, lettre );

  Cle cle;
  initialiser_cle( &cle, origine, lettre );
  Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
  Ensemble * ens;
  if( iterateur_est_vide( it ) ){
    ens = creer_ensemble( NULL, NULL, NULL );
    add_table( automate->transitions, (intptr_t) &cle, (intptr_t) ens );
  }else{
    ens = (Ensemble*) get_valeur( it );
  }
  ajouter_element( ens , fin );
}

void ajouter_etat_final(
			Automate * automate, int etat_final
			){
  ajouter_etat( automate, etat_final );
  ajouter_element( automate->finaux, etat_final );
}

void ajouter_etat_initial(
			  Automate * automate, int etat_initial
			  ){
  ajouter_etat( automate, etat_initial );
  ajouter_element( automate->initiaux, etat_initial );
}

const Ensemble * voisins( const Automate* automate, int origine, char lettre ){
  Cle cle;
  initialiser_cle( &cle, origine, lettre );
  Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
  if( ! iterateur_est_vide( it ) ){
    return (Ensemble*) get_valeur( it );
  }else{
    return automate->vide;
  }
}

Ensemble * delta1(
		  const Automate* automate, int origine, char lettre
		  ){
  Ensemble * res = creer_ensemble( NULL, NULL, NULL );
  ajouter_elements( res, voisins( automate, origine, lettre ) );
  return res; 
}

Ensemble * delta(
		 const Automate* automate, const Ensemble * etats_courants, char lettre
		 ){
  Ensemble * res = creer_ensemble( NULL, NULL, NULL );

  Ensemble_iterateur it;
  for( 
      it = premier_iterateur_ensemble( etats_courants );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    const Ensemble * fins = voisins(
				    automate, get_element( it ), lettre
				    );
    ajouter_elements( res, fins );
  }

  return res;
}

Ensemble * delta_star(
		      const Automate* automate, const Ensemble * etats_courants, const char* mot
		      ){
  int len = strlen( mot );
  int i;
  Ensemble * old = copier_ensemble( etats_courants );
  Ensemble * new = old;
  for( i=0; i<len; i++ ){
    new = delta( automate, old, *(mot+i) );
    liberer_ensemble( old );
    old = new;
  }
  return new;
}

void pour_toute_transition(
			   const Automate* automate,
			   void (* action )( int origine, char lettre, int fin, void* data ),
			   void* data
			   ){
  Table_iterateur it1;
  Ensemble_iterateur it2;
  for(
      it1 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it1 );
      it1 = iterateur_suivant_table( it1 )
      ){
    Cle * cle = (Cle*) get_cle( it1 );
    Ensemble * fins = (Ensemble*) get_valeur( it1 );
    for(
	it2 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it2 );
	it2 = iterateur_suivant_ensemble( it2 )
	){
      int fin = get_element( it2 );
      action( cle->origine, cle->lettre, fin, data );
    }
  };
}

Automate* copier_automate( const Automate* automate ){
  Automate * res = creer_automate();
  Ensemble_iterateur it1;
  // On ajoute les états de l'automate
  for(
      it1 = premier_iterateur_ensemble( get_etats( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat( res, get_element( it1 ) );
  }
  // On ajoute les états initiaux
  for(
      it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat_initial( res, get_element( it1 ) );
  }
  // On ajoute les états finaux
  for(
      it1 = premier_iterateur_ensemble( get_finaux( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat_final( res, get_element( it1 ) );
  }
  // On ajoute les lettres
  for(
      it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_lettre( res, (char) get_element( it1 ) );
  }
  // On ajoute les transitions
  Table_iterateur it2;
  for(
      it2 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_table( it2 )
      ){
    Cle * cle = (Cle*) get_cle( it2 );
    Ensemble * fins = (Ensemble*) get_valeur( it2 );
    for(
	it1 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it1 );
	it1 = iterateur_suivant_ensemble( it1 )
	){
      int fin = get_element( it1 );
      ajouter_transition( res, cle->origine, cle->lettre, fin );
    }
  }
  return res;
}

Automate * translater_automate(
			       const Automate * automate, const Automate * automate_a_eviter
			       ){
  if(
     taille_ensemble( get_etats(automate) ) == 0 ||
     taille_ensemble( get_etats(automate_a_eviter) ) == 0
     ){
    return copier_automate( automate );
  }
	
  int translation = 
    get_max_etat( automate_a_eviter ) - get_min_etat( automate ) + 1; 

  return translater_automate_entier( automate, translation );
	
}

int est_une_transition_de_l_automate(
				     const Automate* automate,
				     int origine, char lettre, int fin
				     ){
  return est_dans_l_ensemble( voisins( automate, origine, lettre ), fin );
}

int est_un_etat_de_l_automate( const Automate* automate, int etat ){
  return est_dans_l_ensemble( get_etats( automate ), etat );
}

int est_un_etat_initial_de_l_automate( const Automate* automate, int etat ){
  return est_dans_l_ensemble( get_initiaux( automate ), etat );
}

int est_un_etat_final_de_l_automate( const Automate* automate, int etat ){
  return est_dans_l_ensemble( get_finaux( automate ), etat );
}

int est_une_lettre_de_l_automate( const Automate* automate, char lettre ){
  return est_dans_l_ensemble( get_alphabet( automate ), lettre );
}

void print_ensemble_2( const intptr_t ens ){
  print_ensemble( (Ensemble*) ens, NULL );
}

void print_lettre( intptr_t c ){
  printf("%c", (char) c );
}

void print_automate( const Automate * automate ){
  printf("- Etats : ");
  print_ensemble( get_etats( automate ), NULL );
  printf("\n- Initiaux : ");
  print_ensemble( get_initiaux( automate ), NULL );
  printf("\n- Finaux : ");
  print_ensemble( get_finaux( automate ), NULL );
  printf("\n- Alphabet : ");
  print_ensemble( get_alphabet( automate ), print_lettre );
  printf("\n- Transitions : ");
  print_table( 
	      automate->transitions,
	      ( void (*)( const intptr_t ) ) print_cle, 
	      ( void (*)( const intptr_t ) ) print_ensemble_2,
	      ""
	       );
  printf("\n");
}

int le_mot_est_reconnu( const Automate* automate, const char* mot ){
  Ensemble * arrivee = delta_star( automate, get_initiaux(automate) , mot ); 
	
  int result = 0;

  Ensemble_iterateur it;
  for(
      it = premier_iterateur_ensemble( arrivee );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
      ){
    if( est_un_etat_final_de_l_automate( automate, get_element(it) ) ){
      result = 1;
      break;
    }
  }
  liberer_ensemble( arrivee );
  return result;
}

Automate * mot_to_automate( const char * mot ){
  Automate * automate = creer_automate();
  int i = 0;
  int size = strlen( mot );
  for( i=0; i < size; i++ ){
    ajouter_transition( automate, i, mot[i], i+1 );
  }
  ajouter_etat_initial( automate, 0 );
  ajouter_etat_final( automate, size );
  return automate;
}




Automate * creer_union_des_automates(	const Automate * automate_1, const Automate * automate_2){
  /* /\*On creé un automate qui recoit l'union des ensembles des  */
  /* automates 1 et 2 de chaque un de (finaux , initiaux , alphabet , etats)  *\/   */
 
  /*  //on cree un copie de l'automate 1 pour que on puisse ajouter les transitions */

  Automate * res = creer_automate(); 
  Automate * copie_de_automate_1 = translater_automate( copier_automate( automate_1) , automate_2  );
  res->etats = creer_union_ensemble(automate_1->etats , automate_2->etats);
  res->alphabet = creer_union_ensemble(automate_1->alphabet , automate_2->alphabet);
  res->initiaux = creer_union_ensemble(automate_1->initiaux , automate_2->initiaux);
  res->finaux = creer_union_ensemble(automate_1->finaux , automate_2->finaux);
  res->vide = creer_union_ensemble(automate_1->vide , automate_2->vide);
  /* // On ajoute les transitions de l'automate 2  */
  /*      a la copie de l'autoamte 1 */ 
  Table_iterateur it2 ; 
  Ensemble_iterateur it1;
        
  for(
      it2 = premier_iterateur_table( automate_2->transitions );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_table( it2 )
      ){
    Cle * cle = (Cle*) get_cle( it2 );
    Ensemble * fins = (Ensemble*) get_valeur( it2 );
    for(
	it1 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it1 );
	it1 = iterateur_suivant_ensemble( it1 )
	){
      int fin = get_element( it1 );
      ajouter_transition( copie_de_automate_1, cle->origine, cle->lettre, fin );
    }
  }
  res->transitions = copie_de_automate_1->transitions ;
  xfree(copie_de_automate_1);
  return res ; 
	
}

Ensemble* etats_accessibles( const Automate * automate, int etat ){
  //	A_FAIRE_RETURN( NULL ); 
  Ensemble * res = creer_ensemble( NULL, NULL, NULL );

  Ensemble_iterateur it; 
  Ensemble_iterateur it2;
 
  for(
      it2 = premier_iterateur_ensemble(get_alphabet(automate) );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_ensemble( it2 )
      ){
    res = creer_union_ensemble(res , delta1(automate , etat  ,get_element(it2) ) );		
  }

  for(
      it2 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_table( it2 )
      ){
    Cle * cle = (Cle*) get_cle( it2 );
    Ensemble * fins = (Ensemble*) get_valeur( it2 );

    for(
	it = premier_iterateur_ensemble(res);
	!iterateur_est_vide(it);
	it = iterateur_suivant_ensemble(it)                
	){
      if(get_element(it)== cle->origine)
	res = creer_union_ensemble( res , fins);		
    }        
  }
  ajouter_element(res,etat);   
  return res ; 
}


Ensemble* accessibles( const Automate * automate ){
  //	A_FAIRE_RETURN( NULL ); 8
  Ensemble * res = creer_ensemble( NULL, NULL, NULL );
  Ensemble_iterateur it;
  
  for( 
      it = premier_iterateur_ensemble(  get_initiaux(automate) );
      ! iterateur_ensemble_est_vide( it );
      it = iterateur_suivant_ensemble( it )
       ){
    res = creer_union_ensemble(res,etats_accessibles(automate, get_element(it) ));
  }

  
  return res;


}



Automate *automate_accessible( const Automate * automate ){
  Automate * res = creer_automate();
  Ensemble_iterateur it1;

  Ensemble * access = accessibles( automate );

  // On ajoute les états de l'automate
  for(
      it1 = premier_iterateur_ensemble( access );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat( res, get_element( it1 ) );
  }
  // On ajoute les états initiaux
  for(
      it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_etat_initial( res, get_element( it1 ) );
  }
  // On ajoute les états finaux
  for(
      it1 = premier_iterateur_ensemble( access );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    int etat = get_element( it1 );
    if( est_un_etat_final_de_l_automate( automate, etat ) ){
      ajouter_etat_final( res, etat );
    }
  }
  // On ajoute les lettres
  for(
      it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
      ! iterateur_ensemble_est_vide( it1 );
      it1 = iterateur_suivant_ensemble( it1 )
      ){
    ajouter_lettre( res, (char) get_element( it1 ) );
  }
  // On ajoute les transitions
  Table_iterateur it2;
  for(
      it2 = premier_iterateur_table( automate->transitions );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_table( it2 )
      ){
    Cle * cle = (Cle*) get_cle( it2 );
    int origine = cle->origine; 
    char lettre = cle->lettre;
    if( est_dans_l_ensemble( access, origine ) ){ 
      Ensemble * fins = (Ensemble*) get_valeur( it2 );
      for(
	  it1 = premier_iterateur_ensemble( fins );
	  ! iterateur_ensemble_est_vide( it1 );
	  it1 = iterateur_suivant_ensemble( it1 )
	  ){
	int fin = get_element( it1 );
	ajouter_transition( res, origine, lettre, fin );
      }
    }
  };
  liberer_ensemble( access );
  return res;

}


/* /\*miroir  fait  inverser les ensembles  */
/*    *des finaux devient initiaux et  */
/*    *les initiaux devient finaux   */
/*    *inversses tout les transitions  */
/*        - la fin devient l'origine et */
/*        - l'origine devient  fin   */
/*   *\/ */

Automate *miroir( const Automate * automate){
  
  Automate * copie_de_automate = copier_automate( automate);
  Automate * automate_2 = creer_automate();
  Ensemble * ens ;
  // permuter   les initiaux et les finaux 
  ens = copie_de_automate->finaux;
  copie_de_automate->finaux = copie_de_automate->initiaux ; 
  copie_de_automate->initiaux = ens ;
// On ajouter les transitions
  Table_iterateur it2 ; 
  Ensemble_iterateur it1;
        
  for(
      it2 = premier_iterateur_table( copie_de_automate->transitions );
      ! iterateur_est_vide( it2 );
      it2 = iterateur_suivant_table( it2 )
      ){
    Cle * cle = (Cle*) get_cle( it2 );
    Ensemble * fins = (Ensemble*) get_valeur( it2 );
    for(
	it1 = premier_iterateur_ensemble( fins );
	! iterateur_ensemble_est_vide( it1 );
	it1 = iterateur_suivant_ensemble( it1 )
	){
      int fin = get_element( it1 );
      ajouter_transition(automate_2,  fin, cle->lettre, cle->origine );
    }
  }
  copie_de_automate->transitions = automate_2->transitions ; 
  xfree(automate_2);
  return copie_de_automate ; 
     

}



int nommer_etat(int i,int j){
  return (i << 16) + j;
}


Automate * creer_automate_du_melange(

				     const Automate* automate_1,  const Automate* automate_2
				     ){
  Automate * automate_melange = creer_automate();
 
  Ensemble * initiaux_melange, * finaux_melange;
  initiaux_melange = creer_ensemble(NULL,NULL,NULL);
  finaux_melange = creer_ensemble(NULL,NULL,NULL);
  Ensemble * etats_accessibles;
	
  Ensemble_iterateur it1, it2, it_alph1, it_alph2, it_access;

  for(
      it1 = premier_iterateur_ensemble(get_etats(automate_1));
      !iterateur_ensemble_est_vide(it1);
      it1 = iterateur_suivant_ensemble(it1)){
    for(
	it2 = premier_iterateur_ensemble(get_etats(automate_2));
	!iterateur_ensemble_est_vide(it2);
	it2 = iterateur_suivant_ensemble(it2)){
      int i = get_element(it1);
      int j = get_element(it2);
      for(
	  it_alph1 = premier_iterateur_ensemble(get_alphabet(automate_1));
	  !iterateur_ensemble_est_vide(it_alph1);
	  it_alph1 = iterateur_suivant_ensemble(it_alph1)){
	etats_accessibles = delta1(automate_1, i,get_element(it_alph1));
	for(
	    it_access = premier_iterateur_ensemble(etats_accessibles);
	    !iterateur_ensemble_est_vide(it_access);
	    it_access = iterateur_suivant_ensemble(it_access)){
	    ajouter_transition(automate_melange,nommer_etat(i,j), get_element(it_alph1) , nommer_etat(get_element(it_access), j));
	}
	liberer_ensemble(etats_accessibles);				   
      }
      
      for(
	  it_alph2 = premier_iterateur_ensemble(get_alphabet(automate_2));
	  !iterateur_ensemble_est_vide(it_alph2);
	  it_alph2 = iterateur_suivant_ensemble(it_alph2)){
	etats_accessibles = delta1(automate_2, j,get_element(it_alph2));
	for(
	    it_access = premier_iterateur_ensemble(etats_accessibles);
	    !iterateur_ensemble_est_vide(it_access);
	    it_access = iterateur_suivant_ensemble(it_access)){
	  ajouter_transition(automate_melange,nommer_etat(i,j), get_element(it_alph2), nommer_etat(i, get_element(it_access)));
	}
	liberer_ensemble(etats_accessibles);				   
      }
      
      if(est_dans_l_ensemble( get_initiaux(automate_1) ,i) && est_dans_l_ensemble(get_initiaux(automate_2) ,j))
	ajouter_element(initiaux_melange, nommer_etat(i,j));
      if(est_dans_l_ensemble(get_finaux(automate_1),i) && est_dans_l_ensemble(get_finaux(automate_2) ,j))
	ajouter_element(finaux_melange, nommer_etat(i,j));
    }

  }
  liberer_ensemble(automate_melange->initiaux);
  liberer_ensemble(automate_melange->finaux);
  automate_melange->initiaux = initiaux_melange;
  automate_melange->finaux = finaux_melange;
  return automate_melange;
}
