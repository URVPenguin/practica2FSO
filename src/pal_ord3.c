#include "../include/memoria.h"
#include "../include/semafor.h"
#include "../include/winsuport2.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int num_args, char *args[]) {
  if (num_args < 18) {
    fprintf(stderr, "Error parametres incorrectes");
    exit(1);
  }

  int id_win = atoi(args[1]);
  int win_fil = atoi(args[2]);
  int win_col = atoi(args[3]);
  int ipo_pf = atoi(args[4]);
  int ipo_pc = atoi(args[5]);
  float v_pal = atof(args[6]);
  float pal_ret = atof(args[7]);
  float po_pf = atof(args[8]);
  int retard = atoi(args[9]);
  int id_moviments = atoi(args[10]);
  int id_continuar = atoi(args[11]);
  int id_tec = atoi(args[12]);
  int l_pal = atoi(args[13]);
  int index = atoi(args[14]);
  int id_sem_global = atoi(args[15]);
  int id_espai_premut = atoi(args[16]);
  int id_sem_mov = atoi(args[17]);

  int* p_win = map_mem(id_win); 
  int* p_continuar = map_mem(id_continuar);
  int* p_tec = map_mem(id_tec);
  int* p_moviments = map_mem(id_moviments);
  char* p_espai_premut = map_mem(id_espai_premut);

  win_set(p_win, win_fil, win_col);

  do { 
    win_retard(retard * pal_ret);
    
    if(*p_espai_premut)
      waitS(id_sem_global);

    int f_h = po_pf + v_pal; 
    if (f_h != ipo_pf)   
    {
      if (v_pal > 0.0) 
      {
        if (win_quincar(f_h + l_pal - 1, ipo_pc) ==
            ' ') 
        {
          win_escricar(ipo_pf, ipo_pc, ' ', NO_INV); 
          po_pf += v_pal;
          ipo_pf = po_pf; 
          win_escricar(ipo_pf + l_pal - 1, ipo_pc, '1' + index, INVERS); 
          if (*p_moviments > 0){
            waitS(id_sem_mov);
            (*p_moviments)--; 
            signalS(id_sem_mov);
          }
        } else 
          v_pal = -v_pal;
      } else 
      {
        if (win_quincar(f_h, ipo_pc) == ' ') 
        {
          win_escricar(ipo_pf + l_pal - 1, ipo_pc, ' ', NO_INV); 
          po_pf += v_pal;
          ipo_pf = po_pf;                            
          win_escricar(ipo_pf, ipo_pc, '1' + index, INVERS); 
          if (*p_moviments > 0) {
            waitS(id_sem_mov);
            (*p_moviments)--; 
            signalS(id_sem_mov);
          }
        } else 
          v_pal = -v_pal;
      }
    } else
      po_pf += v_pal; 

    if((*p_tec == TEC_ESPAI || *p_tec == TEC_RETURN) && !*p_espai_premut)
      signalS(id_sem_global);
  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 &&
           ((*p_moviments > 0) || *p_moviments == -1));

  return 0;
}