/* $Id: makehtml.c,v 1.7 2003-12-29 16:58:05 geuzaine Exp $ 
   
   Generate a HTML slide show from a set of image files
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define HOME         "http://www.geuz.org/getdp/"
#define TOC          "getdp.toc" /* should contain 1 line per slide */
#define INDEX        "getdp.html"
#define POSTSCRIPT   "getdp.ps"
#define PDF          "getdp.pdf"
#define TITLE        "GetDP presentation"
#define AUTHOR       "Christophe Geuzaine"
#define MAXFILES     1000

int  main(int argc, char *argv[]) {
  int i, j, nbfiles;
  char out[MAXFILES][256], toc[MAXFILES][256];
  FILE *file;
  time_t now;
  
  if(argc < 2){
    printf("usage: %s file(s)\n", argv[0]);
    exit(1);
  } 

  time(&now);

  /* create a table containing the output file names */

  for(i=1 ; i<argc ; i++){
    for(j=strlen(argv[i])-1; j>=0; j--){
      if(argv[i][j] == '.'){
	strncpy(out[i-1],argv[i],j);
	out[i-1][j]='\0';
	break;
      }
    }
    if(j<=0) strcpy(out[i-1],argv[i]);
    strcat(out[i-1], ".html");
  }
  nbfiles=argc-1;

  /* create the index */

  for(i=0;i<MAXFILES;i++) strcpy(toc[i],"");

  if(!(file=fopen(TOC, "r"))){
    printf("could not open file '%s'\n", TOC);
  }
  else{
    for(i=0 ; i<nbfiles ; i++)
      fgets(toc[i], 256, file);
  }

  if(!(file=fopen(INDEX, "w"))){
    printf("could not open file '%s'\n", INDEX);
    exit(1);
  }

  printf("creating [%s]", INDEX);

  fprintf(file, 
	  "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
	  "<html>\n"
	  "<head>\n"
	  "<title>%s - index</title>\n"
	  "<link href=\"/general.css\" rel=\"stylesheet\" type=\"text/css\">\n"
	  "</head>\n"
	  "<body>\n"
	  "<center>\n"
	  "[<a href=\"%s\"> Postscript version </a>]<br>\n"
	  "[<a href=\"%s\"> PDF version </a>]<br>\n"
	  "<P>\n"
	  "<table cellspacing=0 cellpadding=0>\n",
	  TITLE, POSTSCRIPT, PDF);

  for(i=0 ; i<nbfiles ; i++){
    fprintf(file, 
	    "<tr><td align=right>[<a href=\"%s\"> %d/%d </a>]</td>",
	    out[i], i+1, nbfiles);
    fprintf(file, 
	    "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s</td></tr>\n",
	    toc[i]);
  }

  fprintf(file, 
	  "</table>\n"
	  "<p>\n"
	  "This document was generated by <i>%s</i> on <i>%s</i>\n"
	  "<p>\n"
	  "Back to <a href=\"/\">geuz.org</a>/<a href=\"/getdp/\">getdp</a>"
	  "</center>\n"
	  "</body>\n"
	  "</html>\n",
	  AUTHOR, ctime(&now));

  fclose(file);

  /* create the output files */
  
  for(i=0 ; i<nbfiles ; i++){

    if(!(file=fopen(out[i], "w"))){
      printf("could not open file '%s'\n", out[i]);
      exit(1);
    }
    
    printf(" [%s]", out[i]);
    if(i==nbfiles-1) printf("\n");
    
    fprintf(file, 
	    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
	    "<html>\n"
	    "<head>\n"
	    "<title>%s - %d/%d</title>\n"
	    "<link href=\"/general.css\" rel=\"stylesheet\" type=\"text/css\">\n"
	    "</head>\n"
	    "<body>\n"
	    "<center>\n",
	    TITLE, i+1, nbfiles);

    if(i!=0)
      fprintf(file, 
	      "[<a href=\"%s\"> &lt;&lt; </a>]\n"
	      "[<a href=\"%s\"> &lt; </a>]\n",
	      out[0], out[i-1]);
    else
      fprintf(file, 
	      "[ &lt;&lt; ]\n"
	      "[ &lt; ]\n");

    fprintf(file, 
	    "[<a href=\"%s\"> Index </a>]\n",
	    INDEX);

    if(i==nbfiles-1)
      fprintf(file, 
	      "[ &gt; ]\n"
	      "[ &gt;&gt; ]\n");
    else
      fprintf(file, 
	      "[<a href=\"%s\"> &gt; </a>]\n"
	      "[<a href=\"%s\"> &gt;&gt; </a>]\n",
	      out[i+1], out[nbfiles-1]);
    
    fprintf(file, 
	    "<p>\n"
	    "<img src=\"%s\">\n"
            "<p>\n"
	    "Back to <a href=\"/\">geuz.org</a>/<a href=\"/getdp/\">getdp</a>\n"
	    "</center>\n"
	    "</body>\n"
	    "</html>\n",
	    argv[i+1]);

    fclose(file);

  }

  return 0;
}
