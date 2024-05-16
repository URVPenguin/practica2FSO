/*****************************************************************************/
/*									     */
/*				     Tennis0.c				     */
/*									     */
/*  Programa inicial d'exemple per a les practiques 2 i 3 de FSO.	     */
/*     Es tracta del joc del tennis: es dibuixa un camp de joc rectangular   */
/*     amb una porteria a cada costat, una paleta per l'usuari, una paleta   */
/*     per l'ordinador i una pilota que va rebotant per tot arreu; l'usuari  */
/*     disposa de dues tecles per controlar la seva paleta, mentre que l'or- */
/*     dinador mou la seva automaticament (amunt i avall). Evidentment, es   */
/*     tracta d'intentar col.locar la pilota a la porteria de l'ordinador    */
/*     (porteria de la dreta), abans que l'ordinador aconseguixi col.locar   */
/*     la pilota dins la porteria de l'usuari (porteria de l'esquerra).      */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil n_col m_por l_pal					     */
/*		pil_pf pil_pc pil_vf pil_vc pil_ret			     */
/*		ipo_pf ipo_pc po_vf pal_ret				     */
/*									     */
/*     on 'n_fil', 'n_col' son les dimensions del taulell de joc, 'm_por'    */
/*     es la mida de les dues porteries, 'l_pal' es la longitud de les dues  */
/*     paletes; 'pil_pf', 'pil_pc' es la posicio inicial (fila,columna) de   */
/*     la pilota, mentre que 'pil_vf', 'pil_vc' es la velocitat inicial,     */
/*     pil_ret es el percentatge respecte al retard passat per paràmetre;    */
/*     finalment, 'ipo_pf', 'ipo_pc' indicara la posicio del primer caracter */
/*     de la paleta de l'ordinador, mentre que la seva velocitat vertical    */
/*     ve determinada pel parametre 'po_fv', i pal_ret el percentatge de     */
/*     retard en el moviment de la paleta de l'ordinador.		     */
/*									     */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment de la pilota i la paleta de l'ordinador (en ms);   */
/*     el valor d'aquest parametre per defecte es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides en 'winsuport.o', les       */
/*     quals proporcionen una interficie senzilla per a crear una finestra   */
/*     de text on es poden imprimir caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc tennis0.c winsuport.o -o tennis0 -lcurses		     */
/*	   $ tennis0 fit_param [retard]					     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  fitxer no accessible					     */
/*	3  ==>  dimensions del taulell incorrectes			     */
/*	4  ==>  parametres de la pilota incorrectes			     */
/*	5  ==>  parametres d'alguna de les paletes incorrectes		     */
/*	6  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*****************************************************************************/

#include "memoria.h"
#include "winsuport2.h" /* incloure definicions de funcions propies */
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> /* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define MIN_FIL 7 /* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MIN_PAL 3
#define MIN_VEL -1.0
#define MAX_VEL 1.0
#define MIN_RET 0.0
#define MAX_RET 5.0
#define MAX_PAL_ORD 9
#define MAX_THREADS 3

/* variables globals */
int n_fil, n_col, m_por;    /* dimensions del taulell i porteries */
int l_pal;                  /* longitud de les paletes */
float v_pal[MAX_PAL_ORD];   /* velocitat de la paleta del programa */
float pal_ret[MAX_PAL_ORD]; /* percentatge de retard de la paleta */

int ipu_pf, ipu_pc; /* posicio del la paleta d'usuari */
int ipo_pf[MAX_PAL_ORD],
    ipo_pc[MAX_PAL_ORD];  /* posicio del la paleta de l'ordinador */
float po_pf[MAX_PAL_ORD]; /* pos. vertical de la paleta de l'ordinador, en valor
                             real */
int n_po;

int ipil_pf, ipil_pc; /* posicio de la pilota, en valor enter */
float pil_pf, pil_pc; /* posicio de la pilota, en valor real */
float pil_vf, pil_vc; /* velocitat de la pilota, en valor real*/
float pil_ret;        /* percentatge de retard de la pilota */

int retard; /* valor del retard de moviment, en mil.lisegons */

pthread_t tid[MAX_THREADS];
pid_t tpid[MAX_PAL_ORD];

