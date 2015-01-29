python generateSentence.py "$1" "$2"
if "$2" == "end" 
then
	make
	SESSION_MANAGER=''
	export DISPLAY=:0.0
	export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/ivan/TurboParser-2.1.0/deps/local/lib:"
	./main --syntmark testSentence.xml newmmodel.txt newsyntmodel.txt output_upper.txt /home/ivan/TurboParser-2.1.0/TurboParser
	python toLowerCase.py
fi