#include <stdio.h>
main(int argc, char **argv)
{
  FILE *ifp = stdin, *ofp;
  char outname[100];
  char line[1000];
  int i, c;
    if (argc == 2)
    {
        if (!(ifp = fopen(argv[1], "r")))
            printf("Can't open %s\n", argv[1]);
        sprintf(outname, "%s2", argv[1]);
        if (!(ofp = fopen(outname, "w")))
            printf("Can't open %s\n", argv[1]);
        printf("output in %s\n", outname);
    }
    else ofp = stdout;
    while (fgets(line, 10000, ifp))
        if (!strncmp(line, "%!PS", 4))
            break;
    fputs(line, ofp);
    while ((c = getc(ifp)) != EOF)
        putc(c, ofp);
    fclose(ofp);
    fclose(ifp);
}
