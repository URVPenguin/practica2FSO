#include "../include/winsuport.h" 
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h>

#define MIN_FIL 7 
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MIN_PAL 3
#define MIN_VEL -1.0
#define MAX_VEL 1.0
#define MIN_RET 0.0
#define MAX_RET 5.0
#define MAX_ORD_PALS 9
#define MAX_THREADS MAX_ORD_PALS + 1


int n_fil, n_col, m_por;    
int l_pal;                  
float v_pal[MAX_ORD_PALS];   
float pal_ret[MAX_ORD_PALS]; 

int ipu_pf, ipu_pc; 
int ipo_pf[MAX_ORD_PALS],
    ipo_pc[MAX_ORD_PALS];  
float po_pf[MAX_ORD_PALS]; 

int ipil_pf, ipil_pc; 
float pil_pf, pil_pc; 
float pil_vf, pil_vc; 
float pil_ret;        

int retard;    
int moviments; 
pthread_t tid[MAX_THREADS];
int tec = -1, cont = -1, n_paletes_ord;


void carrega_parametres(const char *nom_fit)
{
  FILE *fit;

  fit = fopen(nom_fit, "rt"); 
  if (fit == NULL)
  {
    fprintf(stderr, "No s'ha pogut obrir el fitxer \'%s\'\n", nom_fit);
    exit(2);
  }

  if (!feof(fit))
    fscanf(fit, "%d %d %d %d\n", &n_fil, &n_col, &m_por, &l_pal);
  if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) || (n_col < MIN_COL) ||
      (n_col > MAX_COL) || (m_por < 0) || (m_por > n_fil - 3) ||
      (l_pal < MIN_PAL) || (l_pal > n_fil - 3))
  {
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
      (pil_ret > MAX_RET))
  {
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
  while (!feof(fit) && n_paletes_ord < MAX_ORD_PALS)
  {
    fscanf(fit, "%d %d %f %f\n", &ipo_pf[n_paletes_ord], &ipo_pc[n_paletes_ord], &v_pal[n_paletes_ord],
           &pal_ret[n_paletes_ord]);

    if ((ipo_pf[n_paletes_ord] < 1) || (ipo_pf[n_paletes_ord] + l_pal > n_fil - 2) ||
        (ipo_pc[n_paletes_ord] < 5) || (ipo_pc[n_paletes_ord] > n_col - 2) ||
        (v_pal[n_paletes_ord] < MIN_VEL) || (v_pal[n_paletes_ord] > MAX_VEL) ||
        (pal_ret[n_paletes_ord] < MIN_RET) || (pal_ret[n_paletes_ord] > MAX_RET))
    {
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


int inicialitza_joc(void)
{
  int i, i_port, f_port, retwin;
  char strin[51];

  retwin = win_ini(&n_fil, &n_col, '+', INVERS); 

  if (retwin < 0) 
  {
    fprintf(stderr, "Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {
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
  for (i = i_port; i <= f_port; i++)
  {
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
      win_escricar(ipo_pf[j] + i, ipo_pc[j], '1' + j, INVERS);
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

void *moure_pilota(void *cap)
{
  int f_h, c_h;
  char rh, rv, rd, pd;

  do
  {
    win_retard(retard);

    f_h = pil_pf + pil_vf;
    c_h = pil_pc + pil_vc;
    cont = -1;
    rh = rv = rd = pd = ' ';
    if ((f_h != ipil_pf) || (c_h != ipil_pc))
    {
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
        if ((ipil_pc > 0) && (ipil_pc <= n_col))       
          win_escricar(ipil_pf, ipil_pc, '.', INVERS); 
        else
          cont = ipil_pc; 
      }
    }
    else
    {
      pil_pf += pil_vf;
      pil_pc += pil_vc;
    }

  } while ((tec != TEC_RETURN) && (cont == -1) &&
           ((moviments > 0) || moviments == -1));

  return ((void *)(intptr_t)0);
}


void *mou_paleta_usuari(void *cap)
{
  do
  {
    tec = win_gettec();
    if (tec != 0)
    {
      if ((tec == TEC_AVALL) && (win_quincar(ipu_pf + l_pal, ipu_pc) == ' '))
      {
        win_escricar(ipu_pf, ipu_pc, ' ', NO_INV); 
        ipu_pf++;                                  
        win_escricar(ipu_pf + l_pal - 1, ipu_pc, '0',
                     INVERS); 
        if (moviments > 0)
          moviments--; 
      }
      if ((tec == TEC_AMUNT) && (win_quincar(ipu_pf - 1, ipu_pc) == ' '))
      {
        win_escricar(ipu_pf + l_pal - 1, ipu_pc, ' ',
                     NO_INV);                      
        ipu_pf--;                                  
        win_escricar(ipu_pf, ipu_pc, '0', INVERS); 
        if (moviments > 0)
          moviments--; 
      }
      if (tec == TEC_ESPAI)
        win_escristr("ARA HAURIA D'ATURAR ELS ELEMENTS DEL JOC");
    }
  } while ((tec != TEC_RETURN) && cont == -1 &&
           ((moviments > 0) || moviments == -1));

  return ((void *)(intptr_t)0);
}

void *mou_paleta_ordinador(void *index)
{
  int f_h;

  int i = (intptr_t)index;

  do
  {
    win_retard(retard);
    f_h = po_pf[i] + v_pal[i]; 
    if (f_h != ipo_pf[i])      
    {
      if (v_pal[i] > 0.0) 
      {
        if (win_quincar(f_h + l_pal - 1, ipo_pc[i]) ==
            ' ') 
        {
          win_escricar(ipo_pf[i], ipo_pc[i], ' ',
                       NO_INV); 
          po_pf[i] += v_pal[i];
          ipo_pf[i] = po_pf[i]; 
          win_escricar(ipo_pf[i] + l_pal - 1, ipo_pc[i], '1' + i,
                       INVERS); 
          if (moviments > 0)
            moviments--; 
        }
        else 
          v_pal[i] = -v_pal[i];
      }
      else 
      {
        if (win_quincar(f_h, ipo_pc[i]) == ' ') 
        {
          win_escricar(ipo_pf[i] + l_pal - 1, ipo_pc[i], ' ',
                       NO_INV); 
          po_pf[i] += v_pal[i];
          ipo_pf[i] = po_pf[i]; 
          win_escricar(ipo_pf[i], ipo_pc[i], '1' + i,
                       INVERS); 
          if (moviments > 0)
            moviments--; 
        }
        else 
          v_pal[i] = -v_pal[i];
      }
    }
    else
      po_pf[i] += v_pal[i]; 
  } while ((tec != TEC_RETURN) && cont == -1 &&
           ((moviments > 0) || moviments == -1));

  return ((void *)(intptr_t)0);
}


int main(int n_args, const char *ll_args[])
{
  int n_thr = 0;

  if ((n_args != 3) && (n_args != 4))
  {
    fprintf(stderr, "Comanda: tennis0 fit_param moviments [retard]\n");
    exit(1);
  }
  carrega_parametres(ll_args[1]);
  moviments = atoi(ll_args[2]);

  if (n_args == 4)
    retard = atoi(ll_args[3]);
  else
    retard = 100;

  if (inicialitza_joc() != 0) 
    exit(4);                  

  if (pthread_create(&tid[n_thr], NULL, moure_pilota, NULL) == 0)
    n_thr++;

  if (pthread_create(&tid[n_thr], NULL, mou_paleta_usuari, NULL) == 0)
    n_thr++;

  for (int i = 0; i < n_paletes_ord; i++)
  {
    if (pthread_create(&tid[n_thr], NULL, mou_paleta_ordinador,
                       (void *)(intptr_t)i) == 0)
      n_thr++;
  }

  char temps[6];
  char minuts = 0, segons = 0;
  do
  {
    sprintf(temps, "%02u:%02u", minuts, segons);
    win_escristr(temps);
    win_retard(950);
    segons++;
    if (segons >= 60)
    {
      segons = 0;
      minuts++;
    }
  } while ((tec != TEC_RETURN) && cont == -1 &&
           ((moviments > 0) || moviments == -1));

  for (int i = 0; i < n_thr; i++)
  {
    pthread_join(tid[i], NULL);
  }

  win_fi();

  printf("Temps = %s\n", temps);
  if (tec == TEC_RETURN)
    printf("S'ha aturat el joc amb la tecla RETURN!\n");
  else
  {
    if (cont == 0 || moviments == 0)
      printf("Ha guanyat l'ordinador!\n");
    else
      printf("Ha guanyat l'usuari!\n");
  }
  return (0);
}
