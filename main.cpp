#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <utility>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
using namespace std;

class Sentence
{
    public:
        string text;

        Sentence(string _text) : text(_text) {}
};

class InputFile {
    public:
        InputFile(const char *path) {
            file.open(path);
        }

        ~InputFile() {
        	file.close();
        }

        /**
         * Get next sentence from input file.
         *
         * @return NULL if file is read, otherwise return ptr to next Sentence.
         */
        Sentence *nextSentence() {
            string text = "";
            char symbol;
            static char _end_characters[] = {'.', '!', '?', ';'};
            static set<char> end_characters(_end_characters,
                                            _end_characters + sizeof(_end_characters));
            while (!file.eof()) {
                symbol = file.get();
                text += symbol;
                if (end_characters.find(symbol) != end_characters.end()) {
                    if (text.length() > 1) { // if sentence is not empty
                        return new Sentence(text);
                    } else {
                        text = "";
                    }
                }
            }
            return NULL;
        }
    private:
        ifstream file;
};

enum Tag {
	NOUN,
	VERB,
	PRONOUN,
	ADJECTIVE,
	DETERMINER,
	POSSESSIVE_ENDING,
	CARDINAL_NUMBER,
	ADVERB,
	COORDINATING_CONJUNCTION,
	TO,
	IN,
	PUNCTUATION
};

enum DependTag
{
	AMOD,
	PCOMP,
	PRED,
	COORD,
	OBJ,
	SUBJ,
	CIRC
};

class SyntaxInfo {
	public:
		string word;
		string morphological_form_of_word;
		Tag tag;
		DependTag dep_tag;
		string s_tag;
		string s_dep_tag;
		vector<int> links;
		vector<string> semantic;
		int link;

		SyntaxInfo(string _word,
				   string _morphological_form_of_word,
				   Tag _tag,
				   int _link) : word(_word),
								morphological_form_of_word(_morphological_form_of_word),
								tag(_tag),
								link(_link) {}

		SyntaxInfo() {}
};

enum Language
{
    RUS,
    ENG
};

class Parser {
    public:
        Parser(Language _lang) : lang(_lang) {

        }

        /**
         * Syntax parsing of sentence.
         *
         * @param	sentence Sentence to parse.
         * @return	Return vector of infos for words in sentence. Each info consists of
		 *			morphological and syntactic information.
         */
        vector<SyntaxInfo>
        parse(Sentence *sentence) {
            pid_t pid;
            if (pid = fork()) { // parrent
            	wait(NULL);
				vector<SyntaxInfo> synt_info;
				ifstream sem_out("../RussianDependencyParser/output.txt");
				string tmp;
				char buff[256];
				while (!sem_out.eof()) {
					SyntaxInfo info;
					sem_out >> tmp
							>> info.word
							>> info.morphological_form_of_word
							>> info.s_tag
							>> info.s_tag
							>> tmp
							>> info.link
							>> info.s_dep_tag;
					transform(info.morphological_form_of_word.begin(),
							  info.morphological_form_of_word.end(),
							  info.morphological_form_of_word.begin(), ::tolower);
					sem_out.getline(buff, 255); // skip all chars to the end of line
					switch (info.s_tag[0]) {
						case 'n':
							info.tag = NOUN;
							break;
						case 'v':
						case 'm':
							info.tag = VERB;
							break;
						case 'j':
							info.tag = ADJECTIVE;
							break;
						case 'd':
							info.tag = DETERMINER;
							break;
						case 'p':
							info.tag = PRONOUN;
							break;
						case 'c':
							info.tag = COORDINATING_CONJUNCTION;
							break;
						default:
							info.tag = PUNCTUATION;
							break;
					}
					switch (info.s_dep_tag[0])
					{
						case 's':
							info.dep_tag = SUBJ;
							break;
						case 'p':
							if (info.s_dep_tag[1] == 'r') info.dep_tag = PRED;
							if (info.s_dep_tag[1] == 'c') info.dep_tag = PCOMP;
							break;
						case 'o':
							info.dep_tag = OBJ;
							break;
						case 'a':
							if (info.s_dep_tag[1] == 'm' && info.s_dep_tag[2] == 'o') info.dep_tag = AMOD;
							break;
						case 'c':
							if (info.s_dep_tag[1] == 'o' && info.s_dep_tag[2] == 'o') info.dep_tag = COORD;
							if (info.s_dep_tag[1] == 'i') info.dep_tag = CIRC;
							break;

						default:
							break;
					}


					synt_info.push_back(info);
				}
				sem_out.close();
				return synt_info;
            } else { // child
                switch (lang)
                {
                	case ENG:
                        execlp("./en.sh","en.sh",sentence->text.c_str(),NULL);
                		break;

                	case RUS:
                        execlp("./ru.sh","ru.sh",sentence->text.c_str(),NULL);
                		break;
                }

            }
        }
    private:
        Language lang;
};

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

