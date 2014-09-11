#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
            char symbol; //TODO Support Unicode symbols
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

class SyntaxInfo {
	public:
		string word;
		string morphological_form_of_word;
		Tag tag;
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
                wait();
				vector<SyntaxInfo> synt_info;
				ifstream sem_out("../RussianDependencyParser/output.txt");
				string tmp, string_tag;
				char buff[256];
				while (!sem_out.eof()) {
					SyntaxInfo info;
					sem_out >> tmp
							>> info.word
							>> info.morphological_form_of_word
							>> string_tag
							>> string_tag
							>> tmp
							>> info.link;
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

					synt_info.push_back(info);
				}
				sem_out.close();
				return synt_info;
            } else { // child
                switch (lang)
                {
                	case RUS:
                        execl("en.sh","en.sh",sentence->text.c_str(),NULL);
                		break;

                	case ENG:
                        execl("ru.sh","ru.sh",sentence->text.c_str(),NULL);
                		break;
                }

            }
        }
    private:
        Language lang;
};

int
main(int argc, char **argv)
{
	InputFile in_file("input.txt");
	Sentence *sentence = in_file.nextSentence();
	Parser parser(RUS);
	vector<SyntaxInfo> info = parser.parse(sentence);
	cout << info.size() << endl << info[1].word;
    return 0;
}
