#define RCSID "$Id: Lanczos.c,v 1.13 2003-01-24 23:00:31 geuzaine Exp $"

/* Version comment�e par A. Nicolet de Lanczos.c le 2001/11/29 */

/* Bibliographie

   Numerical Linear Algebra, Trefethen & Bau, SIAM, Philadelphia,
   1997.  Tr�s r�cent et tr�s clair, excellent ouvrage introductif sur
   l'alg�bre lin�aire num�rique moderne�!
   
   Y. Saad, Numerical Methods for Large Eigenvalue Problems,
   Manchester University Press Avantages�: gratuit sur le Web
   (http://www.users.cs.umn.edu/~saad/books.html et le bouquin sur la
   r�solution des syst�mes s'y trouve aussi maintenant�!), assez
   r�cent (1992), tr�s cibl� sur le probl�me comme son titre
   l'indique, tr�s clair avec peu de pr�requis (le premier chapitre
   vous rem�more tout ce que vous devez savoir sur les matrices pour
   commencer), bon niveau th�orique sans abstraction stratosph�rique
   et d�crit pr�cis�ment les ALGORITHMES (dont par exemple la m�thode
   d'Arnoldi-Tchebychev con�u par Saad lui-m�me).
   
   Golub & Van Loan, Matrix Computations, Johns Hopkins University
   Press, 3rd ed, 1996.  La troisi�me �dition de la bible du calcul
   matriciel appliqu�!

   Valeurs propres des matrices, Fran�oise Chatelin, Masson, Paris,
   1988.  Bien cibl� et assez th�orique, plut�t court, un bon texte
   compl�mentaire pour se faire un synth�se.

   Analyse Num�rique, sous la direction de Jacques Baranger, Hermann,
   Paris, 1991, Chapitre 7 par Fran�oise Chatelin. Bon chapitre
   synth�tique par l'auteur de l'ouvrage precedent.

   M. G�radin, D. Rixen, Th�orie des Vibrations, Masson, Paris, 2�me
   ed, 1996.  La r�f�rence utilis�e par Beno�t Meys. Introduction de
   la m�thode de Lanczos comme outil pour l'ing�nieur donc approche
   tr�s pragmatique et tr�s instructive pour le passage aux
   applications.

   Isaacson & Keller, Analysis of Numerical Methods, Dover (�dition
   originale 1966).  L'ouvrage g�n�ral de r�f�rence en calcul
   num�rique dans les ann�es 60.

   J. Stoer, R. Bulirsh, introduction to Numerical Analysis, Springer
   Verlag, 1979.  Un des meilleurs ouvrages g�n�raux de r�f�rence en
   calcul num�rique dans les ann�es 80. Entre Isaacson & Keller et
   Stoer & Bulirsh, il y a le Dahlquist et Bjork des ann�es 70 mais
   c'est un ouvrage beaucoup plus introductif.

   W.H. Press & al., Numerical Recipes, Cambridge University Press,
   � Le best seller du calcul num�rique des ann�es 90. G�n�ration
   fast food oblige, c'est le fast programming. Indispensable en fait
   comme trousse de secours�: un petit rappel th�orique rapide ou une
   petite routine toute faite peuvent �tre des gestes qui sauvent �

   Et enfin, les Wilkinson�: J. H. Wilkinson, Rounding Errors in
   Algebraic Processes, Dover Petit ouvrage sur l'analyse d'erreur
   comme son nom l'indique.

   J. H. Wilkinson, C. Reinsch, Linear Algebra, Springer Verlag, 1971.
   Un pr�-Numerical Recipes plus pouss� sur l'alg�bre lin�aire et avec
   tous les algorithmes en Algol (Je vous parle d'un temps que les
   moins de vingt ans ne peuvent pas conna�tre � ).

   J. H. Wilkinson, Algebraic eigenvalue problem, Clarendon Press,
   1965.  Le livre sacr� du calcul des valeurs propres � que je
   n'ai jamais eu en main. Il va falloir que je mette la main dessus
   �

*/


