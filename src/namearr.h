#ifndef __NAMEARR_H__
#define __NAMEARR_H__
/************************************************************************
 * Статический объект для хранения именованных переменных, задаваемых
 * во время работы программы
 ************************************************************************
 */
//примечание: реализации не вынес в отдельный *.cpp файл из соображений лени
//и простоты компиляции одной строкой 'gcc main.cpp -Os -lm -Wall -o prog'
#include <string.h>
#include <stdlib.h>

#define NA_UNKNOWN	0
#define NA_INT		1
#define NA_FLOAT	2

//именованная переменная (long / double)
class NameVal{
private:
  union{
    long ival;
    double fval;
  };
  char valtype;
  char name[100];
  char SetName(const char _name[]){
    strcpy(name,_name);
    return 0;
  }
public:
  NameVal(){valtype=NA_UNKNOWN;}
  ~NameVal(){}
  void Delete(){valtype=NA_UNKNOWN;}
  char IsEmpty(){return (valtype==NA_UNKNOWN);}
  char HasName(const char _name[]){if(valtype==NA_UNKNOWN)return 0; return (strcmp(_name,name)==0);}
  void Set(const char _name[], long x){
    if(SetName(_name))return;
    ival = x;
    valtype = NA_INT;
  }
  void Set(const char _name[], double x){
    if(SetName(_name))return;
    fval = x;
    valtype = NA_FLOAT;
  }
  long GetInt(){
    if(valtype == NA_INT)return ival;
      else{printf("Wrong type variable [%s]\n",name); return fval;}
  }
  double GetFloat(){
    if(valtype == NA_FLOAT)return fval;
      else{printf("Wrong type variable [%s]\n",name); return ival;}
  }
  void Print(){
    if(name)printf("%s = ",name);else printf("?= ");
    switch(valtype){
      case NA_INT: printf("(i)%li\n",ival); break;
      case NA_FLOAT: printf("(f)%e\n",fval); break;
      default: printf("(?)?\n");
    }
  }
  void fprint(FILE *pf, const char str[]){
    switch(valtype){
      case NA_INT: fprintf(pf,"%s = %li %s",name,ival,str); break;
      case NA_FLOAT:fprintf(pf,"%s = %e %s",name,fval,str); break;
    }
  }
};

//хранилище именованных переменных
#define NACOUNTMAX 10
class NameArr{
private:
  static NameArr single;
  NameVal **arr;
  int count;
  NameArr(){single.arr = NULL; single.count=0;}
  ~NameArr(){
    if(single.arr){
      for(int i=0;i<single.count;i++)if(single.arr[i])delete single.arr[i];
      free(single.arr);
      single.arr = NULL;
    }
    single.count=0;
  }
  static void Resize(){
    NameVal **newarr = (NameVal**)realloc((void*)single.arr, sizeof(NameVal*)*(single.count+NACOUNTMAX));
    if(!newarr)return;
    single.arr = newarr;
    int i;
    for(i=single.count;i<single.count+NACOUNTMAX;i++)newarr[i]=new NameVal;
    single.count+=NACOUNTMAX;
  }
  static int Find(const char name[]){
    int i;
    for(i=0;i<single.count;i++){
      if(single.arr[i]->HasName(name))return i;
    }
    return -1;
  }
  static int Create(){
    int i;
    for(i=0;i<single.count;i++)if(single.arr[i]->IsEmpty())return i;
    i = single.count;
    Resize();
    if(single.count > i)return i; else return -1;
  }
public:
  static void Set(const char name[], long val){
    int pos = Find(name);
    if(pos<0)pos = Create();
    if(pos<0)return;
    single.arr[pos]->Set(name,val);
  }
  static void Set(const char name[], double val){
    int pos = Find(name);
    if(pos<0)pos = Create();
    if(pos<0)return;
    single.arr[pos]->Set(name,val);
  }
  static int GetInt(const char name[]){
    int pos = Find(name);
    if(pos<0){printf("Wrong name [%s]\n",name); return 0;}
    return single.arr[pos]->GetInt();
  }
  static double GetFloat(const char name[]){
    int pos = Find(name);
    if(pos<0){printf("Wrong name [%s]\n",name); return 0;}
    return single.arr[pos]->GetFloat();
  }
  static void Delete(const char name[]){
    int pos = Find(name);
    if(pos>=0)single.arr[pos]->Delete();
  }
  static void Print(){
    int i;
    for(i=0;i<single.count;i++)single.arr[i]->Print();
  }
  static void fprint(FILE *pf,const char name[], const char str[]){
    int pos = Find(name);
    if(pos >= 0)single.arr[pos]->fprint(pf,str);
  }
};
NameArr NameArr::single;

#endif