class Multimap {
public:
	Multimap() {}

	void insert(string key, DependTag val) {
		v.push_back(pair<string, DependTag>(key, val));
	}

	vector<DependTag> equal_range(string key) {
		vector<DependTag> tags_v;
		for (unsigned int i = 0; i < v.size(); ++i) {
		    if (v[i].first == key) {
				tags_v.push_back(v[i].second);
		    }
		}
		return tags_v;
	}

	DependTag operator[] (int index) {
		return v[index].second;
	}
private:
	vector<pair<string, DependTag> > v;
};

class Dependency {
	public:
		static Multimap match;

		Dependency() {}

		static void
		config() {
			match.insert("к_доп", AMOD);
			match.insert("п_доп", AMOD);
			match.insert("к_доп", PCOMP);
			match.insert("п_доп", PCOMP);
			match.insert("к_доп", PRED);
			match.insert("п_доп", PRED);
			match.insert("к_доп", COORD);
			match.insert("п_доп", COORD);
			match.insert("к_доп", OBJ);
			match.insert("п_доп", OBJ);

			match.insert("подл", AMOD);
			match.insert("подл", SUBJ);
			match.insert("подл", PRED);
			match.insert("подл", OBJ);

			match.insert("прим_опр", AMOD);
			match.insert("с_опр",    AMOD);
			match.insert("прим_опр", PCOMP);
			match.insert("с_опр",    PCOMP);
			match.insert("прим_опр", CIRC);
			match.insert("с_опр",    CIRC);
			match.insert("прим_опр", OBJ);
			match.insert("с_опр",    OBJ);

			match.insert("amod", AMOD);
			match.insert("pcomp", PCOMP);
			match.insert("pred", PRED);
			match.insert("coord", COORD);
			match.insert("obj", OBJ);
			match.insert("subj", SUBJ);
			match.insert("cir", CIRC);
		}
};
Multimap Dependency::match;

class Map {
public:
	Map() {}

	void insert(pair<string, DependTag> key, vector<string> val, string _gf) {
		v.push_back(pair< pair<string, DependTag>, vector<string> >(key, val));
		gf.push_back(_gf);
	}

	int find(pair<string, DependTag> key) {
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (v[i].first.first == key.first && v[i].first.second == key.second) return i;
		}
		return -1;
	}

	vector<string>& operator[] (int index) {
		return v[index].second;
	}

	string get_gf(int index) {
		return gf[index];
	}
private:
	vector< pair<pair<string, DependTag>, vector<string> > > v;
	vector<string> gf;
};

vector<pair<string, string> > cat;
vector<pair<string, string> > cat0;
vector<pair<string, string> > sense;
vector<pair<string, string> > word_gf;
vector<pair<string, string> > gf0;
vector<pair<string, vector<string> > > word_sf;
vector<pair<string, vector<string> > > sf0;

string find_in_v(vector<pair<string, string> > &v, string &str) {
	for (unsigned int i = 0; i < v.size(); ++i) {
	    if (v[i].first == str) return v[i].second;
	}
	return "";
}

vector<string>*
find_v_in_v(vector<pair<string, vector<string> > > &v, string &str) {
	for (unsigned int i = 0; i < v.size(); ++i) {
	    if (v[i].first == str) return &v[i].second;
	}
	return NULL;
}

