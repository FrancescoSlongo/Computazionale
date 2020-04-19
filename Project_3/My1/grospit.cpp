#include<iostream>
#include<cmath>

using namespace std;

const double X_MAX = 7.;
const double h = 0.0001;
const int MESH_SIZE = ceil(X_MAX/h);
double *yold = new double[MESH_SIZE];
double *y = new double[MESH_SIZE];

inline double pot(int i, double a)
{
    return (h*(i+1)*h*(i+1)/2. + a *yold[i]/h/(i+1)*yold[i]/h/(i+1));
}

double numerov(double E)
{
    double a = 1.;
    double k_1 = E - pot(0, a);
    double k0 = E - pot(1, a);
    double k1;
    double norm= h*h*h+2*h*2*h*h;

    y[0]=h;
    y[1]=2.*h;


    for(int i=1;i<MESH_SIZE-1;i++)
    {
        k1 = E - pot(i+1, a);

        y[i+1] = (y[i]*(2-5*h*h/6*2*k0)-y[i-1]*(1+h*h/12*2*k_1))/(1+h*h/12*2*k1);
        k_1 = k0;
        k0 = k1;
        norm += y[i+1]*y[i+1]*h;
    }
    //cout<<"norm "<<norm<<endl;
    for(int i=0;i<MESH_SIZE;i++)
    {
        y[i]/=sqrt(abs(norm));
    }

    return y[MESH_SIZE-1];
}

double findGS(double precision)
{
    double tmp1, tmp2;
    double x1, x2, xm, y1, y2, ym;
    int jj=0;
    double E;
    double E0 = 0.;
    double dE = .01;
    double Emax = 6.;
    double n;

    tmp1 = numerov(E0);
    for(n=E0+dE;n<Emax;n+=dE)
    {
        tmp2 =numerov(n);
        if(tmp1*tmp2<0)
        {
            x1 = n-dE;
            x2 = n;
            y1 = numerov(x1);
            y2 = numerov(x2);
            while(abs(x2-x1)>precision&jj<10000)
            {
                xm = x2 - y2*(x1-x2)/(y1-y2);
                ym = numerov(xm);
                //cout<<"x1 "<<x1<<" x2 "<<x2<<" xm "<<xm;
                //cout<<"   y1 "<<y1<<" y2 "<<y2<<" ym "<<ym<<endl;
                if(ym*y1<0.)
                {
                    x2=xm;
                    y2=ym;
                }
                else if(ym*y2<0.)
                {
                    x1 = xm;
                    y1 = ym;
                }
                else if(ym==0.)
                {
                    x2 = xm;
                    x1 = x2;
                }
                jj++;
            }
            E=xm;
            cout<<"E = ";
            printf("%5.10f\n", E);
            break;
        }
        else if(tmp1*tmp2==0.)
        {
            if(tmp1==0.) E = n-dE;
            if(tmp2==0.) E = n;
            cout<<"E = ";
            printf("%5.10f\n", E);
            break;
        }
        tmp1 = tmp2;
    }
    if(n>=Emax){cout<<"No Energy Found"<<endl;}
    return E;

}








int main()
{
    FILE *out;
    out = fopen("output.txt", "w+");
    double *tmp;

    for(int i=0;i<MESH_SIZE;i++)
    {
        yold[i]=2./pow(M_PI,1./4.)*exp(-0.5*h*(i+1)*h*(i+1));
    }
    cout<<"h= "<<h<<" X_MAX = "<<X_MAX<<endl;
    //cout<<numerov(2.5);
    findGS(1e-12);

    for(int i=0;i<MESH_SIZE;i++)
    {
        fprintf(out, "%.12lf ", y[i]);
    }

    fclose(out);
    return 0;
}
