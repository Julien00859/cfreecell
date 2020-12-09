# freecell solver

Generate freecell boards and search for a solution using best-first
approach and a fine tunned evaluation function.

	 _   _   _   _  |  _   _   _   _
	---------------------------------

	  9S  5C 10D  1C  3C  JC 10C  9C
	  9H  5D  8C  7H  QD  2H  4H  6C
	  KS  8H  6H  2C  8S  1H  JS  8D
	  7S  KC  1S 10S  2S  JH  3D  4S
	  QS  3S  6S  9D  QC  JD  6D 10H
	  7D  7C  4D  5S  2D  KD  5H  1D
	  3H  QH  KH  4C


	Found a solution, game depth is 108

	 1D ->
	 5H ->
	 2D ->  1D
	 KH ->
	 6D ->
	 3D ->  2D
	 QC ->  KD
	 4D ->  3D
	 3H ->  4C
	 6S ->  7D
	 1S ->
	 2S ->  1S
	10H ->  JS
	 QH ->
	 5H ->  6S
	 7C ->
	 3S ->  2S
	 4S ->  3S
	 7C ->  8D
	 3H ->
	 4C ->  5H
	 5S ->  4S
	 6D ->  7C
	 3H ->  4C
	 9D ->
	10S ->
	 2C ->  3H
	 7H ->  8S
	 1C ->
	 2C ->  1C
	 KC ->
	 QH ->  KC
	 8H ->
	 5D ->  4D
	 6D ->  5D
	 5C ->  6H
	10S ->
	 7H ->
	 9D -> 10S
	 8S ->  9D
	 QD ->
	 3C ->  2C
	 7C ->
	 7C ->  8D
	 3H ->
	 4C ->  3C
	 5C ->  4C
	 6H ->  7C
	 7H ->  8C
	 3H ->
	 KH ->
	 5H ->
	 6S ->  5S
	 7D ->  6D
	 QS ->  KH
	 7S ->  6S
	 8S ->  7S
	 QD ->  KS
	 7H ->
	 8C ->  9D
	 7H ->  8C
	10D ->
	 8H ->
	 6H ->
	 7C ->  8H
	 8D ->  7D
	 6C ->  5C
	 9C -> 10H
	 7C ->  6C
	 7H ->
	 8C ->  7C
	 9D ->  8D
	 8H ->  9C
	 QC ->
	10D ->  9D
	 KD ->
	 JD -> 10D
	 QD ->  JD
	 JH ->  QC
	 1H ->
	 2H ->  1H
	10S ->  JH
	 3H ->  2H
	 KD ->  QD
	 8H ->
	 9C ->  8C
	 KS ->
	 9H ->
	 9S ->  8S
	10S ->  9S
	10H ->  JC
	 JS -> 10S
	 4H ->  3H
	10C ->  9C
	 5H ->  4H
	 6H ->  5H
	 7H ->  6H
	 QS ->  JS
	 8H ->  7H
	 9H ->  8H
	10H ->  9H
	 KS ->  QS
	 JC -> 10C
	 JH -> 10H
	 QH ->  JH
	 KH ->  QH
	 QC ->  JC
	 KC ->  QC

	Stats:
	play() call count: 34818720
	tree sizes: 5308648

	________________________________________________________
	Executed in   92,89 secs   fish           external
	   usr time   91,83 secs   24,00 micros   91,83 secs
	   sys time    1,05 secs  863,00 micros    1,05 secs


