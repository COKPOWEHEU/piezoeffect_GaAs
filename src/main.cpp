#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "SI.h"
#include "namearr.h"

const double DMIN=1e-100;
const char constsfile[]="consts.in";
const double ElCharge=1.6e-19;
const double epsilon0=8.85e-12;
//расчет плотности пьезозаряда
/* входные параметры
 *   x - расстояние от середины зазора (м)
 *   z - глубина (м)
 * результат
 *   плотность пьезозаряда (Кл/м3)
 */
double piezo_conc(double x, double z){
  double x1,x2,r1,r2,w2;
  double w,gam,sigma,df,beta;
  w = NameArr::GetFloat("W");
  gam = NameArr::GetFloat("gam");
  sigma = NameArr::GetFloat("sigma");
  df = NameArr::GetFloat("df");
  beta = NameArr::GetFloat("beta");
  w2 = w/2;
  x1 = x + w2;
  x2 = x - w2;
  r1 = x1*x1 + z*z; r1 = r1*r1*r1;
  r2 = x2*x2 + z*z; r2 = r2*r2*r2;
  r1 = x1*z*(x1*x1 - beta*z*z)/r1;
  r2 = x2*z*(x2*x2 - beta*z*z)/r2;
  return gam*sigma*df*(r1-r2);
}
//удельное сопротивление участка проводящего объемного слоя
/* входные параметры
 *   x1, x2 - расстояния начала и конца участка от середины зазора (м)
 *   z - глубина (м)
 * результат
 *   сопротивление на единицу площади сечения (Ом*м2)
 */
double SdR(double x1, double x2, double z){
  double rho1, rho2;
  double rho0,mu;
  rho0 = NameArr::GetFloat("rho0");
  mu = NameArr::GetFloat("mu");
  rho1 = piezo_conc(x1,z) + rho0;
  rho2 = piezo_conc(x2,z) + rho0;
  if( rho1 < DMIN )return -1;
  if( rho2 < DMIN )return -2;
  rho1 = 0.5*(1/rho1 + 1/rho2);
  x1 = (x2-x1)/mu;
  return fabs(rho1*x1);
}
//потенциал в точке (относительно начала координат), вызванный одним зарядом
/* входные параметры
 *   x,z - координаты заряда
 *   dx,dz - размеры заряда
 *   px - координаты точки
 * результат
 *   потенципл
 */
double PotentialPoint(double x, double z, double dx, double dz, double px){
  double q1,q2,r,pr;
  double pz = NameArr::GetFloat("z");
  double cons= 1/(4*M_PI*epsilon0*NameArr::GetFloat("epsilon")); //пока будет 1
  dx /= 2; dz /= 2;
  r = sqrt(x*x+z*z);
  pr = sqrt(px*px+pz*pz);
  q1 = piezo_conc(x-dx,z-dz);
  q2 = piezo_conc(x+dx,z+dz);
  q1 = (q1+q2)*dx*dz*2; //множитель 2 потому что dx и dz были поделены на 2, то есть надо умножить на 4, но среднее арифметическое зарядов требует деления на 2
  if( r < DMIN ) return 0; //слишком близко расположенные точки не учитываем
  return q1*cons*log(pr/r);
}

//потенциал в точке, обусловленный всем объемом
/* входные параметры
 *   x - координаты искомой точки
 *   z - толщина объема
 *   Nx,Nz - количество точек по горизонтали и вертикали
 * результат
 *   потенциал
 */
float extprogress,dprogress;
double PotentialVolume(double _x, double zmax, unsigned long Nx, unsigned long Nz){
  double x,z,dx,dz,U,dU,xmax,_z;
  float progress;
  unsigned long i,maxi,j,maxj;
  static float oldprogress=1e10;
  maxi=Nx*Nz; i=0;
  if( Nx == 0 )return 0;
  if( Nz == 0 )return 0;
  U=0;
  xmax = NameArr::GetFloat("W");
  _z = NameArr::GetFloat("z");
  dx = xmax / Nx;
  dz = zmax / Nz;
  for(x=0; x<xmax; x+=dx){
    for(z=_z+dz; z<zmax;z+=dz){
      dU = PotentialPoint(x,z,dx,dz,_x);
      U += dU;
      i++;
      progress = i*dprogress; progress/=maxi;
      progress = extprogress+progress;      
      if( fabs(progress - oldprogress)>0.00001 ){
	oldprogress = progress;
        putchar('\r');
        putchar('[');
        maxj = progress*40/100;
        for(j=0;j<maxj;j++)putchar('#');
        for(;j<40;j++)putchar(' ');
        printf("]  %.10f %% ",progress);
      }
    }
  }
  //printf("\n");
  return U;
}

