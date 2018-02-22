#ifndef __SI_H__
#define __SI_H__
/**********************************************************************
 * Функции перевода физических величин из текстового представления в числовое
 **********************************************************************
 */

#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

//сокращения десятичных степеней
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень
 */
double SI_decpwr(const char *s, char **_next){
  char *next = (char*)s;
  double pwr;
  if(next[0] == 'n'){ //нано
    pwr = 1e-9; next+=1;
  }else if(next[0] == 'u'){ //микро
    pwr = 1e-6; next++;
  }else if(next[0] == 'm'){ //милли
    pwr = 1e-3; next++;
  }else if(next[0] == 'c'){ //санти (разве что для сантиметров, и то редко)
    pwr = 1e-2; next++;
  }else if(next[0] == 'k'){ //кило
    pwr = 1e3; next++;
  }else if(next[0] == 'M'){ //мега
    pwr = 1e6; next++;
  }else if(next[0] == 'G'){ //гига
    pwr = 1e9; next++;
  }else{
    pwr=1;
  }
  if(_next)*_next = next;
  return pwr;
}

//степени, входящие в названия системных величин (пока только "грамм" - 10^-3 кг)
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень (0 - ошибка)
 */
double SI_units(const char *s, char **next){
  if(s[0] == 'g'){
    *next = (char*)(s+1); return 1e-3;
  }else if(s[0] == 'm'){
    *next = (char*)(s+1); return 1;
  }else if(s[0] == 's'){
    *next = (char*)(s+1); return 1;
  }else if(s[0] == 'K'){
    *next = (char*)(s+1); return 1;
  }else if(s[0] == 'C'){
    *next = (char*)(s+1); return 1;
  }else if(s[0] == 'N'){
    *next = (char*)(s+1); return 1;
  }else if(s[0] == 'V'){
    *next = (char*)(s+1); return 1;
  }
  *next = (char*)s; return 0;
}

//прочие, не входящие в Си единицы
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень (0 - ошибка)
 */
double Other_units(const char *s, char **next){
  if(s[0] == 'd' && s[1] == 'y' && s[2] == 'n'){ //дин
    *next = (char*)(s+3); return 1e-5;
  }else if(s[0] == 'h'){ //час
    *next = (char*)(s+1); return 3600;
  }
  *next = (char*)s; return 0;
}

//Перевод из строки в единицы измерения
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень
 */
double units(const char *s, char **next){
  double res;
  res = SI_units(s,next);
  if(res == 0)res = Other_units(s,next);
  return res;
}

//считывние степени размерности. Скажем, m2 (квадратный метр) считываем двойку
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень
 */
int unitpwr(const char *s, char **next){
  int res = strtol(s,next,10);
  //если числа нет, то степень равна 1
  if(res == 0)return 1; else return res;
}

//считывание размерностей со степенями, например m/s2 - метр на секунду в квадрате
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень
 */
double unit2pwr(const char *s, char **_next){
  double res,pwr=1;
  int upwr;
  char *next;
  if(s[0] == 0)return 0;
  //считываем масштабный множитель (если есть)
  res = SI_decpwr(s,&next);
  //считываем размерность
  res *= units(next,&next);
  //если не вышло - пробуем еще раз (может, пробел стоял)
  if(res == 0)res = units(s,&next);
  //если снова не вышло - пишем ошибку и выходим
  if(res == 0){
    fprintf(stderr,"E: Could not convert [%s] into a SI\n",s);
    *_next = (char*)s; return 0;
  }
  //считываем степень размерности
  upwr = unitpwr(next,&next);
  //если степень отрицательная - ищем обратные величины
  if(upwr < 0){
    res = 1/res; upwr = -upwr;
  }
  //возводим в нужную степень
  for(;upwr>0;upwr--){
    pwr *= res;
  }

  *_next = next;
  return pwr;
}

//считывание строки (вида 12.3mg, то есть без пробела между числом и единицей измерения)
//и перевод в систему Си. То есть 12.3mg -> 12.3*10^-6 [kg]
/* входные параметры
 *   s - входная строка
 *   _next - остальная часть входной строки
 * результат
 *   степень
 */
double SI_read(const char *s, char **_next=NULL){
  double res=1,pwr=1;
  char pwrinv=0;
  char *next = (char*)s;
  //перебираем символы в начале строки, игнорируем пробелы, переводы строки и знаки равенства
  while(isspace(next[0]) || next[0] == '=' || next[0] == '\n'){
    next++;
    if(next[0] == 0){
      printf("E: Could not convert [%s] into a SI\n",s);
      if(_next)*_next = (char*)s;
      return 0;
    }
  }
  //считываем величину
  res = strtod(next,&next);
  //считаем степени, входящие в размерность, например как в km/h2 - километров на час в квадрате, такой нестандартный способ записи ускорения
  while(pwr != 0){
    if(pwrinv)pwr = 1/pwr; //если уже встречался знак знаменателя, все степени после него будут инвертированы
    res *= pwr;//умножаем на последнюю успешно считанную степень (если считывание было провалено, будет 0 и цикл завершится)
    //отбрасываем лишние пробелы
    while(isspace(next[0])){
      next++;
      if(next[0] == 0){
        goto cycle_end; //если строка внезапно кончилась - выходим
      }
    }
    //знак знаменателя
    if(next[0] == '/'){
      pwrinv = 1; next++;
    }
    //пытаемся считать очередную степень
    pwr = unit2pwr(next,&next);
  }
  cycle_end:
  if(_next)*_next = next;
  return res;
}

#endif
