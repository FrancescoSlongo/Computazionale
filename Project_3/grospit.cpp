#include<iostream>
#include<cmath>
#include<iomanip>
using namespace std;

const double X_MAX = 6.;
const double h = 0.002;
const int MESH_SIZE = 2*ceil(X_MAX/h/2)+1;
double *ddy = new double[MESH_SIZE];
double *y = new double[MESH_SIZE];
double *rho = new double[MESH_SIZE];
double Na = -.575;
const int GPstep = 9000;
double *Vdiff = new double[GPstep];

inline double pot(int i, double Na)
{
    double r2 = h*(i+1)*h*(i+1);
    return (r2/2. + Na *rho[i]/r2);
}

void ddf() // O(h^4) second derivative
{
    int i = 0;
    ddy[i] = 45*y[i]-154*y[i+1]+214*y[i+2]-156*y[i+3]+61*y[i+4]-10*y[i+5];
    ddy[i] /= (12*h*h);
    i++;
    ddy[i] = 45*y[i]-154*y[i+1]+214*y[i+2]-156*y[i+3]+61*y[i+4]-10*y[i+5];
    ddy[i] /= (12*h*h);
    for (i = 2; i < MESH_SIZE-2; i++)
    {
        ddy[i] = -y[i-2]+16*y[i-1]-30*y[i]+16*y[i+1]-y[i+2];
        ddy[i] /= (12*h*h);
    }
    i = MESH_SIZE-2;
    ddy[i] = 45*y[i]-154*y[i-1]+214*y[i-2]-156*y[i-3]+61*y[i-4]-10*y[i-5];
    ddy[i] /= (12*h*h);
    i++;
    ddy[i] = 45*y[i]-154*y[i-1]+214*y[i-2]-156*y[i-3]+61*y[i-4]-10*y[i-5];
    ddy[i] /= (12*h*h);
}

double functional(double* Vifcn)
{
    double T, Ve, Vi, r = h;
    double factor;
    ddf();
    T = y[0]*ddy[0];
    Ve = y[0]*y[0]*r*r;
    //Vi = rho[0]*y[0]*y[0]/r/r;
    Vi = y[0]*y[0]*y[0]*y[0]/r/r;
    for (int i = 1; i < MESH_SIZE - 1; i++)
    {
        (i % 2) ? factor = 4 : factor = 2;
        r = h + i*h;
        T += factor*y[i]*ddy[i];
        Ve += factor*y[i]*y[i]*r*r;
        //Vi += factor*rho[i]*y[i]*y[i]/r/r;
        Vi += factor*y[i]*y[i]*y[i]*y[i]/r/r;
    }

    r = h + (MESH_SIZE-1)*h;
    T += y[MESH_SIZE-1]*ddy[MESH_SIZE-1];
    Ve += y[MESH_SIZE-1]*y[MESH_SIZE-1]*r*r;
    //Vi += rho[N-1]*y[N-1]*y[N-1]/r/r;
    Vi += y[MESH_SIZE-1]*y[MESH_SIZE-1]*y[MESH_SIZE-1]*y[MESH_SIZE-1]/r/r;
    T *= h/3 * (-.5);
    Ve *= h/3 * .5;
    Vi *= h/3 * .5*Na;

    *Vifcn = Vi;
    cout<<"T= "<<T<<" Ve= "<<Ve<<" Vi= "<<Vi<<endl;
    return T+Ve+Vi;
}

double numerov(double E)
{
    double k_1 = E - pot(0, Na);
    double k0 = E - pot(1, Na);
    double k1;

    y[0]=h;
    y[1]=2.*h;


    for(int i=1;i<MESH_SIZE-1;i++)
    {
        k1 = E - pot(i+1, Na);

        y[i+1] = (y[i]*(2-5*h*h/6*2*k0)-y[i-1]*(1+h*h/12*2*k_1))/(1+h*h/12*2*k1);
        k_1 = k0;
        k0 = k1;
    }

    return y[MESH_SIZE-1];
}

double findGS(double precision)
{
    double tmp1, tmp2;
    double x1, x2, xm, y1, y2, ym;
    int jj=0;
    double E;
    double E0 = -10.;
    double dE = .01;
    double Emax = 15.;
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
                //xm = (x1+x2)/2.;
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
                else if(ym==y1 || ym==y2)
                {
                    break;
                }
                jj++;
            }
            E=xm;
            //cout<<"E = ";
            //printf("%5.20f  %d\n", E, jj);
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


    double norm = y[0]*y[0]+4*y[1]*y[1];
    //cout<<"norm "<<norm<<endl;
    for(int i=2;i<MESH_SIZE-1;i++)
    {
        (i % 2) ? norm += 4*y[i]*y[i] : norm += 2*y[i]*y[i];
    }
    norm+=y[MESH_SIZE-1]*y[MESH_SIZE-1];
    norm*=h/3;

    for(int i=0;i<MESH_SIZE;i++)
    {
        y[i]/=sqrt(abs(norm));
    }

    return E;
}

void GPsolve(double alpha)
{
    double Vint;
    double mu = findGS(1e-13);
    double E = functional(&Vint);
    for(int i=0;i<GPstep;i++)
    {
        for(int j=0;j<MESH_SIZE;j++)
        {
            rho[j] = alpha*y[j]*y[j]+(1-alpha)*rho[j];
        }
        cout<<i<<"| mu= "<<mu<<" E= "<<mu-Vint<<"   Diff = "<<mu-Vint-E<<endl;
        mu=findGS(1e-13);
        Vdiff[i]=mu-Vint-E;
        E = functional(&Vint);
    }
}

