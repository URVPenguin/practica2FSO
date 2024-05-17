/*
Programa per controlar la paleta de l'ordinador mitjançant un proces fill

*/

#include "memoria.h"
#include "semafor.h"
#include "winsuport2.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int num_args, char *args[]) {
  int id_win, id_moviments, id_continuar, id_tec;
  int *p_ipo_pf, *p_ipo_pc, *p_v_pal, *p_pal_ret, *p_moviments, *p_continuar,
      *p_tec, *p_space_press;
  void *p_win;
  int f_h, retard, win_fil, win_col, ipo_pf, ipo_pc, l_pal;
  float v_pal, pal_ret, po_pf;

  if (num_args < 14) {
    fprintf(stderr, "proces: id_win_str win_rows_str win_cols_str ipo_pf_str ipo_pc_str, v_pal_str pal_ret_str po_pf_str retard_str id_moviment_str id_continuar_str id_tec_str l_pal_str");
    exit(1);
  }

  id_win = atoi(args[1]);
  win_fil = atoi(args[2]);
  win_col = atoi(args[3]);
  ipo_pf = atoi(args[4]);
  ipo_pc = atoi(args[5]);
  v_pal = atof(args[6]);
  pal_ret = atof(args[7]);
  po_pf = atof(args[8]);
  retard = atoi(args[9]);
  id_moviments = atoi(args[10]);
  id_continuar = atoi(args[11]);
  id_tec = atoi(args[12]);
  l_pal = atoi(args[13]);

  p_win = map_mem(id_win); /* obtenir adres. de mem. compartida */
  p_continuar = map_mem(id_continuar);
  p_tec = map_mem(id_tec);
  p_moviments = map_mem(id_moviments);

  win_set(p_win, win_fil, win_col);

  do { // Bucle principal de la paleta
    win_retard(retard);
    f_h = po_pf + v_pal; /* posicio hipotetica de la paleta */
    if (f_h != ipo_pf)   /* si pos. hipotetica no coincideix amb pos. actual */
    {
      if (v_pal > 0.0) /* verificar moviment cap avall */
      {
        if (win_quincar(f_h + l_pal - 1, ipo_pc) ==
            ' ') /* si no hi ha obstacle */
        {
          win_escricar(ipo_pf, ipo_pc, ' ', NO_INV); /* esborra primer bloc */
          po_pf += v_pal;
          ipo_pf = po_pf; /* actualitza posicio */
          win_escricar(ipo_pf + l_pal - 1, ipo_pc, '1',
                       INVERS); /* impr. ultim bloc */
          if (*p_moviments > 0)
            (*p_moviments)--; /* he fet un moviment de la paleta */
        } else /* si hi ha obstacle, canvia el sentit del moviment */
          v_pal = -v_pal;
      } else /* verificar moviment cap amunt */
      {
        if (win_quincar(f_h, ipo_pc) == ' ') /* si no hi ha obstacle */
        {
          win_escricar(ipo_pf + l_pal - 1, ipo_pc, ' ',
                       NO_INV); /* esbo. ultim bloc */
          po_pf += v_pal;
          ipo_pf = po_pf;                            /* actualitza posicio */
          win_escricar(ipo_pf, ipo_pc, '1', INVERS); /* impr. primer bloc */
          if (*p_moviments > 0)
            (*p_moviments)--; /* he fet un moviment de la paleta */
        } else /* si hi ha obstacle, canvia el sentit del moviment */
          v_pal = -v_pal;
      }
    } else
      po_pf += v_pal; /* actualitza posicio vertical real de la paleta */


  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 &&
           ((*p_moviments > 0) || *p_moviments == -1));

  return 0;
}