class SemanticInfo {
	public:
		Map info;

		SemanticInfo() {}

		void
		load(const char *path) {
			ifstream file;
			file.open(path);

			string word, tmp, gf;
			vector<DependTag> tags;
			file >> tmp;
			while (!file.eof()) {
				if (tmp == "TITLE") {
					file >> tmp >> word >> tmp;
					transform(word.begin(), word.end(), word.begin(), ::tolower);
				} else if (tmp[1] == 'F' && tmp.size() > 2) { // GFN or SFN where N = 0, 1, 2, ... (but not GF or SF)
					if (tmp[2] == '0') {
						if (tmp[0] == 'G') { // GF0
							file >> tmp;
							getline(file, tmp);
							gf0.push_back(pair<string, string>(word, tmp));
							file >> tmp;
						} else { // SF0
							vector<string> semantic;
							file >> tmp >> tmp;
							while (is_number(tmp)) {
								file >> tmp; // semantic info
								semantic.push_back(tmp);
								getline(file, tmp);
								file >> tmp;
								sf0.push_back(pair<string, vector<string> >(word, semantic));
							}
						}
					} else if (tmp[0] == 'G') {
						tags.clear();
						file >> tmp >> tmp; // =
						while (is_number(tmp)) {
							file >> tmp; // tag

							tags = Dependency::match.equal_range(tmp);
							gf = tmp;

							getline(file, tmp);
							gf += tmp;
							file >> tmp;
						}
					} else if (tmp[0] == 'S') {
						vector<string> semantic;
						file >> tmp >> tmp; // =
						while (is_number(tmp)) {
							file >> tmp; // semantic info
							semantic.push_back(tmp);
							getline(file, tmp);
							file >> tmp;
						}
						for (unsigned int i = 0; i < tags.size(); ++i) {
							info.insert(pair<string, DependTag>(word, tags[i]), semantic, gf);
						}
					} else file >> tmp;
				} else if (tmp[1] == 'F') { // GF or SF
					if (tmp[0] == 'G') { // GF
						file >> tmp;
						getline(file, tmp);
						word_gf.push_back(pair<string, string>(word, tmp));
						file >> tmp;
					} else { // SF
						vector<string> semantic;
						file >> tmp >> tmp;
						while (is_number(tmp)) {
							file >> tmp; // semantic info
							semantic.push_back(tmp);
							file >> tmp;
							word_sf.push_back(pair<string, vector<string> >(word, semantic));
						}
					}
				} else if(tmp == "SENSE") {
					file >> tmp >> tmp;
					sense.push_back(pair<string, string>(word, tmp));
					file >> tmp;
				} else if(tmp == "CAT") {
					file >> tmp >> tmp >> tmp;
					cat.push_back(pair<string, string>(word, tmp));
					file >> tmp;
				} else if(tmp == "CAT0") {
					file >> tmp >> tmp >> tmp;
					cat0.push_back(pair<string, string>(word, tmp));
					file >> tmp;
				} else {
					file >> tmp;
				}
			}

            file.close();
		}
};

SemanticInfo semantic_info;
vector<SyntaxInfo> info;
ofstream output, vocabulary;

