//
// Created by Mario Marchand on 16-12-30.
//

#include "ReseauGTFS.h"
#include <sys/time.h>

using namespace std;

//! \brief ajout des arcs dus aux voyages
//! \brief insère les arrêts (associés aux sommets) dans m_arretDuSommet et m_sommetDeArret
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsVoyages(const DonneesGTFS & p_gtfs)
{

    // m_arret du sommet à la position k = i*j +j
    unsigned int i = 0;
    unsigned int j = 0;
    for (auto voy : p_gtfs.getVoyages()){
        for (auto arr : voy.second.getArrets()){

            unsigned int temps = arr->getHeureArrivee() - arr->getHeureDepart();
            m_leGraphe.ajouterArc(i, j, temps);
            m_arretDuSommet.push_back(arr);
            m_sommetDeArret.insert({arr, i*j +j});
            j++;
        }
        j = 0;
        i++;
    }
	//écrire votre code ici
}



//! /! \brief ajouts des arcs dus aux transferts entre stations
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsTransferts(const DonneesGTFS & p_gtfs)
{

    //comme le nom dit lui cest les transferts et les stations
    //cette fonction là execute 492 fois

    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();

    unsigned int counter = 0;
    for (auto trans : p_gtfs.getTransferts()){

        unsigned int from = get<0>(trans);
        unsigned int to = get<1>(trans);
        unsigned int transfer_time =  get<1>(trans);
        Station from_stat = toutes_stations[from];
        Station to_stat = toutes_stations[to];
        for (auto from_heure_et_arret : from_stat.getArrets()){
            Arret::Ptr from_arret = from_heure_et_arret.second;
            cout << "yen a tu vrm un?" << endl;
            cout << m_sommetDeArret[from_arret] << endl;
            for (auto to_heure_et_arret: to_stat.getArrets()){
                Arret::Ptr to_arret = from_heure_et_arret.second;
                
        }
    }

    }

}

//! \brief ajouts des arcs d'une station à elle-même pour les stations qui ne sont pas dans DonneesGTFS::m_stationsDeTransfert
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttente(const DonneesGTFS & p_gtfs)
{
	//écrire votre code ici
}


//! \brief ajoute des arcs au réseau GTFS à partir des données GTFS
//! \brief Il s'agit des arcs allant du point origine vers une station si celle-ci est accessible à pieds et des arcs allant d'une station vers le point destination
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \param[in] p_pointOrigine: les coordonnées GPS du point origine
//! \param[in] p_pointDestination: les coordonnées GPS du point destination
//! \throws logic_error si une incohérence est détecté lors de la construction du graphe
//! \post constuit un réseau GTFS représenté par un graphe orienté pondéré avec poids non négatifs
//! \post assigne la variable m_origine_dest_ajoute à true (car les points orignine et destination font parti du graphe)
//! \post insère dans m_sommetsVersDestination les numéros de sommets connctés au point destination
void ReseauGTFS::ajouterArcsOrigineDestination(const DonneesGTFS &p_gtfs, const Coordonnees &p_pointOrigine,
                                               const Coordonnees &p_pointDestination)
{
	//écrire votre code ici
}

//! \brief Remet ReseauGTFS dans l'était qu'il était avant l'exécution de ReseauGTFS::ajouterArcsOrigineDestination()
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \throws logic_error si une incohérence est détecté lors de la modification du graphe
//! \post Enlève de ReaseauGTFS tous les arcs allant du point source vers un arrêt de station et ceux allant d'un arrêt de station vers la destination
//! \post assigne la variable m_origine_dest_ajoute à false (les points orignine et destination sont enlevés du graphe)
//! \post enlève les données de m_sommetsVersDestination
void ReseauGTFS::enleverArcsOrigineDestination()
{
	//écrire votre code ici
}


