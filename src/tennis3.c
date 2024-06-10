#include "../include/memoria.h"
#include "../include/semafor.h"
#include "../include/winsuport2.h" 
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MIN_FIL 7 
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MIN_PAL 3
#define MIN_VEL -1.0
#define MAX_VEL 1.0
#define MIN_RET 0.0
#define MAX_RET 5.0
#define MAX_PAL_ORD 9
#define MAX_THREADS 2

int n_fil, n_col, m_por;    
int l_pal;                  
float v_pal[MAX_PAL_ORD];   
float pal_ret[MAX_PAL_ORD]; 

int ipu_pf, ipu_pc; 
int ipo_pf[MAX_PAL_ORD],
    ipo_pc[MAX_PAL_ORD];  
float po_pf[MAX_PAL_ORD]; 
int n_paletes_ord;

int ipil_pf, ipil_pc; 
float pil_pf, pil_pc; 
float pil_vf, pil_vc; 
float pil_ret;        

int retard; 

pid_t tpid[MAX_PAL_ORD];
pthread_t tid[MAX_THREADS];

void *p_win;
int *p_moviments, *p_continuar, *p_tec, *p_espai_premut;
int id_moviments, id_continuar, id_tec, id_win,  id_espai_premut;
int id_sem_global, id_sem_mov; 

void carrega_parametres(const char *nom_fit) {
  FILE *fit;

  fit = fopen(nom_fit, "rt"); 
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

  n_paletes_ord = 0;
  while (!feof(fit) && n_paletes_ord < MAX_PAL_ORD) {
    fscanf(fit, "%d %d %f %f\n", &ipo_pf[n_paletes_ord], &ipo_pc[n_paletes_ord], &v_pal[n_paletes_ord],
           &pal_ret[n_paletes_ord]);

    if ((ipo_pf[n_paletes_ord] < 1) || (ipo_pf[n_paletes_ord] + l_pal > n_fil - 2) ||
        (ipo_pc[n_paletes_ord] < 5) || (ipo_pc[n_paletes_ord] > n_col - 2) ||
        (v_pal[n_paletes_ord] < MIN_VEL) || (v_pal[n_paletes_ord] > MAX_VEL) ||
        (pal_ret[n_paletes_ord] < MIN_RET) || (pal_ret[n_paletes_ord] > MAX_RET)) {
      fprintf(stderr, "Error: parametres paleta ordinador incorrectes:\n");
      fprintf(stderr, "\t1 =< ipo_pf (%d) =< n_fil-l_pal-3 (%d)\n",
              ipo_pf[n_paletes_ord], (n_fil - l_pal - 3));
      fprintf(stderr, "\t5 =< ipo_pc (%d) =< n_col-2 (%d)\n", ipo_pc[n_paletes_ord],
              (n_col - 2));
      fprintf(stderr, "\t%.1f =< v_pal (%.1f) =< %.1f\n", MIN_VEL, v_pal[n_paletes_ord],
              MAX_VEL);
      fprintf(stderr, "\t%.1f =< pal_ret (%.1f) =< %.1f\n", MIN_RET,
              pal_ret[n_paletes_ord], MAX_RET);
      fclose(fit);
      exit(5);
    }

    n_paletes_ord++;
  }

  fclose(fit); 
}


int inicialitza_joc(void) {
  int i, i_port, f_port, retwin;
  char strin[51];

  retwin = win_ini(&n_fil, &n_col, '+', INVERS); 
  id_win = ini_mem(retwin);     
  p_win = map_mem(id_win);      
  win_set(p_win, n_fil, n_col); 

  if (retwin < 0) 
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

  i_port = n_fil / 2 - m_por / 2; 
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
  ipu_pc = 3; 
  if (ipu_pf + l_pal >= n_fil - 3)
    ipu_pf = 1;
  for (i = 0; i < l_pal; i++) 
  {
    win_escricar(ipu_pf + i, ipu_pc, '0', INVERS);
    for (int j = 0; j < n_paletes_ord; j++)
      win_escricar(ipo_pf[j] + i, ipo_pc[j], '1'+j, INVERS);
  }

  for (int i = 0; i < n_paletes_ord; i++)
    po_pf[i] = ipo_pf[i]; 

  pil_pf = ipil_pf;
  pil_pc = ipil_pc; 
  win_escricar(ipil_pf, ipil_pc, '.', INVERS); 

  sprintf(strin, "Tecles: \'%c\'-> amunt, \'%c\'-> avall, RETURN-> sortir.",
          TEC_AMUNT, TEC_AVALL);
  win_escristr(strin);
  return (0);
}

