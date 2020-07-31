#include<iostream>
#include<cmath>
#include<iomanip>
#include<vector>
#include<random>
#include<gsl/gsl_math.h>
#include<gsl/gsl_blas.h>
#include<gsl/gsl_linalg.h>
using namespace std;

/*
1| inizializzo le sigle particle WFs con loro parametri
2| inizializzo le posizioni x,y                                                                             <___
3| calcolo (H Y)/Y e suo quadrato                                                                               |
    3.1| calcolo matrice con derivate prime e seconde delle single particle WFs                                 |
    3.2| calcolo matrice inversa                                                                                |
    3.3| calcolo derivata determinante con formula matrice delle derivate * matrice inversa                     |   ripeto per stimare <E>
    3.?| se cambio una sola posizione devo ricalcolare una riga e basta o anche la matrice inversa?             |
4| calcolo nuova posizione con x(i+1)=x(i) + D*(rand()-.5)                                                      |
5| calcolo la nuova Y(x(i+1)) e valuto P=|Y(x(i+1)|^2/|Y(x(i))|^2 e scelgo se accettare o no con metropolis ____|

6| cambio parametri delle single particle WFs
7| cerco minimo di <E> dai parametri
*/

const int dim1 = 3;
const int dim2 = 2;
int orba[dim1] = {0, 1, 2};
int orbb[dim2] = {0, 1};
gsl_matrix *A   = gsl_matrix_alloc(dim1, dim1);
gsl_matrix *dxA = gsl_matrix_alloc(dim1, dim1);
gsl_matrix *dyA = gsl_matrix_alloc(dim1, dim1);
gsl_matrix *ddA = gsl_matrix_alloc(dim1, dim1);
gsl_matrix *B   = gsl_matrix_alloc(dim2, dim2);
gsl_matrix *dxB = gsl_matrix_alloc(dim2, dim2);
gsl_matrix *dyB = gsl_matrix_alloc(dim2, dim2);
gsl_matrix *ddB = gsl_matrix_alloc(dim2, dim2);
double *xa, *xb, *ya, *yb;



double state(double x, double y, double alpha, int which){
    if(which==0){
        return exp(-0.5*alpha*(x*x+y*y));
    } else if(which==1){
        return x*exp(-0.5*alpha*(x*x+y*y));
    } else if(which==2){
        return y*exp(-0.5*alpha*(x*x+y*y));
    }
}

double dxstate(double x, double y, double alpha, int which){
    if(which==0){
        return -x*alpha*exp(-0.5*alpha*(x*x+y*y));
    } else if(which==1){
        return (1-alpha*x*x)*exp(-0.5*alpha*(x*x+y*y));
    } else if(which==2){
        return (-alpha*x*y)*exp(-0.5*alpha*(x*x+y*y));
    }
}

double dystate(double x, double y, double alpha, int which){
    if(which==0){
        return -y*alpha*exp(-0.5*alpha*(x*x+y*y));
    } else if(which==1){
        return (-alpha*x*y)*exp(-0.5*alpha*(x*x+y*y));
    } else if(which==2){
        return (1-alpha*y*y)*exp(-0.5*alpha*(x*x+y*y));
    }
}

double ddstate(double x, double y, double alpha, int which){
    if(which==0){
        return alpha*exp(-0.5*alpha*(x*x+y*y))*(alpha*x*x+alpha*y*y-2);
    } else if(which==1){
        return alpha*x*exp(-0.5*alpha*(x*x+y*y))*(alpha*x*x+alpha*y*y-4);
    } else if(which==2){
        return alpha*y*exp(-0.5*alpha*(x*x+y*y))*(alpha*x*x+alpha*y*y-4);
    }
}


void init_matrix(){
    double *xa = new double[dim1] {1, 0, 2};
    double *ya = new double[dim1] {0, 1, 1};
    double *xb = new double[dim2] {1, 0};
    double *yb = new double[dim2] {1, 1};

    for(int i=0;i<dim1;i++){
        for(int j=0;j<dim1;j++){
            gsl_matrix_set(A,   j, i,   state(xa[j], ya[j], 1, orba[i]));
            gsl_matrix_set(dxA, j, i, dxstate(xa[j], ya[j], 1, orba[i]));
            gsl_matrix_set(dyA, j, i, dystate(xa[j], ya[j], 1, orba[i]));
            gsl_matrix_set(ddA, j, i, ddstate(xa[j], ya[j], 1, orba[i]));
        }
    }
    //cout<<gsl_matrix_get(A,0,0)<<" "<<gsl_matrix_get(dxA,0,0)<<" "<<gsl_matrix_get(dyA,0,0)<<" "<<gsl_matrix_get(ddA,0,0)<<endl;

    for(int i=0;i<dim2;i++){
        for(int j=0;j<dim2;j++){
            gsl_matrix_set(B,   j, i,   state(xb[j], yb[j], 1, orbb[i]));
            gsl_matrix_set(dxB, j, i, dxstate(xb[j], yb[j], 1, orbb[i]));
            gsl_matrix_set(dyB, j, i, dystate(xb[j], yb[j], 1, orbb[i]));
            gsl_matrix_set(ddB, j, i, ddstate(xb[j], yb[j], 1, orbb[i]));
        }
    }
    //cout<<gsl_matrix_get(B,0,0)<<" "<<gsl_matrix_get(dxB,0,0)<<" "<<gsl_matrix_get(dyB,0,0)<<" "<<gsl_matrix_get(ddB,0,0)<<endl;
}

