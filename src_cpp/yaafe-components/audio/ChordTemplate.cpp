/*
 * ChordsTemplate.cpp
 *
 *  Created on: Dec 2, 2009
 *      Author: bmathieu
 */

#include "ChordTemplate.h"
#include <iostream>
#include "string.h"

using namespace std;
using namespace YAAFE_EXT;

const ChordTemplate ChordTemplate::UNKNOWN("unknown",0,"");
vector<ChordTemplate> ChordTemplate::s_chords;

ChordTemplate::ChordTemplate(char* n, int q, char* pattern) :
	m_name(n), m_quality(q), m_notes() {

	string ref("0123456789AB");
	for (unsigned int i = 0; i < strlen(pattern); i++)
	{
		size_t note = ref.find(pattern[i]);
		if (note==string::npos)
			cerr << "ERROR: in chord templates definition. " << pattern[i] << " is an invalid note (chord " << n << ")!" << endl;
		else
			m_notes.push_back(note);
	}
}


ChordTemplate::~ChordTemplate() {
}

const std::vector<ChordTemplate>& ChordTemplate::listChordTemplates() {
	if (s_chords.size() == 0) {
		s_chords.push_back(ChordTemplate("maj",0,"047"));
		s_chords.push_back(ChordTemplate("min",1,"037"));
		s_chords.push_back(ChordTemplate("dim",2,"036"));
		s_chords.push_back(ChordTemplate("aug",3,"048"));
		s_chords.push_back(ChordTemplate("maj7",0,"047B"));
		s_chords.push_back(ChordTemplate("min7",1,"037A"));
		s_chords.push_back(ChordTemplate("7",0,"047A"));
		s_chords.push_back(ChordTemplate("dim7",2,"0369"));
		s_chords.push_back(ChordTemplate("hdim7",2,"036A"));
		s_chords.push_back(ChordTemplate("minmaj7",1,"037B"));
		s_chords.push_back(ChordTemplate("maj6",0,"0479"));
		s_chords.push_back(ChordTemplate("min6",1,"0379"));
		s_chords.push_back(ChordTemplate("9",0,"047A2"));
		s_chords.push_back(ChordTemplate("maj9",0,"047B2"));
		s_chords.push_back(ChordTemplate("min9",1,"037A1"));
		s_chords.push_back(ChordTemplate("sus4",4,"057"));
		s_chords.push_back(ChordTemplate("sus2",4,"027"));
	}
	return s_chords;
}

const ChordTemplate& ChordTemplate::getChordTemplate(const std::string& chord) {
	const vector<ChordTemplate>& chords = listChordTemplates();
	for (size_t i = 0; i < chords.size(); i++)
		if (chord == chords[i].name())
			return chords[i];
	cerr << "ERROR: unknown chord " << chord.c_str() << endl;
	return ChordTemplate::UNKNOWN;
}

