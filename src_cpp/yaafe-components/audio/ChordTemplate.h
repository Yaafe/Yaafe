/*
 * Chords.h
 *
 *  Created on: Dec 2, 2009
 *      Author: bmathieu
 */

#ifndef CHORDS_H_
#define CHORDS_H_

#include <string>
#include <vector>

namespace YAAFE_EXT {

class ChordTemplate {
public:
	ChordTemplate(char* name, int q, char* pattern);
	~ChordTemplate();

	inline const std::string& name() const { return m_name; };
	inline const int& quality() const { return m_quality; };
	inline const std::vector<int>& notes() const { return m_notes; };

	static const std::vector<ChordTemplate>& listChordTemplates();
	static const ChordTemplate& getChordTemplate(const std::string& chord);

	static const ChordTemplate UNKNOWN;

private:
	std::string m_name;
	int m_quality;
	std::vector<int> m_notes;

	static std::vector<ChordTemplate> s_chords;
};


}


#endif /* CHORDS_H_ */
