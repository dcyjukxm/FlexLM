#include <stdio.h>
#include <stdlib.h>
main(int argc, char **argv)
{
  FILE *ofp;
  int c;
	if (argc != 2) exit(1);

	ofp = fopen(argv[1], "w");
	while ((c = fgetc(stdin)) != EOF)
	{
		fputc(c, ofp);
		fputc(c, stdout);
		fflush(ofp);
		fflush(stdout);
	}
	exit(0);

	

}
