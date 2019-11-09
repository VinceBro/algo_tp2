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

    // chiffres voulus : methode 1 116773, methode 2 44385, methode 3 172294
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
                j++;
            }
            m_arretDuSommet.push_back(*it);
            m_sommetDeArret.insert({*it, counter});
            counter++;
        }
        j = 0;
        i++;
    }
//    const std::map<std::string, Voyage> voyages = p_gtfs.getVoyages();
//
//    unsigned int i = 0;
//
//    for (auto& voyage : voyages) {
//        auto arrets = voyage.second.getArrets();
//
//        std::vector<unsigned int> indexes;
//
//        //Create arret nodes
//        for (auto& arret : arrets) {
//
//            //add to m_sommetdearret
//            m_sommetDeArret.insert({arret, i});
//
//            //addto m_arretdusommet
//            m_arretDuSommet.push_back(arret);
//
//
//            indexes.push_back(i);
//            i++;
//        }
//
//        //Create arcs between nodes
//        for (int j=0; j < indexes.size()-1; j++) {
//            unsigned int origin = indexes[j];
//            unsigned int destination = indexes[j+1];
//
//            //get heure d'arrivee de origin
//            Heure arrivee = (*m_arretDuSommet[origin]).getHeureArrivee();
//
//            //get heure d'arrivee(devrait etre de depart?) de destination
//            Heure depart = (*m_arretDuSommet[destination]).getHeureArrivee();
//
//            //trouve le nombre de secondes entre ces deux temps là
//            int poids = depart - arrivee;
//
//            // add arcs
//            try {
//                m_leGraphe.ajouterArc(origin, destination, poids);
//            } catch (...) {
//                throw std::logic_error("incohérence");
//            }
//        }
//
//
//    }
    cout << "1 : " << getNbArcs() << endl;
	//écrire votre code ici
}
    //comme le nom dit lui cest les transferts et les stations
    //cette fonction là execute 492 fois



//! /! \brief ajouts des arcs dus aux transferts entre stations
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsTransferts(const DonneesGTFS & p_gtfs)
{

    unsigned int arc_deb = getNbArcs();
    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();
    auto tous_voyages = p_gtfs.getVoyages();
    auto toutes_lignes = p_gtfs.getLignes();

    for (auto trans : p_gtfs.getTransferts()){

        unsigned int from = get<0>(trans);
        unsigned int to = get<1>(trans);
        unsigned int min_transfer_time =  get<2>(trans);

        Station from_stat = toutes_stations.at(from);
        Station to_stat = toutes_stations.at(to);

        auto to_arrets = to_stat.getArrets();
        for (auto from_heure_et_arret : from_stat.getArrets()){
            Arret::Ptr from_arret = from_heure_et_arret.second;
            std::multimap<Heure, Arret::Ptr>::iterator it;
            string from_numero_ligne = toutes_lignes.at(tous_voyages.at(from_arret->getVoyageId()).getLigne()).getNumero();
            set<string> memory;
            for (it = to_arrets.lower_bound(from_arret->getHeureArrivee()); it!= to_arrets.end(); it++ ){
                    Arret::Ptr to_arret = (*it).second;
                    unsigned int transfer_time = to_arret->getHeureArrivee() - from_arret->getHeureArrivee();

                    string to_numero_ligne = toutes_lignes.at(
                            tous_voyages.at(to_arret->getVoyageId()).getLigne()).getNumero();
                    if (transfer_time >= min_transfer_time and to_numero_ligne != from_numero_ligne and
                        memory.count(to_numero_ligne) == 0) {
                        m_leGraphe.ajouterArc(m_sommetDeArret.at(from_arret), m_sommetDeArret.at(to_arret),
                                              transfer_time);
                        memory.insert(to_numero_ligne);
                }
            }
            memory.clear();
        }
    }

    cout << "2 : " << getNbArcs() - arc_deb << endl;
}

