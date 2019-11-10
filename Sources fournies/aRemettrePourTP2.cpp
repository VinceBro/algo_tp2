//
// Created by Mario Marchand on 16-12-30.
//

#include "ReseauGTFS.h"
#include <sys/time.h>

using namespace std;

//! \brief ajout des arcs dus aux voyages
//! \brief insère les arrêts (associés aux sommets) dans m_arretDuSommet et m_sommetDeArret
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsVoyages(const DonneesGTFS &p_gtfs) {

    unsigned int counter = 0;
    for (auto voy : p_gtfs.getVoyages()) {
        std::set<Arret::Ptr, Voyage::compArret> arrets = voy.second.getArrets();
        std::set<Arret::Ptr, Voyage::compArret>::iterator it;
        for (it = arrets.begin(); it != arrets.end(); ++it) {
            m_arretDuSommet.push_back(*it);
            m_sommetDeArret.insert({*it, counter});
            counter++;
        }
        for (it = arrets.begin(); it != --arrets.end(); ++it) {
            int temps = (*(next(it)))->getHeureArrivee() - (*it)->getHeureArrivee();
            if (temps < 0) throw logic_error("Incohérence : Poids négatif");
            m_leGraphe.ajouterArc(m_sommetDeArret.at((*it)), m_sommetDeArret.at((*next(it))), temps);
        }
    }
}
//écrire votre code ici



//! /! \brief ajouts des arcs dus aux transferts entre stations
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsTransferts(const DonneesGTFS &p_gtfs) {

    unsigned int arc_deb = getNbArcs();
    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();
    auto tous_voyages = p_gtfs.getVoyages();
    auto toutes_lignes = p_gtfs.getLignes();

    for (auto trans : p_gtfs.getTransferts()) {

        unsigned int from = get<0>(trans);
        unsigned int to = get<1>(trans);
        unsigned int min_transfer_time = get<2>(trans);

        Station from_stat = toutes_stations.at(from);
        Station to_stat = toutes_stations.at(to);

        auto to_arrets = to_stat.getArrets();
        for (auto from_heure_et_arret : from_stat.getArrets()) {
            Arret::Ptr from_arret = from_heure_et_arret.second;
            std::multimap<Heure, Arret::Ptr>::iterator it;
            string from_numero_ligne = toutes_lignes.at(
                    tous_voyages.at(from_arret->getVoyageId()).getLigne()).getNumero();
            set<string> memory;
            for (it = to_arrets.lower_bound(from_arret->getHeureArrivee()); it != to_arrets.end(); it++) {
                Arret::Ptr to_arret = (*it).second;
                unsigned int transfer_time = to_arret->getHeureArrivee() - from_arret->getHeureArrivee();

                string to_numero_ligne = toutes_lignes.at(
                        tous_voyages.at(to_arret->getVoyageId()).getLigne()).getNumero();
                if (transfer_time >= min_transfer_time and to_numero_ligne != from_numero_ligne and
                    memory.count(to_numero_ligne) == 0) {

                    // Le try suivant catch le logic error de .at() du conteneur map et relance une logic_error plus spécifique
                    try {
                        m_leGraphe.ajouterArc(m_sommetDeArret.at(from_arret), m_sommetDeArret.at(to_arret),
                                              transfer_time);
                    }
                    catch (logic_error) {
                        throw logic_error("Incohérence : L'arrêt n'est pas stocké dans m_sommetDeArret");
                    }

                    memory.insert(to_numero_ligne);
                }
            }
            memory.clear();
        }
    }

}

