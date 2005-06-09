#include "utils.h"
#include "nfft.h"
#include "malloc.h"

/**
 * infft makes an inverse 3d-nfft
 */
void infft(char* filename,int N,int M,int Z,int iteration, int weight)
{
  int j,k,l;                    /* some variables  */
  nfft_plan my_plan;            /* plan for the two dimensional nfft  */
  infft_plan my_iplan;          /* plan for the two dimensional infft */
  FILE* fin;                    /* input file                         */
  FILE* fout_real;              /* output file (real part) */
  FILE* fout_imag;              /* output file (imag part) */
  int my_N[3],my_n[3];          /* to init the nfft */
  double epsilon=0.0000003;     /* tmp to read the obsolent z from 700.acs
                                   epsilon is a the break criterion for
                                   the iteration */
  unsigned infft_flags = CGNR_E; /* flags for the infft */
                                   
  /* initialise my_plan, specific. 
     we don't precompute psi */
  my_N[0]=Z; my_n[0]=next_power_of_2(Z); 
  my_N[1]=N; my_n[1]=next_power_of_2(N);
  my_N[2]=N; my_n[2]=next_power_of_2(N);
  nfft_init_specific(&my_plan, 3, my_N, M*Z, my_n, 6,
                      PRE_PHI_HUT| PRE_LIN_PSI |MALLOC_X| MALLOC_F_HAT|
                      MALLOC_F| FFTW_INIT| FFT_OUT_OF_PLACE,
                      FFTW_MEASURE| FFTW_DESTROY_INPUT);
 
  /* precompute lin psi */
  if(my_plan.nfft_flags & PRE_LIN_PSI) {
    nfft_precompute_lin_psi(&my_plan,10000);
  }
                    
  if (weight)
    infft_flags = infft_flags | PRECOMPUTE_WEIGHT;
  
  /* initialise my_iplan, specific */
  infft_init_specific(&my_iplan,&my_plan, infft_flags );
 
  /* get the weights */
  fin=fopen("weights.dat","r");
  if(my_iplan.infft_flags & PRECOMPUTE_WEIGHT)
    for(k=0;k<Z;k++) {
      for(j=0;j<M;j++) {
        fscanf(fin,"%le ",&my_iplan.w[j+k*M]);
      }
      fclose(fin);
      fin=fopen("weights.dat","r");
    }
  fclose(fin);

  /* open the input file */
  fin=fopen(filename,"r");
  
  /* open the output files */
  fout_real=fopen("output_real.dat","w");
  fout_imag=fopen("output_imag.dat","w");

  /* read x,y,freal and fimag from the nodes */
  for(j=0;j<M*Z;j++)
  {
    fscanf(fin,"%le %le %le %le %le ",&my_plan.x[3*j+1],&my_plan.x[3*j+2], &my_plan.x[3*j+0],
    &my_iplan.given_f[j][0],&my_iplan.given_f[j][1]);
  }

  /* precompute psi */
  if(my_plan.nfft_flags & PRE_PSI)
    nfft_precompute_psi(&my_plan);      
  
  /* precompute full psi */
  if(my_plan.nfft_flags & PRE_FULL_PSI)
    nfft_full_psi(&my_plan,pow(10,-15));

  /* init some guess */
  for(k=0;k<my_plan.N_L;k++)
  {
    my_iplan.f_hat_iter[k][0]=0.0;
    my_iplan.f_hat_iter[k][1]=0.0;
  }
  
  /* inverse trafo */
  infft_before_loop(&my_iplan);
  for(l=0;l<iteration;l++)
  {
    /* break if dot_r_iter is smaller than epsilon*/
    if(my_iplan.dot_r_iter<epsilon)
      break;
    fprintf(stderr,"%e,  %i of %i\n",sqrt(my_iplan.dot_r_iter),
    l,iteration+1);
    infft_loop_one_step(&my_iplan);
  }
  
  for(l=0;l<Z;l++) {
    for(k=0;k<N*N;k++) {
      /* write every Layer in the files */
      fprintf(fout_real,"%le ",my_iplan.f_hat_iter[(k +N*N*Z/2+(N*N*l))%(N*N*Z) ][0]);
      fprintf(fout_imag,"%le ",my_iplan.f_hat_iter[(k +N*N*Z/2+(N*N*l))%(N*N*Z) ][1]);
    }  
    fprintf(fout_real,"\n");
    fprintf(fout_imag,"\n");
  }
  
  fclose(fout_real);
  fclose(fout_imag);

  infft_finalize(&my_iplan);
  nfft_finalize(&my_plan);
}

int main(int argc, char **argv)
{
  if (argc <= 6) { 
    printf("usage: ./reconstruct3D FILENAME N M Z ITER WEIGHTS\n");
    return 1;
  }

  infft(argv[1],atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),atoi(argv[6]));
  return 1;
}