void *moure_pilota(void *cap) {
  int f_h, c_h;
  char rh, rv, rd, pd;

  do {
    win_retard(retard);

    if(*p_espai_premut)
      waitS(id_sem_global);

    f_h = pil_pf + pil_vf; 
    c_h = pil_pc + pil_vc;
    *p_continuar = -1; 
    rh = rv = rd = pd = ' ';
    if ((f_h != ipil_pf) || (c_h != ipil_pc)) {
      
      if (f_h != ipil_pf) 
      {
        rv = win_quincar(f_h, ipil_pc); 
        if (rv != ' ')                  
        {
          pil_vf = -pil_vf;      
          f_h = pil_pf + pil_vf; 
        }
      }
      if (c_h != ipil_pc) 
      {
        rh = win_quincar(ipil_pf, c_h); 
        if (rh != ' ')                  
        {
          pil_vc = -pil_vc;      
          c_h = pil_pc + pil_vc; 
        }
      }
      if ((f_h != ipil_pf) && (c_h != ipil_pc)) 
      {
        rd = win_quincar(f_h, c_h);
        if (rd != ' ') 
        {
          pil_vf = -pil_vf;
          pil_vc = -pil_vc; 
          f_h = pil_pf + pil_vf;
          c_h = pil_pc + pil_vc; 
        }
      }

      if (win_quincar(f_h, c_h) == ' ') 
      {                                 
        win_escricar(ipil_pf, ipil_pc, ' ', NO_INV); 

        pil_pf += pil_vf;
        pil_pc += pil_vc;
        ipil_pf = f_h;
        ipil_pc = c_h; 

        if ((ipil_pc > 0) && (ipil_pc <= n_col)) {
          win_escricar(ipil_pf, ipil_pc, '.', INVERS); 
        }                                              
        else {
          *p_continuar = ipil_pc;
        }
      }
    } else {
      pil_pf += pil_vf;
      pil_pc += pil_vc;
    }

    if((*p_tec == TEC_ESPAI || *p_tec == TEC_RETURN) && !*p_espai_premut)
      signalS(id_sem_global);
  } while ((*p_tec != TEC_RETURN) && (*p_continuar == -1) &&
           ((*p_moviments > 0) || *p_moviments == -1));

  return ((void *)(intptr_t)0);
}


void *mou_paleta_usuari(void *cap) {
  do {
    win_retard(retard);

    *p_tec = win_gettec();
    if(*p_tec == TEC_ESPAI) {
      if (*p_espai_premut)
      {
        signalS(id_sem_global);
      }else {
        waitS(id_sem_global);
      }

      *p_espai_premut = (*p_espai_premut + 1) % 2;
    }

    if (*p_tec != 0) {
      if ((*p_tec == TEC_AVALL) &&
          (win_quincar(ipu_pf + l_pal, ipu_pc) == ' ')) {
        win_escricar(ipu_pf, ipu_pc, ' ', NO_INV); 

        ipu_pf++; 

        win_escricar(ipu_pf + l_pal - 1, ipu_pc, '0', INVERS); 

        if (*p_moviments > 0) {
          waitS(id_sem_mov);
          (*p_moviments)--;
          signalS(id_sem_mov);
        }
      }

      if ((*p_tec == TEC_AMUNT) && (win_quincar(ipu_pf - 1, ipu_pc) == ' ')) {
        win_escricar(ipu_pf + l_pal - 1, ipu_pc, ' ', NO_INV); 

        ipu_pf--;

        win_escricar(ipu_pf, ipu_pc, '0', INVERS); 

        if (*p_moviments > 0) {
          waitS(id_sem_mov);
          (*p_moviments)--;
          signalS(id_sem_mov);
        }
      }
    }

  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 && ((*p_moviments > 0) || *p_moviments == -1));

  *p_espai_premut = 0;
  signalS(id_sem_global);

  return ((void *)(intptr_t)0);
}

