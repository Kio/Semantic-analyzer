python generateSentence.py "$1"
make
SESSION_MANAGER=''
export DISPLAY=:0.0
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/ivan/TurboParser-2.1.0/deps/local/lib:"
#./main --syntmark test1142.xml newmmodel.txt newsyntmodel.txt output.txt /home/ivan/TurboParser-2.1.0/TurboParser
#./main --morphmark testSentence.xml morphmark_output.txt newmmodel.txt
./main --syntmark testSentence.xml newmmodel.txt newsyntmodel.txt output_upper.txt /home/ivan/TurboParser-2.1.0/TurboParser
#head -n 100 output.txt
python toLowerCase.py

