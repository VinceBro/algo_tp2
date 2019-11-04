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
    unsigned int counter = 0;
    for (auto voy : p_gtfs.getVoyages()){
        std::set<Arret::Ptr, Voyage::compArret> arrets = voy.second.getArrets();
        std::set<Arret::Ptr, Voyage::compArret>::iterator it;
        for (it = arrets.begin(); it != arrets.end(); ++it){

            if (it != --arrets.end()){
                unsigned int temps =(*(next(it)))->getHeureArrivee() - (*it)->getHeureArrivee();
                m_leGraphe.ajouterArc(i, j, temps);
                m_arretDuSommet.push_back(*it);
                m_sommetDeArret.insert({*it, counter});
                counter++;
                j++;
            }
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
    auto tous_voyages = p_gtfs.getVoyages();
    auto toutes_lignes = p_gtfs.getLignes();

    unsigned int counter = 0;
    for (auto trans : p_gtfs.getTransferts()){

        unsigned int from = get<0>(trans);
        unsigned int to = get<1>(trans);
        unsigned int transfer_time =  get<2>(trans);

        Station from_stat = toutes_stations.at(from);
        Station to_stat = toutes_stations.at(to);

        unsigned int i = 0;
        for (auto from_heure_et_arret : from_stat.getArrets()){
            Arret::Ptr from_arret = from_heure_et_arret.second;

            auto from_ligne_id = toutes_lignes.at(tous_voyages.at(from_arret->getVoyageId()).getLigne()).getId();

            unsigned int lowest;
            Arret::Ptr lowest_arret;
            unsigned int j = 0;
            for (auto to_heure_et_arret: to_stat.getArrets()){
                Arret::Ptr  to_arret = to_heure_et_arret.second;
                unsigned int transfert_de_stations = to_arret->getHeureArrivee() - from_arret->getHeureArrivee();

                auto to_ligne_id = toutes_lignes.at(tous_voyages.at(to_arret->getVoyageId()).getLigne()).getId();

                if (transfert_de_stations >= transfer_time && to_ligne_id != from_ligne_id){
                    if(j == 0){
                        lowest = transfert_de_stations;
                        lowest_arret = to_arret;
                    }
                    else if (transfert_de_stations < lowest){
                        lowest = transfert_de_stations;
                        lowest_arret = to_arret;
                    }
                }
                j++;
//                if (transfert_de_stations >= transfer_time && to_ligne_id != from_ligne_id){
//                    unsigned int index_from = m_sommetDeArret[from_arret];
//                    unsigned int index_to = m_sommetDeArret[to_arret];
//                    m_leGraphe.ajouterArc(index_from, index_to, transfert_de_stations);
//                }
            }
            unsigned int index_from = m_sommetDeArret[from_arret];
            unsigned int index_to = m_sommetDeArret[lowest_arret];
            m_leGraphe.ajouterArc(index_from, index_to, lowest);


            i++;
        }

    }

}

//! \brief ajouts des arcs d'une station à elle-même pour les stations qui ne sont pas dans DonneesGTFS::m_stationsDeTransfert
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttente(const DonneesGTFS & p_gtfs)
{
    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();
    auto tous_voyages = p_gtfs.getVoyages();
    auto toutes_lignes = p_gtfs.getLignes();
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


