# Установка #

1. Необходимо установить [TurboParser](http://www.ark.cs.cmu.edu/TurboParser/).
2. Необходимо установить [RussianDependencyParser](https://github.com/maslenni/RussianDependencyParser) (если он уже был установлен, можно сделать символическую ссылку).
3. В соседнюю директорию установите Semantic analyzer (необходимо, чтобы обе программы были расположены в одной папке).
4. Скопируйте файлы из папки Semantic analyzer/RussianDependencyParser в папку с RussianDependencyParser и замените в файле launch.sh путь к TurboParser.
5. Добавит права на исполнение скриптов ru.sh и ../RussianDependencyParser/launch.sh (chmod +x file.sh)
6. Скомпилируйте Semantic analyzer:

```
#!shell

g++ -o semantic_analyzer main.cpp
```

# Использование #

1. Анализируемый текст поместите в файл tests/input.txt.
2. Запустите Semantic analyzer: `./semantic_analyzer`
3. Результат анализа текста будет в файле tests/output.txt. Формат: номер слова в тексте, слово, морфологическая форма слова, синтаксическая роль слова, ссылка на другое слово в предложении, грамматическая роль слова, синтаксическая роль слова из семантического словаря, семантические роли слова, перечисление номеров слов, имеющих ссылки на данное слово.
4. В файл tests/vocabulary.txt будут добавлены слова, для которых были установлены семантические роли (содержимое этого файла можно добавить в конец файла tests/ru_ross.txt и слова будут добавлены в семантический словарь).