int id_moviments, id_continuar, id_tec, id_win;
int *p_moviments, *p_continuar, *p_tec;
void *p_win;

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins un fitxer de text, el nom del qual es passa per referencia en   */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris del principi del programa).		*/
void carrega_parametres(const char *nom_fit) {
  FILE *fit;

  fit = fopen(nom_fit, "rt"); /* intenta obrir fitxer */
  if (fit == NULL) {
    fprintf(stderr, "No s'ha pogut obrir el fitxer \'%s\'\n", nom_fit);
    exit(2);
  }

  if (!feof(fit))
    fscanf(fit, "%d %d %d %d\n", &n_fil, &n_col, &m_por, &l_pal);
  if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) || (n_col < MIN_COL) ||
      (n_col > MAX_COL) || (m_por < 0) || (m_por > n_fil - 3) ||
      (l_pal < MIN_PAL) || (l_pal > n_fil - 3)) {
    fprintf(stderr, "Error: dimensions del camp de joc incorrectes:\n");
    fprintf(stderr, "\t%d =< n_fil (%d) =< %d\n", MIN_FIL, n_fil, MAX_FIL);
    fprintf(stderr, "\t%d =< n_col (%d) =< %d\n", MIN_COL, n_col, MAX_COL);
    fprintf(stderr, "\t0 =< m_por (%d) =< n_fil-3 (%d)\n", m_por, (n_fil - 3));
    fprintf(stderr, "\t%d =< l_pal (%d) =< n_fil-3 (%d)\n", MIN_PAL, l_pal,
            (n_fil - 3));
    fclose(fit);
    exit(3);
  }

  if (!feof(fit))
    fscanf(fit, "%d %d %f %f %f\n", &ipil_pf, &ipil_pc, &pil_vf, &pil_vc,
           &pil_ret);
  if ((ipil_pf < 1) || (ipil_pf > n_fil - 3) || (ipil_pc < 1) ||
      (ipil_pc > n_col - 2) || (pil_vf < MIN_VEL) || (pil_vf > MAX_VEL) ||
      (pil_vc < MIN_VEL) || (pil_vc > MAX_VEL) || (pil_ret < MIN_RET) ||
      (pil_ret > MAX_RET)) {
    fprintf(stderr, "Error: parametre pilota incorrectes:\n");
    fprintf(stderr, "\t1 =< ipil_pf (%d) =< n_fil-3 (%d)\n", ipil_pf,
            (n_fil - 3));
    fprintf(stderr, "\t1 =< ipil_pc (%d) =< n_col-2 (%d)\n", ipil_pc,
            (n_col - 2));
    fprintf(stderr, "\t%.1f =< pil_vf (%.1f) =< %.1f\n", MIN_VEL, pil_vf,
            MAX_VEL);
    fprintf(stderr, "\t%.1f =< pil_vc (%.1f) =< %.1f\n", MIN_VEL, pil_vc,
            MAX_VEL);
    fprintf(stderr, "\t%.1f =< pil_ret (%.1f) =< %.1f\n", MIN_RET, pil_ret,
            MAX_RET);
    fclose(fit);
    exit(4);
  }

  n_po = 0;
  while (!feof(fit) && n_po < MAX_PAL_ORD) {
    fscanf(fit, "%d %d %f %f\n", &ipo_pf[n_po], &ipo_pc[n_po], &v_pal[n_po],
           &pal_ret[n_po]);

    if ((ipo_pf[n_po] < 1) || (ipo_pf[n_po] + l_pal > n_fil - 2) ||
        (ipo_pc[n_po] < 5) || (ipo_pc[n_po] > n_col - 2) ||
        (v_pal[n_po] < MIN_VEL) || (v_pal[n_po] > MAX_VEL) ||
        (pal_ret[n_po] < MIN_RET) || (pal_ret[n_po] > MAX_RET)) {
      fprintf(stderr, "Error: parametres paleta ordinador incorrectes:\n");
      fprintf(stderr, "\t1 =< ipo_pf (%d) =< n_fil-l_pal-3 (%d)\n",
              ipo_pf[n_po], (n_fil - l_pal - 3));
      fprintf(stderr, "\t5 =< ipo_pc (%d) =< n_col-2 (%d)\n", ipo_pc[n_po],
              (n_col - 2));
      fprintf(stderr, "\t%.1f =< v_pal (%.1f) =< %.1f\n", MIN_VEL, v_pal[n_po],
              MAX_VEL);
      fprintf(stderr, "\t%.1f =< pal_ret (%.1f) =< %.1f\n", MIN_RET,
              pal_ret[n_po], MAX_RET);
      fclose(fit);
      exit(5);
    }

    n_po++;
  }

  fclose(fit); /* fitxer carregat: tot OK! */
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
int inicialitza_joc(void) {
  int i, i_port, f_port, retwin;
  char strin[51];

  retwin = win_ini(&n_fil, &n_col, '+', INVERS); /* intenta crear taulell */
  id_win = ini_mem(retwin);     /* crear zona mem. compartida */
  p_win = map_mem(id_win);      /* obtenir adres. de mem. compartida */
  win_set(p_win, n_fil, n_col); /* crea acces a finestra oberta */

  if (retwin < 0) /* si no pot crear l'entorn de joc amb les curses */
  {
    fprintf(stderr, "Error en la creacio del taulell de joc:\t");
    switch (retwin) {
    case -1:
      fprintf(stderr, "camp de joc ja creat!\n");
      break;
    case -2:
      fprintf(stderr, "no s'ha pogut inicialitzar l'entorn de curses!\n");
      break;
    case -3:
      fprintf(stderr, "les mides del camp demanades son massa grans!\n");
      break;
    case -4:
      fprintf(stderr, "no s'ha pogut crear la finestra!\n");
      break;
    }
    return (retwin);
  }

  i_port = n_fil / 2 - m_por / 2; /* crea els forats de la porteria */
  if (n_fil % 2 == 0)
    i_port--;
  if (i_port == 0)
    i_port = 1;
  f_port = i_port + m_por - 1;
  for (i = i_port; i <= f_port; i++) {
    win_escricar(i, 0, ' ', NO_INV);
    win_escricar(i, n_col - 1, ' ', NO_INV);
  }

  ipu_pf = n_fil / 2;
  ipu_pc = 3; /* inicialitzar pos. paletes */
  if (ipu_pf + l_pal >= n_fil - 3)
    ipu_pf = 1;
  for (i = 0; i < l_pal; i++) /* dibuixar paleta inicialment */
  {
    win_escricar(ipu_pf + i, ipu_pc, '0', INVERS);
    for (int j = 0; j < n_po; j++)
      win_escricar(ipo_pf[j] + i, ipo_pc[j], '1', INVERS);
  }

  for (int i = 0; i < n_po; i++)
    po_pf[i] = ipo_pf[i]; /* fixar valor real paleta ordinador */

  pil_pf = ipil_pf;
  pil_pc = ipil_pc; /* fixar valor real posicio pilota */
  win_escricar(ipil_pf, ipil_pc, '.', INVERS); /* dibuix inicial pilota */

  sprintf(strin, "Tecles: \'%c\'-> amunt, \'%c\'-> avall, RETURN-> sortir.",
          TEC_AMUNT, TEC_AVALL);
  win_escristr(strin);
  return (0);
}

