/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifndef simfab_h
#define simfab_h

#include "dataobj/koord3d.h"
#include "dataobj/translator.h"

#include "tpl/slist_tpl.h"
#include "tpl/vector_tpl.h"
#include "besch/fabrik_besch.h"
#include "halthandle_t.h"
#include "simworld.h"


class spieler_t;
class stadt_t;


// production happens in every second
#define PRODUCTION_DELTA_T (1024)
// error of shifting
#define BASEPRODSHIFT (8)
// base production=1 is 16 => shift 4
#define MAX_PRODBASE_SHIFT (4)


// up to this distance, factories will be connected to their towns ...
#define CONNECT_TO_TOWN_SQUARE_DISTANCE 5000

// to prepare for 64 precision ...
class ware_production_t
{
private:
	const ware_besch_t *type;
public:
	const ware_besch_t* get_typ() const { return type; }
	void set_typ(const ware_besch_t *t) { type=t; }
	sint32 menge;	// in internal untis shifted by precision (see produktion)
	sint32 max;
	sint32 abgabe_sum;	// total this month (in units)
	sint32 abgabe_letzt;	// total last month (in units)
};


/**
 * Eine Klasse f�r Fabriken in Simutrans. Fabriken produzieren und
 * verbrauchen Waren und beliefern nahe Haltestellen.
 *
 * Die Abfragefunktionen liefern -1 wenn eine Ware niemals
 * hergestellt oder verbraucht wird, 0 wenn gerade nichts
 * hergestellt oder verbraucht wird und > 0 sonst
 * (entspricht Vorrat/Verbrauch).
 * 
 * A class of factories in Simutrans.
 *  Factories produce and consume goods and supplies near bus stops. 
 * The query functions return -1 if a product is never produced or consumed, 
 * 0 when nothing is manufactured or consumed and> 0 otherwise (equivalent to stocks / consumption).
 * @date 1998
 * @see haltestelle_t
 * @author Hj. Malthaner
 */
class fabrik_t
{
public:
	/**
	 * Konstanten
	 * @author Hj. Malthaner
	 */
	enum { precision_bits = 10, old_precision_bits = 10, precision_mask = 1023 };

private:
	/**
	 * Die m�glichen Lieferziele
	 * 
	 * The possible delivery targets
	 * @author Hj. Malthaner
	 */
	vector_tpl <koord> lieferziele;
	uint32 last_lieferziel_start;

	/**
	 * suppliers to this factry
	 * @author hsiegeln
	 */
	vector_tpl <koord> suppliers;

	/**
	 * fields of this factory (only for farms etc.)
	 * @author prissi
	 */
	vector_tpl <koord> fields;

	/**
	 * Die erzeugten waren auf die Haltestellen verteilen
	 * The produced were distributed at the stops
	 * @author Hj. Malthaner
	 */
	void verteile_waren(const uint32 produkt);

	/* still needed for the info dialog; otherwise useless
	 */
	slist_tpl<stadt_t *> arbeiterziele;

	spieler_t *besitzer_p; //"possessive" (Google)
	karte_t *welt;

	const fabrik_besch_t *besch;

	/**
	 * Bauposition gedreht?
	 * Building position turned?
	 * @author V.Meyer
	 */
	uint8 rotate;

	/**
	 * produktionsgrundmenge
	 * "production fundamental set"
	 * @author Hj. Malthaner
	 */
	sint32 prodbase;

	/**
	 * multiplikator f�r die Produktionsgrundmenge
	 * multiplier for the production of basic quantity
	 * @author Hj. Malthaner
	 */
	sint32 prodfaktor;

	vector_tpl<ware_production_t> eingang; //< das einganslagerfeld ("in goose camp field" (!!) (Babelfish)
	vector_tpl<ware_production_t> ausgang; //< das ausgangslagerfeld ("the output camp field") (Babelfish)

	/**
	 * Zeitakkumulator f�r Produktion
	 * Time accumulator for production
	 * @author Hj. Malthaner
	 */
	sint32 delta_sum;

	// true, if the factory did produce something in the last step
	bool currently_producing;

	// power that can be currently drawn from this station (or the amount delivered)
	uint32 power;

	uint32 total_input, total_output;
	uint8 status;

	/**
	 * Die Koordinate (Position) der fabrik
	 * @author Hj. Malthaner
	 */
	koord3d pos;

	void recalc_factory_status();

	/**
	 * increase the amount for a fixed time PRODUCTION_DELTA_T
	 * @author Hj. Malthaner
	 */
	uint32 produktion(uint32 produkt) const;

public:
	fabrik_t(karte_t *welt, loadsave_t *file);
	fabrik_t(koord3d pos, spieler_t* sp, const fabrik_besch_t* fabesch);
	~fabrik_t();

	static fabrik_t * get_fab(const karte_t *welt, const koord pos);

	/**
	 * @return vehicle description object
	 * @author Hj. Malthaner
	 */
	const fabrik_besch_t *get_besch() const {return besch; }

	void laden_abschliessen();

	void set_pos( koord3d p ) { pos = p; }

	void rotate90( const sint16 y_size );

	void link_halt(halthandle_t halt);
	void unlink_halt(halthandle_t halt);

	const vector_tpl<koord>& get_lieferziele() const { return lieferziele; }
	const vector_tpl<koord>& get_suppliers() const { return suppliers; }

