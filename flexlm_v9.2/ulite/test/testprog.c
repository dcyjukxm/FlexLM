main()
{
   char *s, *foo();

	s = foo();
	foo2();
	printf("answer s/b \"this is a test\", it is \"%s\"\n", s);
}
char *foo()
{
#ifdef NOSTATIC
  char result[100];
#else
  static char result[100];
#endif

	strcpy(result, "this is a test");
	return(result);
}
foo2()
{
  char junk[100];	/* Junk to overwrite the stack space that
			   result would have used if it isn't really static */
	strcpy(junk, "not really a test");
}