/* Quelques modifications envisag�es dans le futur (par ordre
   approximatif de difficult� et probablement par ordre chronologique de
   r�alisation future � ) :

   1) Normer les vecteurs propres (valeur absolue max d'un �l�ment = 1
   ) -> OK mais ne resout pas entierement le probleme de l'affichage
   dans le post.

   2) Examiner de plus pr�s et �ventuellement valider ou am�liorer
   quelques d�tails comme par exemple je verrais bien le test if
   (fabs(shift) > 1.e-10) en relatif plut�t qu'en absolu. De toute
   fa�on, si on essaye d'imposer un shift si petit, il vaudrait mieux
   afficher un message d'avertissement�!  CHANGER LES NOMS DE
   CERTAINES VARIABLES

   3) Faire fonctionner LinAlg correctement pour les complexes (au
   moins le produit scalaire�!�!). En gros le codage des matrices et
   des vecteurs complexes du systeme est

   | Re Im |   et  | Re |
   |-Im Re |       | Im |

   Les matrices rectangulaires orthogonales sont stockees dans une
   liste de vecteurs Lan (Changer le nom en Q ou V ?).  Ce qui doit
   �tre fait est le passage a une matrice de Hessenberg complexe Hes
   -> HesR,HesI ... mais on ne peut pas utiliser le codage ci-dessus
   car on perdrait la structure de Hessenberg.

   4) Calculer le RESIDU associ� � un couple : la theorie permet de
   calculer facilement ce r�sultat, voir plus loin -> OK utilis� pour
   selectionner les valeurs propres qui ont converg�.

   5) Am�liorer les valeurs propres et les vecteurs propres a
   posteriori (il y a une m�thode simple expliqu�e dans Numerical
   Recipes, je crois �) et �ventuellemnt introduire un crit�re
   d'arr�t.

   6) Am�lioration de l'algorithme�: Pr�conditionner (il y a des trucs
   la dessus dans Saad, je crois).  R�orthogonaliser les colonnes de
   Qn -> OK mais n'apporte pas grand chose !  Faire des red�marrages
   ... a essayer, ce n'est pas un gros travail, par exemple prendre
   comme point de d�part une combinaison lin�aire (somme !)  des
   vecteurs (norm�s !) qui ont d�j� plus ou moins converg�s !  Hn
   incomplet etc. (cf. Golub & Van Loan, Chatelin) L'algorithme de
   Saad 'Arnoldi-Tchebyshev' semble un bon candidat�!�!

   7)'Fixer omega et chercher gamma' conduit � un probl�me aux VP g�n�ralis� 
   du type�: lambda^2 M1 v + lambda M2 v + M3 v = 0 
   On peut mettre ce probl�me sous la forme d'un probl�me g�n�ralis� de taille 
   'double' du type
   | M1 M2 | |v1| lambda +  | 0  M3 | |v1| = 0
   | 0  I  | |v2|           |-I  0  | |v2|
   Comme d'habitude, on n'a besoin que du produit Matrice Vecteur ce qui 
   revient � travailler avec les 'demi-vecteurs' v1, v2 de v et les 
   sous-matrices M1, M2, M3�: on doit calculer le vecteur 'double'�:
   | -M1 v2 + M2 M3^-1 v1 | 
   | M3^-1 v1             | 
   Il sera surtout n�cessaire d'avoir acc�s aux sous-matrices M1, M2, M3.
   On peut �galement envisager le probl�me des courbes de dispersion�:
   Trouver des paires de nombres (omega,gamma) telles qu'il existe un 
   vecteur v v�rifiant�: gamma^2 M1 v + gamma M2 v + omega^2 M3 v + M4 v = 0
   Le jeu consiste � faire varier (pas trop vite) omega et calculer les 
   nouveaux gammas en utilisant si possible l'information fournie par les 
   r�solutions pr�c�dentes �
   
   8) Autres m�thodes�(puissance inverse�?)
   
   Question�: et si on utilisait une librairie comme ARPACK et son
   interface objet en C++ ARPACK++�?
   http://www.caam.rice.edu/software/ARPACK/
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "GetDP.h"
#include "DofData.h"
#include "Data_Numeric.h"
#include "CurrentData.h"
#include "nrutil.h"

/* 
   calcul des vecteurs propres d'une matrice de Hessenberg r�elle
   (**T) de taille N dont on donne la valeur propre valp r�sultat en
   sortie dans *v 
*/

void cal_vec_pr_T(double **T, int N, double valp, double *v){
  int      i,j,k;
  double **mat,norm=0.0;

  GetDP_Begin("cal_vec_pr_T");

  /* cette procedure devra etre reecrite pour une matrice de
     Hessenberg COMPLEXE */

  mat = dmatrix(1,N,1,N);
  
  for(i=1;i<=N;i++){
    for(j=1;j<=N;j++){
      if(i==j)
	mat[i][j]=T[i][j]-valp;
      else
	mat[i][j]=T[i][j];
    }
  }
  
  /* totalement instable si mat[][] est tres petit.  a
     changer. probl�me � creuser d'une fa�on g�n�rale, consid�rer le
     raffinement des valeurs et vecteurs propres */

  /* r�solution de (T - valp I) v = 0 syst�me ind�termin� car v n'est
     d�fini qu'� une constante multiplicative pr�s on fixe donc v[N] �
     1 !!! pas toujours possible !!!!!  et on r�sout par substitution
     arri�re gr�ce � la forme Hessenberg */
   
  v[N]=1.0;
  for(i=N;i>1;i--){
    v[i-1]=0.0;
    for(j=N;j>=i;j--) v[i-1]-=mat[i][j]*v[j];
    if(mat[i][i-1] != 0.0)
      v[i-1]/=mat[i][i-1];
    else {
      Msg(BIGINFO, " --- INVARIANT SUBSPACE FOUND ! --- ");
      /* v�rifier que la manoeuvre de sortie est valide !!! */
      for(k=i;k<=N;k++)	v[k]=0.0;
      v[i-1]=1.0;
    }
  }

  /* fixer la norme L2 de v � 1 */
  for(i=1;i<=N;i++) norm+=v[i]*v[i];
  norm = sqrt(norm);
  for(i=1;i<=N;i++) v[i]/=norm;

  GetDP_End ;
}