/* funcio per moure la pilota; retorna un valor amb alguna d'aquestes	*/
/* possibilitats:							*/
/*	-1 ==> la pilota no ha sortit del taulell			*/
/*	 0 ==> la pilota ha sortit per la porteria esquerra		*/
/*	>0 ==> la pilota ha sortit per la porteria dreta		*/
void *moure_pilota(void *cap) {
  int f_h, c_h;
  char rh, rv, rd, pd;

  do {
    win_retard(retard);

    f_h = pil_pf + pil_vf; /* posicio hipotetica de la pilota */
    c_h = pil_pc + pil_vc;
    *p_continuar = -1; /* inicialment suposem que la pilota no surt */
    rh = rv = rd = pd = ' ';
    if ((f_h != ipil_pf) || (c_h != ipil_pc)) {
      /* si posicio hipotetica no coincideix amb la pos. actual */
      if (f_h != ipil_pf) /* provar rebot vertical */
      {
        rv = win_quincar(f_h, ipil_pc); /* veure si hi ha algun obstacle */
        if (rv != ' ')                  /* si no hi ha res */
        {
          pil_vf = -pil_vf;      /* canvia velocitat vertical */
          f_h = pil_pf + pil_vf; /* actualitza posicio hipotetica */
        }
      }
      if (c_h != ipil_pc) /* provar rebot horitzontal */
      {
        rh = win_quincar(ipil_pf, c_h); /* veure si hi ha algun obstacle */
        if (rh != ' ')                  /* si no hi ha res */
        {
          pil_vc = -pil_vc;      /* canvia velocitat horitzontal */
          c_h = pil_pc + pil_vc; /* actualitza posicio hipotetica */
        }
      }
      if ((f_h != ipil_pf) && (c_h != ipil_pc)) /* provar rebot diagonal */
      {
        rd = win_quincar(f_h, c_h);
        if (rd != ' ') /* si no hi ha obstacle */
        {
          pil_vf = -pil_vf;
          pil_vc = -pil_vc; /* canvia velocitats */
          f_h = pil_pf + pil_vf;
          c_h = pil_pc + pil_vc; /* actualitza posicio entera */
        }
      }

      if (win_quincar(f_h, c_h) == ' ') /* verificar posicio definitiva */
      {                                 /* si no hi ha obstacle */
        win_escricar(ipil_pf, ipil_pc, ' ', NO_INV); /* esborra pilota */

        pil_pf += pil_vf;
        pil_pc += pil_vc;
        ipil_pf = f_h;
        ipil_pc = c_h; /* actualitza posicio actual */

        if ((ipil_pc > 0) && (ipil_pc <= n_col)) {
          win_escricar(ipil_pf, ipil_pc, '.', INVERS); /* imprimeix pilota */
        }                                              /* si no surt */
        else {
          *p_continuar = ipil_pc; /* codi de finalitzacio de partida */
        }
      }
    } else {
      pil_pf += pil_vf;
      pil_pc += pil_vc;
    }
  } while ((*p_tec != TEC_RETURN) && (*p_continuar == -1) &&
           ((*p_moviments > 0) || *p_moviments == -1));

  return ((void *)(intptr_t)0);
}