void
check_word(int from, int to, Sentence *sentence) {
	string morf_form_of_link_word = info[ to ].morphological_form_of_word;
	int it;
	if ((it = semantic_info.info.find(pair<string, DependTag>
								(morf_form_of_link_word, info[from].dep_tag))) != -1) {
		//add to vocabluary
		vocabulary << "============" << endl
				   << "TITLE\t= " << info[from].morphological_form_of_word << endl
				   << "SENSE\t= 1" << endl;
		if (find_in_v(cat0, morf_form_of_link_word) != "") {
			vocabulary << "CAT\t= 1  " << find_in_v(cat0, morf_form_of_link_word) << endl;
		} else {
			vocabulary << "CAT\t= 1  " << info[from].s_tag << endl;
		}

		if (find_in_v(gf0, morf_form_of_link_word) != "")
			vocabulary << "GF\t= " << find_in_v(gf0, morf_form_of_link_word) << endl;
		if (find_v_in_v(sf0, morf_form_of_link_word)) {
			vocabulary << "SF\t=";
			vector<string> *word_sf_arr = find_v_in_v(sf0, morf_form_of_link_word);
			for (unsigned int sf_i = 1; sf_i <= word_sf_arr->size(); ++sf_i) {
				vocabulary << " " << sf_i << "  " << (*word_sf_arr)[sf_i - 1] << endl;
			}
		}

		vocabulary << "GF1\t= 1  " << semantic_info.info.get_gf(it) << endl;

		for (unsigned int j = 0; j < semantic_info.info[it].size(); ++j) {
			info[from].semantic.push_back(semantic_info.info[it][j]);
			if (!j) vocabulary << "SF1\t=";
			else vocabulary << "   \t ";
			vocabulary << " " << (j + 1) << "  " << semantic_info.info[it][j] << endl;
		}

		if (find_in_v(cat, morf_form_of_link_word) != "")
			vocabulary << "CAT0\t= 1  " << find_in_v(cat, morf_form_of_link_word) << endl;
		if (find_in_v(word_gf, morf_form_of_link_word) != "")
			vocabulary << "GF0\t=" << find_in_v(word_gf, morf_form_of_link_word) << endl;
		if (find_v_in_v(word_sf, morf_form_of_link_word)) {
			vocabulary << "SF0\t=";
			vector<string> *word_sf_arr = find_v_in_v(word_sf, morf_form_of_link_word);
			for (unsigned int sf_i = 1; sf_i <= word_sf_arr->size(); ++sf_i) {
				info[to].semantic.push_back((*word_sf_arr)[sf_i - 1]);
				vocabulary << " " << sf_i << " " << (*word_sf_arr)[sf_i - 1] << endl;
			}
		}
		vocabulary << "EXM\t=" << sentence->text << endl;
	}
}

int
main(int argc, char **argv)
{
	Dependency::config();
	semantic_info.load("tests/ru_ross.txt");
	InputFile in_file("tests/input.txt");
	Parser parser(RUS);
	output.open("tests/output.txt");
	vocabulary.open("tests/vocabulary.txt");
	Sentence *sentence;

	int global_i = 1;
	while (sentence = in_file.nextSentence()) {
	    info = parser.parse(sentence);
	    for (unsigned int i = 0; i < info.size(); ++i) {
			// info[i] - word info;
			if (info[i].link == 0) continue;
			info[ info[i].link-1 ].links.push_back(i + 1);
			check_word(i, info[i].link-1, sentence);
	    }
	    for (unsigned int i = 0; i < info.size(); ++i) {
			if (info[i].links.size() == 0) continue;
			for (unsigned int j = 0; j < info[i].links.size(); ++j) {
				check_word(i, info[i].links[j] - 1, sentence);
			}
	    }
	    for (unsigned int i = 0; i < info.size()-1; ++i) {
			output << (i+1)
					   << "\t"
					   << info[i].word
					   << "\t"
					   << info[i].morphological_form_of_word
					   << "\t"
					   << info[i].s_tag
					   << "\t"
					   << info[i].link
					   << "\t"
					   << info[i].s_dep_tag
					   << "\t"
					   << find_in_v(cat, info[i].morphological_form_of_word)
					   << "\t";
			for (unsigned int j = 0; j < info[i].semantic.size(); ++j) {
				if (j > 0) output << ",";
				output << info[i].semantic[j];
			}
			if (info[i].semantic.size() == 0) {
				vector<string>* v = find_v_in_v(word_sf, info[i].morphological_form_of_word);
				if (v) {
					output << (*v)[0];
					for (unsigned int _i = 1; _i < v->size(); ++_i) {
						output << "," << (*v)[_i];
					}
				} else {
					output << "";
				}
			}
			output << "\t"
					   << find_in_v(sense, info[i].morphological_form_of_word)
					   << "\t";
			for (unsigned int j = 0; j < info[i].links.size(); ++j) {
				if (j > 0) output << ",";
				output << info[i].links[j];
			}
			output << endl;
	    }
	    output << endl;
	}
	vocabulary.close();
	output.close();
	return 0;
}