/* 
   Algorithme de Lanczos (IL S'AGIT EN FAIT d' A R N O L D I POUR LES
   SYSTEMES NON SYMETRIQUES)
   sont transmis :
   le pointeur d'un probl�mes avec ses matrices DofData_P
   le nombre d'it�rations � effectuer LanSize
   *LanSave structure de liste pour le sauvetage des vecteurs propres (voir DataStr/List.c)
   le d�callage shift
*/

void Lanczos (struct DofData * DofData_P, int LanSize, List_T *LanSave, double shift){
  gVector  *Lan, *b, *x ;
  gMatrix  *K, *M ;
  int      i, j, k, ii, jj, NbrDof, Restart, ivmax, newsol = 0 ; 
  double   dum, dum1, dum2;
  double   **Hes, **IMatrix, *diag, *wr, *wi, *err ;
  long     mun = -1 ;
  struct Solution Solution_S ;

  /* declaration pour les parametres de eigen.par */
  
  struct eigenpro {
    double prec;
    int    size; /* remplacer LanSize par eigenpar.size */
    int    reortho;
  } eigenpar ;

  char EigenFileName[MAX_FILE_NAME_LENGTH];
  FILE * eigenf;

  GetDP_Begin("Lanczos");

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  Msg(BIGINFO, "Lanczos - December 2001 - beta 0.2 A. Nicolet - Marseille ");

  if(!DofData_P->Flag_Init[1] || !DofData_P->Flag_Init[3])
    Msg(ERROR, "No System available for Lanczos: check 'DtDt' and 'GenerateSeparate'") ;

  /* lecture des parametres dans le fichier 'eigen.par' */
  /* valeurs par defaut */
  eigenpar.prec = 1.e-4; 
  eigenpar.reortho = 0;

  eigenpar.size = LanSize; /* A CHANGER */
  /* dans le .pro il faudrait changer la syntaxe. Enlever LanSize et
     remplacer le mot clef Lanczos par Eigenproblem */
  
  /* construction du nom de fichier global */  
  strcpy(EigenFileName, Name_Path);
  strcat(EigenFileName, "eigen.par");
  Msg(INFO, "Loading eigenproblem parameter file '%s'", EigenFileName);
  
  /* lecture des parametres */
  eigenf=fopen(EigenFileName,"r+t");
  if (eigenf) { /* le fichier existe ! */
    fscanf(eigenf,"%lg ",&eigenpar.prec); 
    Msg(INFO, "eigenpar.prec = %g", eigenpar.prec );
    fscanf(eigenf,"%d ",&eigenpar.reortho);
    Msg(INFO, "eigenpar.reortho = %d", eigenpar.reortho );
    fscanf(eigenf,"%d ",&eigenpar.size);
    Msg(INFO, "eigenpar.size = %d", eigenpar.size );
    /* tester la fin du fichier avec un entier standard */
    fclose(eigenf);
  }

  /* reecriture des parametres */
  eigenf=fopen(EigenFileName,"w+t");
  fprintf(eigenf,"%g \n",eigenpar.prec);
  fprintf(eigenf,"%d \n",eigenpar.reortho);
  fprintf(eigenf,"%d \n",eigenpar.size);
  fprintf(eigenf,
	  "/*\n"
	  "   The numbers above are the parameters for the numerical\n"
	  "   eigenvalue problem:\n"
	  "\n"
	  "   prec = aimed accuracy for eigenvectors (default=1.e-4)\n"
	  "   reortho = reorthogonalisation of Krylov basis: yes=1, no=0 (default=0) \n"
	  "   size = number of iterations and max size of the Krylov basis\n"
	  "\n"
	  "   The shift is given in the .pro file because its choice relies\n"
	  "   on physical considerations...\n"
	  "*/");
  fclose(eigenf);

  /* LanSize = eigenpar.size */

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


  NbrDof = DofData_P->NbrDof ; /* taille du syst�me */

  Lan = (gVector*)Malloc((LanSize+1)*sizeof(gVector)) ;
  for (i=0 ; i<LanSize+1 ; i++) 
    LinAlg_CreateVector(&Lan[i], &DofData_P->Solver, NbrDof,
		  DofData_P->NbrPart, DofData_P->Part);
  /* r�servation de LanSize+1 vecteurs de taille NbrDof rep�r�s par
     les adresses &Lan[i] dans la suite &Lan[i] sera not� q_i dans les
     commentaires (colonnes d'une matrice rectangulaire orthogonale) */

  /* r�solution du PVP g�n�ralis� K v = valp M v */
  /* identifier les matrices avec des matrices du probl�me en cours */
  K = &DofData_P->M1 ; /* matrice des termes en DtDt */
  /*L = &DofData_P->M2 ;*/ /* matrice des termes en Dt pour le futur */
  M = &DofData_P->M3 ; /* matrice des autres termes  */
  b = &DofData_P->b  ; 
  x = &DofData_P->CurrentSolution->x ;


  /* Si on veut traiter le probl�me 'om�ga fix�' en propagation, les
     valeurs propres sont dans ce cas associ�es � la constante de
     propagation et donc � la d�rivation en z�! La notation Dt est donc
     mal choisie dans ce cas � */
  
  /* Cf �galement remarque sur les courbes de dispersion�! */
  
  /* On est ici dans un cas tr�s particulier o� tout est complexe sauf
     la matrice de Hessenberg et les valeurs propres. On fait Arnoldi
     car on construit une matrice de Hessenberg mais c'est en fait une
     matrice tridiagonale (-> beaucoup de calcul pour rien ! mais �a ne
     ralenti pas trop le calcul car ici c'est le NOMBRE DE RESOLUTIONS
     DU SYSTEME pour les it�rations inverses qui est couteux). Par
     contre on ne consid�re que la partie r�elle des produits scalaires
     de vecteurs et on construit une matrice de Hessenberg r�elle -> Ce
     n'est pas assez g�n�ral pour �tre du vrai Arnoldi sur une matrice
     quelconque !!!  */
  
  /* Construire une Hessenberg complexe !! */
  
  /* d�claration des espaces de travail */
  diag    = dvector(1, LanSize+1);
  wr      = dvector(1, LanSize+1);
  wi      = dvector(1, LanSize+1);
  err     = dvector(1, LanSize+1);
  IMatrix = dmatrix(1, LanSize+1, 1, LanSize+1);
  Hes     = dmatrix(1, LanSize+1, 1, LanSize+1);

  /* initial random vector b=q_o
     pas optimal pour la reproductibilit� des r�sultats ! ! !
     pourquoi ne pas essayer des 1 partout
     Arnoldi-Tchebychev consiste � am�liorer le vecteur de d�part
     d'Arnoldi � l'aide de Tchebychev */
  for (i=0 ; i<NbrDof ; i++) LinAlg_SetDoubleInVector(ran3(&mun), &Lan[0], i) ;

  /* shifting */
  if (fabs(shift) > 1.e-10)
    LinAlg_AddMatrixProdMatrixDouble(K, M, -shift, K) ; 
  /* K = K - shift * M */
  /* DANGER DANGER d�pend de la mise � l'�chelle de K et M */
  /* les routines LinAlg_* sont l'interface standard avec les
     'Solver Toolkits' du type SparsKit ou PETSC (voir LinAlg.h) */


  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  /* it�rations d' A R N O L D I */

  /* 
     Soit le probl�me au vp : A v = lambda v avec A matrice m * m et
     soit un vecteur arbitraire b (choix ????)  Kn la matrice m*n de
     Krylov donn�e par <b, Ab, A^2 b, ... A^(n-1)b> (espace de Krylov
     K_n(A,b) = espace g�n�r� par la combinaison lin�aire de ces
     vecteurs = lin(b, Ab, A^2 b, ... A^(n-1)b)) A l'�tape n des
     it�rations d'Arnoldi, la matrice m*n orthogonale Qn est le
     facteur de la d�composition QR de Kn = Qn Rn (m*n = (m*n) *
     (n*n)) La matrice de Hessenberg Hn correspond � la projection Hn
     = Qn^* A Qn (n*n = (n*m) * (m*m) * (m*n) ) (^* conjugu� hermitien
     ou adjoint - l'algorithme est valable pour les matrices COMPLEXES
     NON HERMITIENNES ) Les it�rations successives sont reli�es par A
     Qn = Qn+1 H'n o� H'n est la matrice (n+1)*n obtenue en compl�tant
     Hn avec l'�l�ment h_(n+1,n).

     La structure des it�rations est (A est une matrice, b,v des
     vecteurs colonnes et les q des vecteurs colonnes de Qn, les h
     sont les coefficients de Hn)

     b arbitraire, q1 = b/norme(b)
     POUR n=1,2,3,...
          |v = A q_n
          |POUR j = 1,n 
          |     |h_(j,n) = q_j^* v
          |     |v = v - h_(j,n) q_j
          |h_(n+1,n)=norme(v)               
          |q_(n+1) = v/h_(n+1,n)     

    (Que se passe-t'il si h_(n+1,n)=0 ? -> on a trouv� un sous espace
    invariant = un 'sous-espace propre')

    Les valeurs propres sont approxim�es par les valeurs propres de Hn
    dans le sens suivant : Une matrice A v�rifie son polyn�me
    caract�ristique P(z) soit P(A) = 0 soit norme(P(A) b) = 0 pour
    tout vecteur b Soit pn(z) le polyn�me caract�ristique de Hn
    (polyn�me d'Arnoldi) C'est le polyn�me de degr� n qui minimise
    norme(p(A) b) pour tout polyn�me p(z) de degr� n.  Remarque : le
    polyn�me d'Arnoldi id�al ou polyn�me de Tchebychev de A est le
    polynome p_Tcheb(z) de degr� n qui minimise norme(p(A)) (norme
    matricielle) Ce polyn�me ne d�pend pas d'un vecteur b arbitraire
    mais il n'existe pas d'algorithme efficace pour le calculer

    Au cours des it�rations on construit Hn et Qn : Hn = Qn* A Qn avec
    Qn* Qn = I et on r��crit A Qn = Qn+1 H'n comme A Qn = Qn Hn +
    h_(n+1,n) q_n+1 e^*_n (h_n+1,n prochain coefficient de Hn+1, q_n+1
    prochaine colonne de Qn+1, e_n base canonique de C^n)

    Si v est vecteur propre de Hn alors Hn v = lambda v.  On choisit
    Norme(v)=1.  v est un vecteur de Ritz et lambda une valeur de
    Ritz.  Ils constituent l'approximation au sens de Galerkine dans
    l'espace de Krylov K_n(A,b) du probl�me aux valeurs propres : <w,
    A v - lambda v> = 0 pour tout w dans K_n(A,b)

    Les coefficients de h sont les coefficients du d�veloppement de A
    dans la base orthogonale de K_n(A,b) constitu�es des colonnes de
    Qn
     
    Donc Qn* A Qn v = lambda v Malheureusement Qn n'est pas carr�e et
    ce n'est pas une relation de similitude.  N�anmoins si on poursuit
    le processus jusque n=m (si possible) Qm diagonalise A et on a une
    relation de similitude.  Dans le cas d'une relation de similitude
    si v est vecteur propre de Q* A Q alors Q* A Q v = lambda v et
    comme Q Q* = I on a A Q v = lambda Q v et Q v est donc vecteur
    propre de A.  Ici on a Qn* Qn = I(n*n) mais pas Qn Qn* = I(m*m) Qn
    Qn* est le projecteur orthogonal de l'espace C^m (dimension m) (=
    vecteurs quelconques de m complexes) sur l'espace de Krylov
    K_n(A,b) (dimension n).  On estime le vecteur propre de A par un
    'rel�vement' de v � l'aide de Qn�: (estimation du vecteur propre
    de A) = Qn v.  La multiplication par Qn est donc un peu
    l'�quivalent de l'interpolation.

    On obtient une approximation du r�sidu de la fa�on suivante On
    pose x = Qn v Norme(A x - lambda x)= Norme(A Qn v - lambda Qn v) =
    Norme(A Qn v - Qn Hn v). Norme((A Qn - Qn Hn) v) = Norme(h_(n+1,n)
    q_n+1 e^*_n v) =h_(n+1,n) v(n) car Norme(q_n+1)=1 =dernier coef de
    Hessenberg x derni�re composante du vecteur de Ritz qu'il suffit
    de comparer � lambda pour avoir une estimation de l'erreur
    relative !

    Probl�me g�n�ralis� : K v = valp M v -> utiliser le produit
    scalaire <M x, y> o� M joue le r�le de m�trique.  Arnoldi/Lanczos
    donne les plus grandes valeurs propres -> utiliser la matrice
    inverse K*-1 -> on aboutit � une r�solution de syst�me.

    A venir, les PVP du second ordre du type 
    (M1 valp^2 + M2 valp + M3) v = 0

    Il faudra �galement voir si on ne peut pas am�liorer le processus
    en utilisant un PRECONDITIONNEMENT pour les probl�mes aux VP
    (diff�rent de ceux pour la r�solution des syst�mes) Voir Y. Saad,
    Numerical Methods for Large Eigenvalue Problems disponible sur le
    Web un red�marrage implicite, voir doc ARPACK
      
    NOTATION du PROGRAMME
    Dans ce qui suit K et M portent ce nom
    Hes contient Hn ( Hes[i][j]=Hn_(i,j) )
    &Lan[i] est l'adresse de la colonne q_i
    Variable de boucle ext�rieure i � la place de n
                       int�rieure j � la place de i
    L'indice i commence � z�ro.
  */


  /* traitement sp�cial des premi�res it�rations */
  /* ------------------------------------------- */

  /* �tape 0 */
  LinAlg_ProdMatrixVector(M, &Lan[0], b); /* b = M q_o */
  LinAlg_ProdVectorVector(b, &Lan[0], &dum);/* dum = b^T q_o  avec  ^T = transposition */
  LinAlg_ProdVectorDouble(&Lan[0], 1./sqrt(dum), &Lan[0]); /* Lan[0] is built q_o = q_o/sqrt(dum) */

  Msg(BIGINFO, "Lanczos iteration 1/%d", LanSize);

  /* �tape 1 */
  LinAlg_ProdMatrixVector(M, &Lan[0], b); /* b = M * Lan[0] b = M q_o */   
  LinAlg_Solve(K, b, &DofData_P->Solver, &Lan[1]); /* Lan[1] = K^-1 * b q_1 = K^-1 b */  
  /* pas d'inversion explicite, on utilise le solver bien s�r ! -> on a calcul� K^-1 M q_o */  

  LinAlg_ProdVectorVector(b, &Lan[1], &dum1); /* alpha1 = Lan[0]^T * M * Lan[1] dum1 = b^T q_1*/  

  /* orthogonalization */
  LinAlg_AddVectorProdVectorDouble(&Lan[1], &Lan[0], -dum1, &Lan[1]); /* Lan[1] -= alpha1 * Lan[0] */
  /* q_1= q_1 - dum1 q_o */

  LinAlg_ProdMatrixVector(M, &Lan[1], b); /* b = M * Lan[1] in prevision */  
  /* b = M q_1 */
  LinAlg_ProdVectorVector(b, &Lan[1], &dum2); /* gama2 = beta1 = sqrt(Lan[1]^T * M * Lan[1]) */
  /* dum2 = b^T q_1 */

  dum2 = sqrt(dum2); 
  LinAlg_ProdVectorDouble(&Lan[1], 1./dum2, &Lan[1]); /* normation in the metric M: Lan[1] = q_1  is built */  
  LinAlg_ProdVectorDouble(b, 1./dum2, b); /* b = M * Lan[1] in prevision of next step   */

  /* d�but de construction de la matrice de Hessenberg pour l'approximation des vp */
  Hes[1][1] = dum1;
  Hes[2][1] = dum2;

  /* print_lanczos_to_file (1, NbrDof, Hes, Lan, shift, Name); */

  /* D�but de la boucle principale des it�rations d' A R N O L D I */
  /* ------------------------------------------------------------- */

  Restart = 2 ; /* � quoi �a sert ?  pour 'Arnoldi with restarting' cf Golub & Van Loan */
  for (i=Restart ; i<=LanSize ; i++){
    
    Msg(BIGINFO, "Lanczos iteration %d/%d", i, LanSize);

    LinAlg_Solve(K, b, &DofData_P->Solver, &Lan[i]);/* q_1 = K^-1 b  avec b d�j� multipli� par M */

    
    for (j=1 ; j<=i ; j++){
      LinAlg_ProdMatrixVector(M, &Lan[j-1], x);/* x = M q_(j-1) */
      LinAlg_ProdVectorVector(x, &Lan[i], &Hes[j][i]);/* h_(j,i) = x^T q_i */
      LinAlg_AddVectorProdVectorDouble(&Lan[i], &Lan[j-1], -Hes[j][i], &Lan[i]); /* orthogonalization */ 
      /* q_i = q_i - h_(j,i) q_(j-1) */
    }

    /* OPTIONNAL PART : re-orthogonalization DGKS */
    if (eigenpar.reortho == 1) {
      Msg(BIGINFO, " *** reorthogonalization *** ");
      for (j=1 ; j<=i ; j++) {
	LinAlg_ProdMatrixVector(M, &Lan[j-1], x);
	LinAlg_ProdVectorVector(x, &Lan[i], &dum);
	diag[j]=dum;
      }
      /* two separate loops so that &Lan[i] is not modified in the first one */
      for (j=1 ; j<=i ; j++) {
	LinAlg_AddVectorProdVectorDouble(&Lan[i], &Lan[j-1], -diag[j], &Lan[i]); /* reorthogonalization */ 
	Hes[j][i]=Hes[j][i]+diag[j];
	Msg(INFO, " hes %d = %g corrected by %g ",j,Hes[j][i],diag[j]);
      }
    }

    LinAlg_ProdMatrixVector(M, &Lan[i], x); /* x = M q_i */
    LinAlg_ProdVectorVector(x, &Lan[i], &dum); /* dum  = x^T q_i */
    Hes[i+1][i] = sqrt(dum); 
    LinAlg_ProdVectorDouble(&Lan[i], 1./Hes[i+1][i], &Lan[i]); /* q_i = q_i / h_(j,i)*/ 
    LinAlg_ProdMatrixVector(M, &Lan[i], b); /* b = M * Lan[i] in prevision of next step */
    
    /* eigen value computation of the current Hes matrix
       On compl�te la matrice de Hessenberg par des z�ros */
    for(ii=3 ; ii<=i ; ii++)
      for(jj=1 ; jj<=ii-2 ; jj++)
	Hes[ii][jj] = 0.0 ;
    
    for(ii=1 ; ii<=i ; ii++)
      for(jj=1 ; jj<=i ; jj++)
	IMatrix[ii][jj] = Hes[ii][jj] ;

    /* cette partie est pour info,
       elle n'est n�cessaire que pour afficher les VP
       on pourrait imaginer de ne pas la calculer chaque fois�!  */
    hqr(IMatrix, i, wr, wi) ;

    /* Algorithme QR pour une matrice de Hessenberg (from Numerical Recipes)
       Pour une MATRICE REELLE�?�?�?�?�?�?�?�?�?�?�?�?�?�?�?�?�?�?�? 
       Ecrire l'algorithme correspondant en complexe : d�composition LR
       cf version Algol dans Wilkinson */

    /* 
       print_valpr_to_file(i,wr,wi,shift,Name);
       print_lanczos_to_file (i,NbrDof,Hes,Lan,shift,Name); 
    */

    /*
    Msg(INFO, "Lanczos eigenvalue of the transformed problem ");
    for(k=1 ; k<=i ; k++)
      Msg(INFO, "Lanczos eigenvalue %d = %g +I %g",k, 
	  wr[k], wi[k]);

    Msg(INFO, "Lanczos eigenvalue of the original problem ");
    for(k=1 ; k<=i ; k++)
      Msg(INFO, "Lanczos eigenvalue %d = %g (%g) %g",k, 
	  shift+1.0/wr[k], sqrt(shift+1.0/wr[k])/TWO_PI, wi[k]);
    */

    /* ATTENTION shift+1.0/wr[k] ne donne la VP r�elle que si wr[i]
       est NEGLIGEABLE
       quid des VP complexes */

    /* store the real eigen values */
    for (k=1 ; k<=i ; k++)
      wi[k] = shift+1.0/wr[k];

    /* estimation d'erreur et test de convergence */
    Msg(BIGINFO, "------------------ hessenberg coeff %d = %g  ",i,Hes[i+1][i]);
    if (Hes[i+1][i] < 1e-20) 
      Msg(BIGINFO, " --- INVARIANT SUBSPACE FOUND ! --- ");
    
    /* search the largest eigenvalue */
    dum = 0. ; ivmax =1 ;
    for (k=1 ; k<=i ; k++)
      if (wr[k] > dum) {ivmax = k; dum = wr[k];};
    
    if (wr[ivmax] == 0.) Msg(BIGINFO," OOOPS !! - no positive eigen value ? - ");
    
    Msg(INFO, "Max eigenvalue = %g on %d ", dum, ivmax);  
    cal_vec_pr_T(Hes, i, wr[ivmax], diag); /* compute the corresponding eigenvector */
    Msg(INFO, "Last eigenvector component = %g ", diag[i]);  
    Msg(BIGINFO, " ******** Residual estimate = %g ",
	Hes[i+1][i] * diag[i] / wr[ivmax] );

  }

  /* fin des it�rations d' A R N O L D I */
  /* ----------------------------------- */

  Msg(INFO, "Final eigenvalue/eigenvector Computation");

  /* eigen value computation of the final
     Hes matrix Calcul final des valeurs propres = Euh ? D�j� fait en
     sortie de boucle principale, non ?? */

  for(ii=3 ; ii<=LanSize ; ii++)
    for(jj=1 ; jj<=ii-2 ; jj++)
      Hes[ii][jj] = 0.0 ;

  for(ii=1 ; ii<=LanSize ; ii++)
    for(jj=1 ; jj<=LanSize ; jj++)
      IMatrix[ii][jj] = Hes[ii][jj] ;

  hqr(IMatrix, LanSize, wr, wi);

  /* store the real eigen values */
  for (k=1 ; k<=LanSize ; k++) wi[k]=shift+1./wr[k]; 
  /* est-ce une bonne id�e de stocker les VP r�elles reconstitu�es
     dans la partie complexe ???? non�! */

  /* eigen vector computation of the final Hes matrix */
  /* Pour chacune des valeurs propres de Hn, calcul du vecteur propre
     � l'aide de la routine 'cal_vec_pr_T' */

  for(i=1 ; i<=LanSize ; i++){
    
    cal_vec_pr_T(Hes, LanSize, wr[i], diag);/* diag ne sert qu'ici, son nom est loufoque ! */
    
    /* estimation d'erreur et test de convergence */
    /* Msg(BIGINFO, "*********** estim %d = %g",i,Hes[LanSize+1][LanSize]*diag[LanSize]); */
    
    if (wr[i]>1e-20) 
      err[i] = Hes[LanSize+1][LanSize]*diag[LanSize]/wr[i];
    else
      err[i] = 1.e99; /* a changer! */

    /* copy the current eigen vector in IMatrix */
    for(j=1 ; j<=LanSize ; j++) IMatrix[j][i]=diag[j];
    
  }
  
  /* reconstruction of the global eigen vectors */
  
  for(i=0 ; i<List_Nbr(LanSave) ; i++){
    List_Read(LanSave, i, &ii) ;
    
    if(ii<1 || ii>LanSize){
      Msg(WARNING, "Eigenvalue index out of range") ;
      break ;
    }
    
    /* test de validite de la valeur propre */
    Msg(BIGINFO, "Eigenvalue %d = %g (f = %g)", ii, wi[ii], sqrt(wi[ii])/TWO_PI);

    /* if error smaller than required precision and non pathological eigenvalue ! */
    if ((err[ii] < eigenpar.prec ) && (wr[ii]>0.)) { 
      Msg(BIGINFO, "GOOD -> Eigenvalue %d = %g with error %g ", ii, wr[ii], err[ii]);
      
      /* if(i != 0){ */
      if (newsol != 0) {
	/* Msg(BIGINFO, "###########################  create new solution space"); */
	LinAlg_CreateVector(&Solution_S.x, &DofData_P->Solver, NbrDof,
			    DofData_P->NbrPart, DofData_P->Part);

	/* NbrPart,Part -> pour les multiprocesseurs */
	/* cr�ation d'un nouveau 'vecteur solution' associ� au pas de temps en cours */

	List_Add(DofData_P->Solutions, &Solution_S) ;
	/* List_Add�: manipulation des listes */
	
	DofData_P->CurrentSolution = (struct Solution*)
	List_Pointer(DofData_P->Solutions, List_Nbr(DofData_P->Solutions)-1) ;
      } 
      else 
	newsol = 1 ; /* peut-�tre pas � la bonne place ? */

      DofData_P->CurrentSolution->Time = wi[ii] ;
      /* Time = partie r�elle de la vp ? r�cup�ration possible dans le
         Post ? J'ai v�rifi� dans l'affichage de Gmsh et �a semble
         bien �tre le cas */
      
      DofData_P->CurrentSolution->TimeStep = ii ;
      DofData_P->CurrentSolution->TimeFunctionValues = NULL ;

      DofData_P->CurrentSolution->SolutionExist = 1 ;
      LinAlg_ZeroVector(&DofData_P->CurrentSolution->x) ;
      /* mise � z�ro du vecteur solution courant */

      for(k=0;k<NbrDof;k++){
	/* boucle de taille m = taille des matrices A,K,M du probl�me */
	/* calcul de la composant k du vecteur ii */
	
	for (j=1 ; j<=LanSize ; j++){
	  /* boucle de taille n = le nombre de vecteurs propres � estimer */
	  /* produit vecteur^T vecteur mais PAS avec un vecteur q */
	  /* on balaye Qn dans l'autre sens ! */
	  
	  LinAlg_GetDoubleInVector(&dum, &Lan[j-1], k) ;
	  /* dum = �l�ment k de q_(j-1) */
	  
	  LinAlg_AddDoubleInVector(IMatrix[j][ii]*dum, &DofData_P->CurrentSolution->x, k) ;
	  /* x_k = x_k + q_(k,j-1)* v_j */
	  /* on a multipli� le 'ii i�me' vecteur propre de Hn par Qn */  
	  /* v de taille n -> Qn v de taille (m*n) * n = m     */
	}
      }
      
      /* normation L infini : abs plus grand �l�ment mis a un */
      /* Msg(BIGINFO, "normation du vecteur propre %d  ",ii); */
      /* d�termination du maximum */
      dum = 0.; dum1 = 0.;
      for(k=0;k<NbrDof;k++){
	LinAlg_GetDoubleInVector(&dum,&DofData_P->CurrentSolution->x, k);
	if (fabs(dum)>dum1) dum1=dum;
      }
      /* division par le max */
      if (dum1 > 1.e-16) 
	LinAlg_ProdVectorDouble(&DofData_P->CurrentSolution->x,1./dum1,&DofData_P->CurrentSolution->x);
      
      /* estimation d'erreur et test de convergence */
      /* Msg(BIGINFO, "------------------ estim %d = %g  ",ii,Hes[ii+1][ii]); */
      
    } /* fin du test de validite */
    else{

      Msg(BIGINFO,"BAD! -> Eigenvalue %d = %g with error %g ", ii, wr[ii], err[ii]); 

    }
    
  }    
  
  /* c'est fini */
  
  /* tester newsol pour voir si au moins une solution a �t� trouv�e ! */
  
  /* desallocation */
  
  for (i=0 ; i<LanSize ; i++) LinAlg_DestroyVector(&Lan[i]);
  Free(Lan) ;
  
  free_dvector(diag, 1, LanSize);
  free_dvector(wr, 1, LanSize);
  free_dvector(wi, 1, LanSize);
  free_dvector(err, 1, LanSize);
  free_dmatrix(IMatrix, 1, LanSize, 1, LanSize);
  free_dmatrix(Hes, 1, LanSize, 1, LanSize);

  GetDP_End ;
}