/* funcio per moure la paleta de l'usuari en funcio de la tecla premuda */
void *mou_paleta_usuari(void *cap) {
  do {
    win_retard(retard);

    if (*p_tec != 0) {
      if ((*p_tec == TEC_AVALL) &&
          (win_quincar(ipu_pf + l_pal, ipu_pc) == ' ')) {
        win_escricar(ipu_pf, ipu_pc, ' ', NO_INV); /* esborra primer bloc */

        ipu_pf++; /* actualitza posicio */

        win_escricar(ipu_pf + l_pal - 1, ipu_pc, '0',
                     INVERS); /* impri. ultim bloc */

        if (*p_moviments > 0) {
          (*p_moviments)--; // he fet un moviment de la paleta
        }
      }

      if ((*p_tec == TEC_AMUNT) && (win_quincar(ipu_pf - 1, ipu_pc) == ' ')) {
        win_escricar(ipu_pf + l_pal - 1, ipu_pc, ' ',
                     NO_INV); /* esborra ultim bloc */

        ipu_pf--;

        win_escricar(ipu_pf, ipu_pc, '0', INVERS); /* imprimeix primer bloc */

        if (*p_moviments > 0) {
          (*p_moviments)--; // he fet un moviment de la paleta
        }
      }
    }

  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 &&
           ((*p_moviments > 0) || *p_moviments == -1));

  return ((void *)(intptr_t)0);
}

void *read_key(void *arg) {
  do {
    win_retard(retard);
    *p_tec = win_gettec();
  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 &&
           ((*p_moviments > 0) || *p_moviments == -1));

  return ((void *)(intptr_t)0);
}

void init_shared_vars() {
  id_moviments = ini_mem(sizeof(int));
  p_moviments = map_mem(id_moviments);

  id_continuar = ini_mem(sizeof(int));
  p_continuar = map_mem(id_continuar);

  id_tec = ini_mem(sizeof(int));
  p_tec = map_mem(id_tec);
}