//! \brief ajouts des arcs d'une station à elle-même pour les stations qui ne sont pas dans DonneesGTFS::m_stationsDeTransfert
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttente(const DonneesGTFS & p_gtfs)
{
    unsigned int arc_deb = getNbArcs();
    std::map<unsigned int, Station> toutes_stations = p_gtfs.getStations();
    auto tous_voyages = p_gtfs.getVoyages();
    auto toutes_lignes = p_gtfs.getLignes();
    auto tous_transferts = p_gtfs.getTransferts();
    auto toutes_stations_de_transfert = p_gtfs.getStationsDeTransfert();

    // ligne_revient_deux_fois contient le station_id en clé et le ligne_id qui revient deux fois de celui_ci
    std::set<unsigned int> plus_dune_ligne_pour_station;
    for (auto stat:toutes_stations){
        unsigned int ligne_courante;
        bool is_first = true;
        for (auto arr:stat.second.getArrets())  {
            unsigned int ligne_id = tous_voyages.at(arr.second->getVoyageId()).getLigne();
            if (is_first) {
                ligne_courante = ligne_id;
                is_first = false;
            } else if (ligne_courante != ligne_id){
                plus_dune_ligne_pour_station.insert(stat.first);
            }
        }
    }

    for (auto stat: toutes_stations){
           if (toutes_stations_de_transfert.count(stat.first) == 0  and plus_dune_ligne_pour_station.count(stat.first)) {
               std::multimap<Heure, Arret::Ptr> arrets = stat.second.getArrets();
               std::multimap<Heure, Arret::Ptr>::iterator it;
               std::multimap<Heure, Arret::Ptr>::iterator outer_it;

               for (outer_it = arrets.begin(); outer_it != arrets.end(); ++outer_it) {

                   Arret::Ptr A = (*outer_it).second;
                   set<unsigned int> memory;
                   unsigned int ligne_A = tous_voyages.at(A->getVoyageId()).getLigne();

                   for (it = arrets.lower_bound(A->getHeureArrivee()); it != arrets.end(); ++it) {
                       //TODO check les deux prochaines lignes
//                       if (it != --arrets.end()) {
                           Arret::Ptr B = (*it).second;
                           unsigned int ligne_B = tous_voyages.at(B->getVoyageId()).getLigne();
                           unsigned int temps = B->getHeureArrivee() - A->getHeureArrivee();
                           if (temps >= delaisMinArcsAttente and ligne_A != ligne_B and memory.count(ligne_B) == 0) {
                                   m_leGraphe.ajouterArc(m_sommetDeArret.at(A), m_sommetDeArret.at(B), temps);
                                   memory.insert(ligne_B);
                               }
//                           }
                   }
                   memory.clear();

               }
           }
        }


    cout << "3 : " << getNbArcs() - arc_deb << endl;
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
    Heure heure_debut = p_gtfs.getTempsDebut();
    Heure heure_fin = p_gtfs.getTempsFin();
    Arret arret_origine = Arret(stationIdOrigine,heure_fin,heure_debut, 20010, "null");
    Arret arret_destination = Arret(stationIdDestination,heure_fin,heure_debut, 20010, "null");
    Arret::Ptr ptr_origine = make_shared<Arret>(arret_origine);
    Arret::Ptr ptr_destination = make_shared<Arret>(arret_destination);


    m_arretDuSommet.push_back(ptr_origine);
    m_arretDuSommet.push_back(ptr_destination);
    unsigned int pos_origine= m_arretDuSommet.size() - 2;
    unsigned int pos_destination= m_arretDuSommet.size() - 1;
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

    for (auto stat: toutes_stations){
        Coordonnees stat_coord = stat.second.getCoords();
        double distance_depart =  stat_coord  - p_pointOrigine;
        double distance_arrivee = stat_coord - p_pointDestination;
        if (distance_depart <= distanceMaxMarche){
            std::set<unsigned int> ligne_passees;
            std::multimap<Heure, Arret::Ptr>::iterator it;
            for(auto arr: stat.second.getArrets()){
                unsigned int ligne_id = tous_voyages.at(arr.second->getVoyageId()).getLigne();
                int temps_marche = (distance_depart/vitesseDeMarche) * 3600;
//                int temps_seconde_arret = arr.second->getHeureArrivee() - Heure(0,0,0);
//                int temps_seconde_debut = heure_debut - Heure(0,0,0);
                //temps_marche + temps_seconde_debut > temps_seconde_arret
                if (ligne_passees.count(ligne_id) == 0 ){
                    ligne_passees.insert(ligne_id);

                    int poids = temps_marche + (arr.second->getHeureDepart()- arr.second->getHeureArrivee());
                    m_leGraphe.ajouterArc(pos_origine, m_sommetDeArret[arr.second], poids);
                    m_nbArcsOrigineVersStations++;
                }
            }

        }
        else if( distance_arrivee <= distanceMaxMarche){

            int temps_marche = (distance_depart/vitesseDeMarche) * 3600;
            std::multimap<Heure, Arret::Ptr>::iterator it;
            for (auto arr: stat.second.getArrets()){
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
void ReseauGTFS::enleverArcsOrigineDestination()
{

    cout << m_sommetOrigine << endl;
    cout << m_sommetDestination << endl;
    for (size_t sommet: m_sommetsVersDestination){
        m_leGraphe.enleverArc(sommet, m_sommetDestination);
    }


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