void CalcVars(){
  double d14,nu,n0;
  d14 = NameArr::GetFloat("d14");
  nu = NameArr::GetFloat("nu");
  n0 = NameArr::GetFloat("n0");
  NameArr::Set("gam",2*d14*(4-nu)/M_PI);
  NameArr::Set("beta",(2+nu)/(4-nu));
  NameArr::Set("rho0",n0*ElCharge);
}
void SaveConsts(){
  FILE *pf = fopen(constsfile,"wt");
  if(!pf)return;
  NameArr::fprint(pf,"sigma"," N/m2\n");
  NameArr::fprint(pf,"d14"," C/N\n");
  NameArr::fprint(pf,"mu"," m2/Vs\n");
  NameArr::fprint(pf,"nu","\n");
  NameArr::fprint(pf,"n0"," m-3\n");
  NameArr::fprint(pf,"dx"," m\n");
  NameArr::fprint(pf,"W"," m\n");
  NameArr::fprint(pf,"z"," m\n");
  
  fclose(pf);
}
void ChangeVar(const char *str){
  int i,len;
  char *name;
  double res;
  len = strlen(str);
  for(i=0; i<len;i++)if(isspace(str[i])||str[i]=='=')break;
  name = (char*)malloc(sizeof(char)*(i+2));
  if(!name)return;
  strncpy(name,str,i);
  res = SI_read(&str[i]);
  NameArr::Set(name,res);
  free(name);
}
void LoadConsts(){
  char temp[100];
  FILE *pf = fopen(constsfile,"rt");
  if(!pf)return;
  while(!feof(pf)){
    fgets(temp,99,pf);
    ChangeVar(temp);
  }
  fclose(pf);
  CalcVars();
}
double ChargeVert(double x, double dx, double zmax, unsigned long N){
  double z1,z2, res=0, q1,q2,rx,rz;
  unsigned long i,dN;
  dN = 1+N/1000;
  for(i=0;i<N-1;i++){
    z1 = zmax*dN/(i+dN);
    z2 = zmax*dN/(i+dN+1);
    q1 = piezo_conc(x-dx/2,z1);
    q2 = piezo_conc(x+dx/2,z2);
    res += (q1+q2)*0.5*dx*(z1-z2);
    if(z1>0)rx = log10(z1);else rx=log10(-z1);
    if(q1>0)rz = log10(res);else rz=log10(-res);
//    printf("%e\t%e\n",z1,rz);
  }
  return res;
}

#ifndef LOG_SCALE
double func(double x){
  return x;
}
#else
double func(double x){
  return log10(fabs(x));
}
#endif

int main(int argc, char **argv){
  char filename[20];
  unsigned long Nx=1000,Nz=1000;
  LoadConsts();
  
  if(argc < 2){
    printf("Use \"%s width [Nx Nz]\"\n%s 20um 100 100 for example\n",argv[0],argv[0]);
    return 0;
  }
  NameArr::Set("W",SI_read(argv[1]));
  sprintf(filename,"%s.txt",argv[1]);
  if(argc > 3){
    Nx = atoi(argv[2]);
    Nz = atoi(argv[3]);
  }
  
  double z,U,dx,dz,minz,maxz,ddz;
  unsigned long i,maxi;
  float progress;
  dx=1e-10; dz=1e-10;
  minz=1e-9; maxz=1; ddz=1.1;
  progress = log(maxz/minz)/log(ddz);
  i=0; maxi=progress;
  dprogress = 1/progress;
  FILE *pf = fopen(filename,"wt");
  if(!pf)return 0;  
  for(z=minz;z<maxz;z*=ddz){
    //U = PotentialPoint(0,z,dx,dz,0);
    extprogress = i*100; extprogress/=maxi;
    i++;    
    //printf("%.1f\n",progress);
    U = PotentialVolume(0,z,Nx,Nz);
    fprintf(pf,"%g\t%g\n",log10(z),func(U));
  }
  fclose(pf);
}