inline double FDdet(double E)
{
    double f_1 = 0.;
    double f0 = 1.;
    double f1;


    for(int i=0;i<MESH_SIZE;i++)
    {
        f1 = (2. + 2.*h*h*pot(i, Na)-2.*h*h*E)*f0 - f_1;
        //f1 = (2./sqrt(h) + 2.*sqrt(h)*h*pot(i, Na)-2.*sqrt(h)*h*E)*f0 - 1./h* f_1;

        //cout<<"f_1 "<<f_1<<" f0 "<<f0<<" f1 "<<f1<<" pot "<<pot(i, Na)<<endl;
        //printf("%.20lf\n",(2. + 2.*h*h*pot(i, Na)-2.*h*h*E));
        f_1 = f0;
        f0 = f1;
    } //*/

    /*
    f_1 = (2. + 2.*h*h*pot(0, Na)-2.*h*h*E);
    f1 = f_1;
    for(int i=1;i<MESH_SIZE;i++)
    {
        f0 = (2. + 2.*h*h*pot(i, Na)-2.*h*h*E)-1./f_1;
        f_1=f0;
        //printf("%.20lf\n", f0);
        f1 *= f0;
    } //*/
    //printf("det = %.20lf", f1);
    return f1;
}

double findGS_FD(double precision)
{
    double tmp1, tmp2;
    double x1, x2, xm, y1, y2, ym;
    int jj=0;
    double E;
    double E0 = 0.;
    double dE = .01;
    double Emax = 15.;
    double n;

    tmp1 = FDdet(E0);
    for(n=E0+dE;n<Emax;n+=dE)
    {
        tmp2 =FDdet(n);
        if(tmp1*tmp2<0)
        {
            x1 = n-dE;
            x2 = n;
            y1 = FDdet(x1);
            y2 = FDdet(x2);
            while(abs(x2-x1)>precision&jj<10000)
            {
                xm = x2 - y2*(x1-x2)/(y1-y2);
                //xm = (x2+x1)/2.;
                ym = FDdet(xm);
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
            //cout<<"E = ";
            //printf("%5.20f  %d\n", E, jj);
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

    //evaluate the WF
    double norm;

    y[MESH_SIZE-1] = 1e-17;
    y[MESH_SIZE-2] = y[MESH_SIZE-1]*(2. + 2.*h*h*pot(MESH_SIZE-2, Na)-2.*h*h*E);
    norm = y[MESH_SIZE-1]*y[MESH_SIZE-1] + 4*y[MESH_SIZE-1]*y[MESH_SIZE-1];
    for(int i=MESH_SIZE-3;i>0;i--)
    {
       y[i]= y[i+1]*(2. + 2.*h*h*pot(i+1, Na)-2.*h*h*E)-y[i+2];
       //y[i]= y[i-1]*(2./sqrt(h) + 2.*sqrt(h)*h*pot(i, Na)-2.*sqrt(h)*h*E)-y[i-2];
       //norm += y[i]*y[i]*h;
       (i % 2) ? norm += 4*y[i]*y[i] : norm += 2*y[i]*y[i];
    }
    y[0]= y[1]*(2. + 2.*h*h*pot(1, Na)-2.*h*h*E)-y[2];
    norm+=y[0]*y[0];
    return E;
    norm*=h/3;

    for(int i=0;i<MESH_SIZE;i++)
    {
        y[i]/=sqrt(norm);
    }

}

void GPsolve_FD(double alpha)
{
    double mu = findGS_FD(1e-12);
    double Vint;
    double E = functional(&Vint);
    for(int i=0;i<GPstep;i++)
    {
        for(int j=0;j<MESH_SIZE;j++)
        {
            rho[j] = alpha*y[j]*y[j]+(1-alpha)*rho[j];
        }
        cout<<i<<"| mu= "<<mu<<"   Diff = "<<mu-Vint-E<<endl;
        Vdiff[i]=mu-Vint-E;
        mu=findGS_FD(1e-12);
        E = functional(&Vint);
    }
}






int main()
{
    FILE *out;
    out = fopen("output.txt", "w+");
    double *tmp;
    double alpha = .1;

    for(int i=0;i<MESH_SIZE;i++)
    {
        y[i]=2./pow(M_PI,1./4.)*exp(-0.5*h*(i+1)*h*(i+1))*h*(i+1);
        //y[i]=0;
        rho[i]= alpha*y[i]*y[i];
    }
    cout<<"h= "<<h<<" X_MAX = "<<X_MAX<<" alpha = "<<alpha<<" Na = "<<Na<<" MESH_SIZE "<<MESH_SIZE<<endl;

    GPsolve(alpha);
    //cout<<"E = "<<fixed<<setprecision(20)<<findGS(1e-13)<<endl;

    for(int i=0;i<3001;i++)//MESH_SIZE;i++)
    {
        fprintf(out, "%.20lf ", y[i]);
    }

    FILE *out2;
    out2 = fopen("potenziale.txt", "w+");
    for(int j=0;j<3001;j++)//MESH_SIZE;j++)
    {
        fprintf(out2, "%.20lf ", pot(j, Na));
    }
    fclose(out2);
    // */

    FILE *out3;
    out3 = fopen("pdiff.txt", "w+");
    for(int j=0;j<GPstep;j++)
    {
        fprintf(out3, "%.20lf ", Vdiff[j]);
    }
    fclose(out3); // /*

    fclose(out);
    return 0;
}