//! \brief ajouts des arcs d'une station à elle-même pour les stations qui ne sont pas dans DonneesGTFS::m_stationsDeTransfert
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttente(const DonneesGTFS &p_gtfs) {
    unsigned int arc_deb = getNbArcs();
    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();
    auto tous_voyages = p_gtfs.getVoyages();
    auto toutes_lignes = p_gtfs.getLignes();
    auto tous_transferts = p_gtfs.getTransferts();
    auto toutes_stations_de_transfert = p_gtfs.getStationsDeTransfert();

    // La double boucle suivante mets les stations qui ont plus d'une ligne dans le set plus_dune_ligne_pour_station
    std::set<unsigned int> plus_dune_ligne_pour_station;
    for (auto stat:toutes_stations) {
        unsigned int ligne_courante;
        bool is_first = true;
        for (auto arr:stat.second.getArrets()) {
            unsigned int ligne_id = tous_voyages.at(arr.second->getVoyageId()).getLigne();
            if (is_first) {
                ligne_courante = ligne_id;
                is_first = false;
            } else if (ligne_courante != ligne_id) {
                plus_dune_ligne_pour_station.insert(stat.first);
            }
        }
    }

    for (auto stat: toutes_stations) {
        if (toutes_stations_de_transfert.count(stat.first) == 0 and plus_dune_ligne_pour_station.count(stat.first)) {
            std::multimap<Heure, Arret::Ptr> arrets = stat.second.getArrets();
            std::multimap<Heure, Arret::Ptr>::iterator it;
            std::multimap<Heure, Arret::Ptr>::iterator outer_it;

            for (outer_it = arrets.begin(); outer_it != arrets.end(); ++outer_it) {

                Arret::Ptr A = (*outer_it).second;
                set<unsigned int> memory;
                unsigned int ligne_A = tous_voyages.at(A->getVoyageId()).getLigne();

                for (it = arrets.lower_bound(A->getHeureArrivee()); it != arrets.end(); ++it) {
                    Arret::Ptr B = (*it).second;
                    unsigned int ligne_B = tous_voyages.at(B->getVoyageId()).getLigne();
                    int temps = B->getHeureArrivee() - A->getHeureArrivee();
                    if (temps >= delaisMinArcsAttente and ligne_A != ligne_B and memory.count(ligne_B) == 0) {


                        // Le try suivant catch le logic error du .at() du conteneur map et relance une logic_error plus spécifique
                        try { m_leGraphe.ajouterArc(m_sommetDeArret.at(A), m_sommetDeArret.at(B), temps); }
                        catch (logic_error) {
                            throw logic_error("Incohérence : L'arrêt n'est pas stocké dans m_sommetDeArret");
                        }

                        memory.insert(ligne_B);
                    }
                }
                memory.clear();

            }
        }
    }


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
                                               const Coordonnees &p_pointDestination) {
    if (m_origine_dest_ajoute) throw logic_error("Incohérence : Les arcs d'origine ont déja été construits");
    Heure heure_debut = p_gtfs.getTempsDebut();
    Heure heure_fin = p_gtfs.getTempsFin();
    Arret arret_origine = Arret(stationIdOrigine, heure_fin, heure_debut, 0, "null");
    Arret arret_destination = Arret(stationIdDestination, heure_fin, heure_debut, 0, "null");
    Arret::Ptr ptr_origine = make_shared<Arret>(arret_origine);
    Arret::Ptr ptr_destination = make_shared<Arret>(arret_destination);


    m_arretDuSommet.push_back(ptr_origine);
    m_arretDuSommet.push_back(ptr_destination);
    unsigned int pos_origine = m_arretDuSommet.size() - 2;
    unsigned int pos_destination = m_arretDuSommet.size() - 1;
    m_sommetDeArret.insert({ptr_origine, pos_origine});
    m_sommetDeArret.insert({ptr_destination, pos_destination});
    m_sommetOrigine = pos_origine;
    m_sommetDestination = pos_destination;
    m_nbArcsStationsVersDestination = 0;
    m_nbArcsOrigineVersStations = 0;

    unsigned int graph_size = m_leGraphe.getNbSommets() + 2;
    m_leGraphe.resize(graph_size);

    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();
    auto tous_voyages = p_gtfs.getVoyages();

    for (auto stat: toutes_stations) {

        Coordonnees stat_coord = stat.second.getCoords();
        double distance_depart = p_pointOrigine - stat_coord;
        double distance_arrivee = p_pointDestination - stat_coord;

        if (distance_depart <= distanceMaxMarche) {

            std::set<unsigned int> ligne_passees;
            std::multimap<Heure, Arret::Ptr>::iterator it;
            auto arrets = stat.second.getArrets();
            unsigned int temps_marche = (distance_depart / vitesseDeMarche) * 3600;
            Heure heure_modif = heure_debut.add_secondes(temps_marche);

            for (it = arrets.lower_bound(heure_modif); it != arrets.end(); it++) {
                unsigned int ligne_id = tous_voyages.at((*it).second->getVoyageId()).getLigne();
                if (ligne_passees.count(ligne_id) == 0) {
                    ligne_passees.insert(ligne_id);

                    int poids = (*it).second->getHeureArrivee() - heure_debut;
                    m_leGraphe.ajouterArc(pos_origine, m_sommetDeArret[(*it).second], poids);
                    m_nbArcsOrigineVersStations++;
                }
            }

        } else if (distance_arrivee <= distanceMaxMarche) {

            unsigned int temps_marche = (distance_arrivee / vitesseDeMarche) * 3600;
            for (auto arr: stat.second.getArrets()) {
                m_leGraphe.ajouterArc(m_sommetDeArret[arr.second], pos_destination, temps_marche);
                m_sommetsVersDestination.push_back(m_sommetDeArret[arr.second]);
                m_nbArcsStationsVersDestination++;
            }
        }

    }


    m_origine_dest_ajoute = true;
    //écrire votre code ici
}

//! \brief Remet ReseauGTFS dans l'était qu'il était avant l'exécution de ReseauGTFS::ajouterArcsOrigineDestination()
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \throws logic_error si une incohérence est détecté lors de la modification du graphe
//! \post Enlève de ReaseauGTFS tous les arcs allant du point source vers un arrêt de station et ceux allant d'un arrêt de station vers la destination
//! \post assigne la variable m_origine_dest_ajoute à false (les points orignine et destination sont enlevés du graphe)
//! \post enlève les données de m_sommetsVersDestination
void ReseauGTFS::enleverArcsOrigineDestination() {

    if (!m_origine_dest_ajoute)
        throw logic_error("Incoherence : Les arcs d'origine doivent être construits avant d'être enlevés");
    for (size_t sommet: m_sommetsVersDestination) {
        m_leGraphe.enleverArc(sommet, m_sommetDestination);
    }

    m_sommetsVersDestination.clear();


    // Il manque d'enlever les arrêts reliés à origine et cette méthode est finie

    m_nbArcsStationsVersDestination = 0;
    m_nbArcsOrigineVersStations = 0;
    Arret::Ptr arret_origine = m_arretDuSommet.at(m_sommetOrigine);
    Arret::Ptr arret_destination = m_arretDuSommet.at(m_sommetDestination);
    m_sommetDeArret.erase(arret_origine);
    m_sommetDeArret.erase(arret_destination);
    m_arretDuSommet.pop_back();
    m_arretDuSommet.pop_back();

    unsigned int graph_size = m_leGraphe.getNbSommets() - 2;
    m_leGraphe.resize(graph_size);
    m_origine_dest_ajoute = false;
}