	/* workers origin only used for info dialog purposes and saving; otherwise useless ...
	 * @author Hj. Malthaner/prissi
	 */
	void  add_arbeiterziel(stadt_t *s) { if(!arbeiterziele.is_contained(s)) arbeiterziele.insert(s); }
	void  remove_arbeiterziel(stadt_t *s) { arbeiterziele.remove(s); }
	void  clear_arbeiterziele() { arbeiterziele.clear(); }
	const slist_tpl<stadt_t*>& get_arbeiterziele() const { return arbeiterziele; }

	/**
	 * F�gt ein neues Lieferziel hinzu
	 * Adds a new delivery goal
	 * @author Hj. Malthaner
	 */
	void  add_lieferziel(koord ziel);
	void  rem_lieferziel(koord pos);

	bool disconnect_consumer(koord pos);
	bool disconnect_supplier(koord pos);

	/**
	 * adds a supplier
	 * @author Hj. Malthaner
	 */
	void  add_supplier(koord pos);
	void  rem_supplier(koord pos);



	/**
	 * @return menge der ware typ ("quantity of the goods type")
	 *   -1 wenn typ nicht produziert wird ("if not type is produced")
	 *   sonst die gelagerte menge ("otherwise the stored quantity")
	 */
	sint32 input_vorrat_an(const ware_besch_t *ware);        // Vorrat von Warentyp ("Inventories of product")
	sint32 vorrat_an(const ware_besch_t *ware);        // Vorrat von Warentyp

	// returns all power and consume it
	uint32 get_power() { uint32 p=power; power=0; return p; }

	// give power to the factory to consume ...
	void add_power(uint32 p) { power += p; }

	// true, if there was production in the last step
	bool is_currently_producing() const { return currently_producing; }

	/**
	 * @return 1 wenn verbrauch,
	 * 0 wenn Produktionsstopp,
	 * -1 wenn Ware nicht verarbeitet wird
	 */
	sint32 verbraucht(const ware_besch_t *);             // Nimmt fab das an ?? ("Notes to the fab?")
	sint32 hole_ab(const ware_besch_t *, sint32 menge );     // jemand will waren abholen ("someone wants to pick up were")
	sint32 liefere_an(const ware_besch_t *, sint32 menge);

	sint32 get_abgabe_letzt(sint32 t) { return ausgang[t].abgabe_letzt; }

	void step(long delta_t);                  // fabrik muss auch arbeiten ("factory must also work")
	void neuer_monat();

	const char *get_name() const { return besch ? translator::translate(besch->get_name()) : "unnamed"; }
	sint32 get_kennfarbe() const { return besch ? besch->get_kennfarbe() : 0; }
	spieler_t *get_besitzer() const { return welt->lookup(pos) ? welt->lookup(pos)->first_obj()->get_besitzer() : NULL; }

	void zeige_info() const;
	void info(cbuffer_t& buf) const;

	void rdwr(loadsave_t *file);

	inline koord3d get_pos() const { return pos; }

	/**
	 * gibt eine NULL-Terminierte Liste von Fabrikpointern zur�ck
	 *
	 * a zero-scheduled list of factory pointers returns
	 * @author Hj. Malthaner
	 */
	static vector_tpl<fabrik_t *> & sind_da_welche(karte_t *welt, koord min, koord max);

	/**
	 * gibt true zurueck wenn sich ein fabrik im feld befindet
	 * 
	 * "gives true back if factory in the field is"
	 *
	 * @author Hj. Malthaner
	 */
	static bool ist_da_eine(karte_t *welt, koord min, koord max);
	static bool ist_bauplatz(karte_t *welt, koord pos, koord groesse, bool water, climate_bits cl);

	// hier die methoden zum parametrisieren der Fabrik
	// "here the methods to parameterize the factory"

	/**
	 * Baut die Geb�ude f�r die Fabrik
	 *
	 * "Build the buildings for the factory"
	 *
	 * @author Hj. Malthaner, V. Meyer
	 */
	void baue(sint32 rotate);

	sint16 get_rotate() const { return rotate; }

	/* field generation code
	 * spawns a field for sure if probability>=1000
	 * @author Kieron Green
	 */
	bool add_random_field(uint16 probability);

	void remove_field_at(koord pos);

	uint32 get_field_count() const { return fields.get_count(); }

	/**
	 * total and current procduction/storage values
	 * @author Hj. Malthaner
	 */
	const vector_tpl<ware_production_t>& get_eingang() const { return eingang; }
	const vector_tpl<ware_production_t>& get_ausgang() const { return ausgang; }

	/**
	 * Produktionsmultiplikator
	 * @author Hj. Malthaner
	 */
	void set_prodfaktor(sint32 i) { prodfaktor = (i < 16 ? 16 : i); }
	sint32 get_prodfaktor(void) const { return prodfaktor; }

	/* does not takes month length into account */
	sint32 get_base_production() const { return prodbase; }
	void set_base_production( sint32 p ) {prodbase = p; }

	sint32 get_current_production() const { return (prodbase * prodfaktor * 16l)>>(26l-(long)welt->ticks_bits_per_tag); }

	/* prissi: returns the status of the current factory, as well as output */
	enum { bad, medium, good, inactive, nothing };
	static unsigned status_to_color[5];

	uint8  get_status()    const { return status;       }
	uint32 get_total_in()  const { return total_input;  }
	uint32 get_total_out() const { return total_output; }

	/**
	 * Crossconnects all factories
	 * @author prissi
	 */
	void add_all_suppliers();

	/* adds a new supplier to this factory
	 * fails if no matching goods are there
	 */
	bool add_supplier(fabrik_t* fab);

	/* adds a new customer to this factory
	 * fails if no matching goods are accepted
	 */
	bool add_customer(fabrik_t* fab);
};

#endif
