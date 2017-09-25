#include <stdio.h>
main(int argc, char *argv[])
{
  int i;
  FILE *fp;
        if (!(fp = fopen("setup.bat", "w")))
        {
                perror("Can't open makesetup.bat, exiting");
                exit(1);
        }
        for (i = 1; i < argc; i++)
        {
                if (!strcmp(argv[i], "NONDEBUG"))
                        fprintf(fp, "set FLEXLM_MAKE_DEBUGFLAG=NONDEBUG\n");
                else if (!strcmp(argv[i], "DEBUG"))
                        fprintf(fp, "set FLEXLM_MAKE_DEBUGFLAG=\n");
                else if (!strcmp(argv[i], "RELEASE"))
                        fprintf(fp, "set FLEXLM_MAKE_RELEASEFLAG=RELEASE\n");
                else if (!strcmp(argv[i], "INTERNAL"))
                        fprintf(fp, "set FLEXLM_MAKE_RELEASEFLAG=\n");
                else if (!strcmp(argv[i], "DLL"))
                        fprintf(fp, "set FLEXLM_MAKE_DLLFLAG=DLL\n");
                else if (!strcmp(argv[i], "STATIC"))
                        fprintf(fp, "set FLEXLM_MAKE_DLLFLAG=\n");
                else if (!strcmp(argv[i], "i86_w3"))
                        fprintf(fp, "set FLEXLM_MAKE_PLATFORM=i86_w3\n");
                else if (!strcmp(argv[i], "i86_z3"))
                        fprintf(fp, "set FLEXLM_MAKE_PLATFORM=i86_z3\n");
                else if (!strcmp(argv[i], "alpha_n3"))
                        fprintf(fp, "set FLEXLM_MAKE_PLATFORM=alpha_n3\n");
                else if (!strcmp(argv[i], "i86_n3"))
                        fprintf(fp, "set FLEXLM_MAKE_PLATFORM=i86_n3\n");
                else if (!strcmp(argv[i], "it64_n"))
                        fprintf(fp, "set FLEXLM_MAKE_PLATFORM=i64_n5\n");
	            else if (!strcmp(argv[i], "MD"))
                        fprintf(fp, "set FLEXLM_MAKE_MT=MD\n");
                else if (!strcmp(argv[i], "MT"))
                        fprintf(fp, "set FLEXLM_MAKE_MT=\n");
                else
                        printf("Unknown arg %s\n", argv[i]);
        }
        fclose(fp);
        printf("Run setup.bat\n");
        exit(0);
}
