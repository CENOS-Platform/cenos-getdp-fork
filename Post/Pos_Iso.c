/* $Id: Pos_Iso.c,v 1.2 2000-09-07 18:47:28 geuzaine Exp $ */

/* ------------------------------------------------------------------------ */
/*  I n t e r p o l a t e                                                   */
/* ------------------------------------------------------------------------ */

void Interpolate(double *X, double *Y, double *Z, 
		 double *Val, double V, int I1, int I2, 
		 double *XI, double *YI ,double *ZI){
  
  if(Val[I1] == Val[I2]){
    *XI = X[I1]; 
    *YI = Y[I1]; 
    *ZI = Z[I1]; 
  }
  else{
    *XI= (V - Val[I1])*(X[I2]-X[I1])/(Val[I2]-Val[I1]) + X[I1];
    *YI= (V - Val[I1])*(Y[I2]-Y[I1])/(Val[I2]-Val[I1]) + Y[I1];
    *ZI= (V - Val[I1])*(Z[I2]-Z[I1])/(Val[I2]-Val[I1]) + Z[I1];
  }
}

/* ------------------------------------------------------------------------ */
/*  C a l _ I s o                                                           */
/* ------------------------------------------------------------------------ */

void Cal_IsoTetrahedron(double *X, double *Y, double *Z, double *Val, 
			double V, double Vmin, double Vmax,
			double *Xp, double *Yp, double *Zp, int *nb){
  if(V != Vmax){
    *nb = 0;
    if((Val[0] > V && Val[1] <= V) || (Val[1] > V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,1,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[0] > V && Val[2] <= V) || (Val[2] > V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[0] > V && Val[3] <= V) || (Val[3] > V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,3,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[1] > V && Val[2] <= V) || (Val[2] > V && Val[1] <= V)){
      Interpolate(X,Y,Z,Val,V,1,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[1] > V && Val[3] <= V) || (Val[3] > V && Val[1] <= V)){
      Interpolate(X,Y,Z,Val,V,1,3,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[2] > V && Val[3] <= V) || (Val[3] > V && Val[2] <= V)){
      Interpolate(X,Y,Z,Val,V,2,3,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
  }
  else{
    *nb=0;
    if((Val[0] < V && Val[1] <= V) || (Val[1] < V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,1,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[0] < V && Val[2] <= V) || (Val[2] < V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[0] < V && Val[3] <= V) || (Val[3] < V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,3,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[1] < V && Val[2] <= V) || (Val[2] < V && Val[1] <= V)){
      Interpolate(X,Y,Z,Val,V,1,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[1] < V && Val[3] <= V) || (Val[3] < V && Val[1] <= V)){
      Interpolate(X,Y,Z,Val,V,1,3,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[2] < V && Val[3] <= V) || (Val[3] < V && Val[2] <= V)){
      Interpolate(X,Y,Z,Val,V,2,3,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
  }
}

void Cal_IsoTriangle1D(double *X, double *Y, double *Z, double *Val, 
		      double V, double Vmin, double Vmax,
		      double *Xp, double *Yp, double *Zp, int *nb){
  
  if(V != Vmax){
    *nb = 0;
    if((Val[0] > V && Val[1] <= V) || (Val[1] > V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,1,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[0] > V && Val[2] <= V) || (Val[2] > V && Val[0] <= V)){
      Interpolate(X,Y,Z,Val,V,0,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[1] > V && Val[2] <= V) || (Val[2] > V && Val[1] <= V)){
      Interpolate(X,Y,Z,Val,V,1,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
  }
  else{
    *nb = 0;
    if((Val[0] < V && Val[1] >= V) || (Val[1] < V && Val[0] >= V)){
      Interpolate(X,Y,Z,Val,V,0,1,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
    if((Val[0] < V && Val[2] >= V) || (Val[2] < V && Val[0] >= V)){
      Interpolate(X,Y,Z,Val,V,0,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }       
    if((Val[1] < V && Val[2] >= V) || (Val[2] < V && Val[1] >= V)){
      Interpolate(X,Y,Z,Val,V,1,2,&Xp[*nb],&Yp[*nb],&Zp[*nb]); (*nb)++;
    }
  }

}

void CalIsoTriangle2D(double *X, double *Y, double *Z, double *Val, 
		      double V1, double V2, double Vmin, double Vmax,
		      double *Xp2, double *Yp2, double *Zp2, int *Np2, double *Vp2){

  int     i, io[3],j,iot,Np,Fl;
  double  Xp[5],Yp[5],Zp[5],Vp[5];

  *Np2 = 0;

  for(i=0;i<3;i++) io[i] = i;

  for(i=0;i<2;i++){
    for(j=i+1;j<3;j++){
      if(Val[io[i]]>Val[io[j]]){
	iot = io[i];
	io[i] = io[j];
	io[j] = iot;
      }
    }
  }

  /* io[] contient un indexage des noeuds t.q. Val[io[i]] > Val[io[j]] si i > j */

  if(Val[io[0]] > V2) return;
  if(Val[io[2]] < V1) return;

  if(V1 <= Val[io[0]] && Val[io[2]] <= V2){
    memcpy(Vp2,Val,3*sizeof(double));
    memcpy(Xp2,X,3*sizeof(double)); 
    memcpy(Yp2,Y,3*sizeof(double)); 
    memcpy(Zp2,Z,3*sizeof(double)); 
    *Np2 = 3;
    return;
  }

  Np = 0;
  if(V1<=Val[io[0]]){
    Vp[Np] = Val[io[0]];
    Xp[Np] = X[io[0]]; 
    Yp[Np] = Y[io[0]]; 
    Zp[Np] = Z[io[0]]; 
    Np++; Fl = 1;
  }
  else if(Val[io[0]] < V1 && V1 <= Val[io[1]]){
    Vp[Np] = V1;
    Interpolate(X,Y,Z,Val,V1,io[0],io[2],&Xp[Np],&Yp[Np],&Zp[Np]); Np++; 
    Vp[Np] = V1;
    Interpolate(X,Y,Z,Val,V1,io[0],io[1],&Xp[Np],&Yp[Np],&Zp[Np]); Np++; Fl = 1;
  }
  else {
    Vp[Np] = V1;
    Interpolate(X,Y,Z,Val,V1,io[0],io[2],&Xp[Np],&Yp[Np],&Zp[Np]); Np++;
    Vp[Np] = V1;
    Interpolate(X,Y,Z,Val,V1,io[1],io[2],&Xp[Np],&Yp[Np],&Zp[Np]); Np++; Fl = 0;
  }    

  if(V2 == Val[io[0]]){
    return;
  }
  else if((Val[io[0]]<V2) && ( V2 < Val[io[1]])){
    Vp[Np] = V2;
    Interpolate(X,Y,Z,Val,V2,io[0],io[1],&Xp[Np],&Yp[Np],&Zp[Np]); Np++;
    Vp[Np] = V2;
    Interpolate(X,Y,Z,Val,V2,io[0],io[2],&Xp[Np],&Yp[Np],&Zp[Np]); Np++;
  }
  else if(V2 < Val[io[2]]){
    if(Fl){
      Vp[Np] = Val[io[1]];
      Xp[Np] = X[io[1]]; 
      Yp[Np] = Y[io[1]]; 
      Zp[Np] = Z[io[1]];
      Np++;
    }
    Vp[Np] = V2;
    Interpolate(X,Y,Z,Val,V2,io[1],io[2],&Xp[Np],&Yp[Np],&Zp[Np]); Np++;
    Vp[Np] = V2;
    Interpolate(X,Y,Z,Val,V2,io[0],io[2],&Xp[Np],&Yp[Np],&Zp[Np]); Np++;
  }
  else{
    if(Fl){
      Vp[Np] = Val[io[1]];
      Xp[Np] = X[io[1]];
      Yp[Np] = Y[io[1]]; 
      Zp[Np] = Z[io[1]];
      Np++;
    }
    Vp[Np] = Val[io[2]];
    Xp[Np] = X[io[2]]; 
    Yp[Np] = Y[io[2]]; 
    Zp[Np] = Z[io[2]];
    Np++;
  }

  Vp2[0] = Vp[0];
  Xp2[0] = Xp[0]; 
  Yp2[0] = Yp[0]; 
  Zp2[0] = Zp[0]; 
  *Np2 = 1;

  for(i=1;i<Np;i++){
    if((Xp[i] != Xp2[(*Np2)-1]) ||(Yp[i] != Yp2[(*Np2)-1]) ||(Zp[i] != Zp2[(*Np2)-1])){      
      Vp2[*Np2] = Vp[i];
      Xp2[*Np2] = Xp[i]; 
      Yp2[*Np2] = Yp[i]; 
      Zp2[*Np2] = Zp[i];
      (*Np2)++;
    }
  }

  if(Xp2[0] == Xp2[(*Np2)-1] && Yp2[0] == Yp2[(*Np2)-1] && Zp2[0] == Zp2[(*Np2)-1]){
    (*Np2)-- ;
  }

}

/*

  des qu'on a genere un postelement, on peut le passer a travers la moulinette
  "Iso". cette moulinette doit 
  1) cracher le nouvel element cree dans le fichier si Format GMSH
  2) remplir autant de listes que d'isos avec les nouveaux elts crees si format TABLE

*/
