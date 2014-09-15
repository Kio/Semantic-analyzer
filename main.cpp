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
	SUBJ,
	PRED,
	OBJ
};

class SyntaxInfo {
	public:
		string word;
		string morphological_form_of_word;
		Tag tag;
		DependTag dep_tag;
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
				string tmp, string_tag, dep_tag;
				char buff[256];
				while (!sem_out.eof()) {
					SyntaxInfo info;
					sem_out >> tmp
							>> info.word
							>> info.morphological_form_of_word
							>> string_tag
							>> string_tag
							>> tmp
							>> info.link
							>> dep_tag;
					transform(info.morphological_form_of_word.begin(),
							  info.morphological_form_of_word.end(),
							  info.morphological_form_of_word.begin(), ::tolower);
					sem_out.getline(buff, 255); // skip all chars to the end of line
					switch (string_tag[0]) {
						case 'N':
							info.tag = NOUN;
							break;
						case 'V':
						case 'M':
							info.tag = VERB;
							break;
						case 'J':
							info.tag = ADJECTIVE;
							break;
						case 'D':
							info.tag = DETERMINER;
							break;
						case 'P':
							info.tag = PRONOUN;
							break;
						case 'C':
							info.tag = COORDINATING_CONJUNCTION;
							break;
						default:
							info.tag = PUNCTUATION;
							break;
					}
					switch (dep_tag[0])
					{
						case 's':
							info.dep_tag = SUBJ;
							break;
						case 'p':
							info.dep_tag = PRED;
							break;
						case 'o':
							info.dep_tag = OBJ;
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
			match.insert("к_доп", OBJ);
			match.insert("п_доп", OBJ);
			match.insert("подл", SUBJ);
			match.insert("с_опр", PRED);
		}
};
Multimap Dependency::match;

class Map {
public:
	Map() {}

	void insert(pair<string, DependTag> key, vector<string> val) {
		v.push_back(pair< pair<string, DependTag>, vector<string> >(key, val));
	}

	int find(pair<string, DependTag> key) {
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (v[i].first.first == key.first && v[i].first.second == key.second) return i;
		}
		return -1;
	}

	vector<string> operator[] (int index) {
		return v[index].second;
	}
private:
	vector< pair<pair<string, DependTag>, vector<string> > > v;
};

class SemanticInfo {
	public:
		Map info;

		SemanticInfo() {}

		void
		load(const char *path) {
			ifstream file;
			file.open(path);

			string word, tmp;
			vector<DependTag> tags;
			file >> tmp;
			while (!file.eof()) {
				if (tmp == "TITLE") {
					file >> tmp >> word >> tmp;
					transform(word.begin(), word.end(), word.begin(), ::tolower);
				} else if (tmp[1] == 'F' && tmp.size() > 2) { // GFN or SFN where N = 1, 2, ... (but not GF or SF)
					if (tmp[0] == 'G') {
						tags.clear();
						file >> tmp >> tmp; // =
						while (is_number(tmp)) {
							file >> tmp; // tag

							tags = Dependency::match.equal_range(tmp);

							getline(file, tmp);
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
							info.insert(pair<string, DependTag>(word, tags[i]), semantic);
						}
					} else file >> tmp;
				} else {
					file >> tmp;
				}
			}

            file.close();
		}
};

int
main(int argc, char **argv)
{
	SemanticInfo semantic_info;
	semantic_info.load("tests/ru_ross.txt");
	InputFile in_file("tests/input.txt");
	Parser parser(RUS);
	ofstream vocabulary;
	vocabulary.open("tests/vocabulary.txt");
	Dependency::config();
	Sentence *sentence;
	while (sentence = in_file.nextSentence()) {
	    vector<SyntaxInfo> info = parser.parse(sentence);
	    for (unsigned int i = 0; i < info.size(); ++i) {
			// info[i] - word info;
			if (info[i].link == 0) continue;
			info[i].link--;
			string morf_form_of_link_word = info[ info[i].link ].morphological_form_of_word;
			int it;
			if ((it = semantic_info.info.find(pair<string, DependTag>
										(morf_form_of_link_word, info[i].dep_tag))) != -1) {
				vocabulary << morf_form_of_link_word
						   << " : "
						   << info[i].morphological_form_of_word
						   << "(";
				for (unsigned int j = 0; j < semantic_info.info[it].size(); ++j) {
					if (j > 0) vocabulary << ", ";
					vocabulary << semantic_info.info[it][j];
				}
				vocabulary << ")"
						   << endl;
			}
	    }
	}
	vocabulary.close();
	return 0;
}
