main(argc, argv)
int argc;
char *argv[];
{
  char *s, *getenv();
  int status = 1;

	if (argc > 1)
	{
		s = getenv(argv[1]);
		if (s) 
		{	
			printf("%s\n", s);
			status = 0;
		}
	}
	exit(status);
}
