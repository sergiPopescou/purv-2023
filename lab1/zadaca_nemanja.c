// pitanje sa stackoverflowa
int main(void){
      char *s;
      int ln;
      puts("Enter String");
      // scanf("%s", s);
      gets(s);
      ln = strlen(s); // remove this line to end seg fault
      char *dyn_s = (char*) malloc (strlen(s)+1); //strlen(s) is used here as well but doesn't change outcome
      dyn_s = s;
      dyn_s[strlen(s)] = '\0';
      puts(dyn_s);
      return 0;
    }