/* programa principal				    */
int main(int n_args, const char *ll_args[]) {
  int n_thr = 0, n = 0;
  char id_win_str[20], win_rows_str[20], win_cols_str[20], ipo_pf_str[20],
      ipo_pc_str[20], v_pal_str[20], pal_ret_str[20], po_pf_str[20],
      retard_str[20], id_moviment_str[20], id_continuar_str[20], id_tec_str[20],
      l_pal_str[20];

  if ((n_args != 3) && (n_args != 4)) {
    fprintf(stderr, "Comanda: tennis3 fit_param moviments [retard]\n");
    exit(1);
  }

  init_shared_vars();
  carrega_parametres(ll_args[1]);

  *p_moviments = atoi(ll_args[2]);
  *p_continuar = -1;

  id_tec = ini_mem(sizeof(int));

  if (*p_moviments == 0)
    *p_moviments = -1;

  if (n_args == 4)
    retard = atoi(ll_args[3]);
  else
    retard = 100;

  if (inicialitza_joc() != 0) /* intenta crear el taulell de joc */
    exit(4);                  /* aborta si hi ha algun problema amb taulell */

  if (pthread_create(&tid[n_thr], NULL, read_key, NULL) == 0)
    n_thr++;

  if (pthread_create(&tid[n_thr], NULL, moure_pilota, NULL) == 0)
    n_thr++;

  if (pthread_create(&tid[n_thr], NULL, mou_paleta_usuari, NULL) == 0)
    n_thr++;

  sprintf(id_win_str, "%d", id_win);
  sprintf(win_rows_str, "%d", n_fil);
  sprintf(win_cols_str, "%d", n_col);
  sprintf(l_pal_str, "%d", l_pal);
  sprintf(id_moviment_str, "%d", id_moviments);
  sprintf(id_tec_str, "%d", id_tec);
  sprintf(id_continuar_str, "%d", id_continuar);
  sprintf(retard_str, "%d", retard);

  for (int i = 0; i < n_po; i++) {
    sprintf(ipo_pf_str, "%d", ipo_pf[i]);
    sprintf(ipo_pc_str, "%d", ipo_pc[i]);
    sprintf(v_pal_str, "%f", v_pal[i]);
    sprintf(pal_ret_str, "%f", pal_ret[i]);
    sprintf(po_pf_str, "%f", po_pf[i]);

    tpid[n] = fork(); // Guardem la id dels processos creats a l'array tpid[]
    if (tpid[n] == (pid_t)0) {
      execlp("./pal_ord3", "pal_ord3", id_win_str, win_rows_str, win_cols_str,
             ipo_pf_str, ipo_pc_str, v_pal_str, pal_ret_str, po_pf_str,
             retard_str, id_moviment_str, id_continuar_str, id_tec_str,
             l_pal_str, (char *)0);
      fprintf(stderr, "No s'ha pogut executar el programa pal_ord3\n");
      exit(1);
    } else if (tpid[n] > (pid_t)0) {
      n++;
    }
  }

  char time[6], minutes = 0, seconds = 0;
  unsigned int ms = 0;
  do {
    sprintf(time, "%02u:%02u", minutes, seconds);

    win_escristr(time);

    if (ms >= 1000) {
      seconds++;
      ms = 0;
    }

    if (seconds >= 60) {
      seconds = 0;
      minutes++;
    }

    win_update();
    win_retard(50);
    ms += 50;
  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 &&
           ((*p_moviments > 0) || *p_moviments == -1));

  for (int i = 0; i < n_thr; i++) {
    pthread_join(tid[i], NULL);
  }

  // Espera que tots els processos acabin
  for (int i = 0; i < n; i++) {
    waitpid(tpid[i], NULL, 0);
  }

  win_fi();

  printf("Temps = %s\n", time);
  if (*p_tec == TEC_RETURN)
    printf("S'ha aturat el joc amb la tecla RETURN!\n");
  else {
    if (*p_continuar == 0 || *p_moviments == 0)
      printf("Ha guanyat l'ordinador!\n");
    else
      printf("Ha guanyat l'usuari!\n");
  }

  elim_mem(id_win);
  elim_mem(id_tec);
  elim_mem(id_continuar);
  elim_mem(id_moviments);

  return (0);
}