void calculateT(double *output, gsl_matrix *Maa, gsl_matrix *Mbb, gsl_permutation *pa, gsl_permutation *pb){
    gsl_matrix *Ma = gsl_matrix_alloc(dim1, dim1);
    gsl_matrix *Mb = gsl_matrix_alloc(dim2, dim2);

    gsl_matrix_memcpy(Ma, Maa);
    gsl_matrix_memcpy(Mb, Mbb);

    gsl_matrix *Ia = gsl_matrix_alloc(dim1, dim1);
    gsl_matrix *Ib = gsl_matrix_alloc(dim2, dim2);

    gsl_linalg_LU_invert(Ma, pa, Ia);
    gsl_linalg_LU_invert(Mb, pb, Ib);

    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, dxA, Ia, 0, Ma);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, dxB, Ib, 0, Mb);

    double trdxa = 0, trdxb = 0;

    for(int i=0;i<dim1;i++){
        trdxa += gsl_matrix_get(Ma, i, i);
    }

    for(int i=0;i<dim2;i++){
        trdxb += gsl_matrix_get(Mb, i, i);
    }

    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, dyA, Ia, 0, Ma);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, dyB, Ib, 0, Mb);

    double trdya = 0, trdyb = 0;

    for(int i=0;i<dim1;i++){
        trdya += gsl_matrix_get(Ma, i, i);
    }

    for(int i=0;i<dim2;i++){
        trdyb += gsl_matrix_get(Mb, i, i);
    }

    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, ddA, Ia, 0, Ma);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, ddB, Ib, 0, Mb);

    double trdda = 0, trddb = 0;

    for(int i=0;i<dim1;i++){
        trdda += gsl_matrix_get(Ma, i, i);
    }

    for(int i=0;i<dim2;i++){
        trddb += gsl_matrix_get(Mb, i, i);
    }

    //cout<<trdxa<<" "<<trdya<<" "<<trdxb<<" "<<trdyb<<" "<<trdda<<" "<<trddb<<endl;

    output[0] = -0.5*(trdda + trddb + 2*(trdxa*trdxb + trdya*trdyb));
    output[1] = 0.25*((trdxa*trdxa + trdya*trdya) + (trdxb*trdxb + trdyb*trdyb) - trdda - trddb);
}

double pot(){
    double p=0;

    for(int i=0;i<dim1;i++){
        p += 0.5*(xa[i]*xa[i] + ya[i]*ya[i]);
    }
    for(int i=0;i<dim2;i++){
        p += 0.5*(xb[i]*xb[i] + yb[i]*yb[i]);
    }

    return p;
}

int main(){
    double out[3];

    xa = new double[dim1] {1, 0, 2};
    ya = new double[dim1] {0, 1, 1};
    xb = new double[dim2] {1, 0};
    yb = new double[dim2] {1, 1};

    init_matrix();

    gsl_permutation *pa = gsl_permutation_alloc(dim1);
    gsl_permutation *pb = gsl_permutation_alloc(dim2);
    gsl_matrix *Ma = gsl_matrix_alloc(dim1, dim1);
    gsl_matrix *Mb = gsl_matrix_alloc(dim2, dim2);
    int sa, sb;

    gsl_matrix_memcpy(Ma, A);
    gsl_matrix_memcpy(Mb, B);
    //cout<<gsl_matrix_get(A,0,0)<<" "<<gsl_matrix_get(A,1,0)<<endl;
    //cout<<gsl_matrix_get(A,0,1)<<" "<<gsl_matrix_get(A,1,1)<<endl;
    gsl_linalg_LU_decomp(Ma, pa, &sa);
    //cout<<gsl_matrix_get(Ma,0,0)<<" "<<gsl_matrix_get(Ma,1,0)<<endl;
    //cout<<gsl_matrix_get(Ma,0,1)<<" "<<gsl_matrix_get(Ma,1,1)<<endl;
    gsl_linalg_LU_decomp(Mb, pb, &sb);

    calculateT(out, Ma, Mb, pa, pb);
    out[2] = pot();
    cout<<"T= "<<out[0]<<" Tjf= "<<out[1]<<" V= "<<out[2]<<endl;

    double detai, detbi;

    detai = gsl_linalg_LU_det(Ma, sa);
    detbi = gsl_linalg_LU_det(Mb, sb);

    double delta,


}