void inicialitza_variables(int n_args, const char *ll_args[]) {
  id_moviments = ini_mem(sizeof(int));
  p_moviments = map_mem(id_moviments);
  *p_moviments = atoi(ll_args[2]);

  id_continuar = ini_mem(sizeof(int));
  p_continuar = map_mem(id_continuar);

  id_tec = ini_mem(sizeof(int));
  p_tec = map_mem(id_tec);

  id_espai_premut = ini_mem(sizeof(char));
  p_espai_premut = map_mem(id_espai_premut);
  *p_espai_premut = 0;

  id_sem_global = ini_sem(0);
  id_sem_mov = ini_sem(0);
  *p_continuar = -1;

  if (n_args == 4)
    retard = atoi(ll_args[3]);
  else
    retard = 100;
}


int main(int n_args, const char *ll_args[]) {
  char win_str[20], win_files_str[20], win_columnes_str[20], ipo_pf_str[20],
      ipo_pc_str[20], v_pal_str[20], pal_ret_str[20], po_pf_str[20],
      retard_str[20], moviment_str[20], continuar_str[20], tec_str[20],
      l_pal_str[20], index_str[20], id_semafor_glob_str[20], id_espai_premut_str[20],
      id_semafor_mov_str[20];

  if ((n_args != 3) && (n_args != 4)) {
    fprintf(stderr, "Comanda: tennis3 fit_param moviments [retard]\n");
    exit(1);
  }

  inicialitza_variables(n_args, ll_args);
  carrega_parametres(ll_args[1]);

  if (inicialitza_joc() != 0) 
    exit(4);                  

  int n_threads = 0;
  if (pthread_create(&tid[n_threads], NULL, moure_pilota, NULL) == 0)
    n_threads++;

  if (pthread_create(&tid[n_threads], NULL, mou_paleta_usuari, NULL) == 0)
    n_threads++;

  sprintf(win_str, "%d", id_win);
  sprintf(win_files_str, "%d", n_fil);
  sprintf(win_columnes_str, "%d", n_col);
  sprintf(l_pal_str, "%d", l_pal);
  sprintf(moviment_str, "%d", id_moviments);
  sprintf(tec_str, "%d", id_tec);
  sprintf(continuar_str, "%d", id_continuar);
  sprintf(retard_str, "%d", retard);
  sprintf(id_semafor_glob_str, "%d", id_sem_global);
  sprintf(id_espai_premut_str, "%d", id_espai_premut);
  sprintf(id_semafor_mov_str, "%d", id_sem_mov);

  int n = 0;
  for (int i = 0; i < n_paletes_ord; i++) {
    sprintf(ipo_pf_str, "%d", ipo_pf[i]);
    sprintf(ipo_pc_str, "%d", ipo_pc[i]);
    sprintf(v_pal_str, "%f", v_pal[i]);
    sprintf(pal_ret_str, "%f", pal_ret[i]);
    sprintf(po_pf_str, "%f", po_pf[i]);
    sprintf(index_str, "%d", i);

    tpid[n] = fork();
    if (tpid[n] == (pid_t)0) {
      execlp("./bin/pal_ord3", "pal_ord3", win_str, win_files_str, win_columnes_str,
             ipo_pf_str, ipo_pc_str, v_pal_str, pal_ret_str, po_pf_str,
             retard_str, moviment_str, continuar_str, tec_str,
             l_pal_str, index_str, id_semafor_glob_str, id_espai_premut_str, id_semafor_mov_str, (char *)0);
      fprintf(stderr, "No s'ha pogut executar el programa pal_ord3\n");
      exit(1);
    } else if (tpid[n] > (pid_t)0) {
      n++;
    }
  }

  char temps[6], minuts = 0, segons = 0;
  unsigned int ms = 0;
  do {
    sprintf(temps, "%02u:%02u", minuts, segons);

    win_escristr(temps);

    if (ms >= 1000) {
      segons++;
      ms = 0;
    }

    if (segons >= 60) {
      segons = 0;
      minuts++;
    }

    win_update();
    win_retard(50);
    ms += 50;
  } while ((*p_tec != TEC_RETURN) && *p_continuar == -1 &&
           ((*p_moviments > 0) || *p_moviments == -1));

  for (int i = 0; i < n_threads; i++) {
    pthread_join(tid[i], NULL);
  }

  for (int i = 0; i < n; i++) {
    waitpid(tpid[i], NULL, 0);
  }

  win_fi();

  printf("Temps = %s\n", temps);
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
  elim_mem(id_espai_premut);

  elim_sem (id_sem_global);
  elim_sem (id_sem_mov);

  return (0);
}
