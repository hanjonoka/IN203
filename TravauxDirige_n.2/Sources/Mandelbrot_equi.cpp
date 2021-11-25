# include <iostream>
# include <cstdlib>
# include <string>
# include <chrono>
# include <cmath>
# include <vector>
# include <fstream>
#include <mpi.h>


/** Une structure complexe est définie pour la bonne raison que la classe
 * complex proposée par g++ est très lente ! Le calcul est bien plus rapide
 * avec la petite structure donnée ci--dessous
 **/
struct Complex
{
    Complex() : real(0.), imag(0.)
    {}
    Complex(double r, double i) : real(r), imag(i)
    {}
    Complex operator + ( const Complex& z )
    {
        return Complex(real + z.real, imag + z.imag );
    }
    Complex operator * ( const Complex& z )
    {
        return Complex(real*z.real-imag*z.imag, real*z.imag+imag*z.real);
    }
    double sqNorm() { return real*real + imag*imag; }
    double real,imag;
};

std::ostream& operator << ( std::ostream& out, const Complex& c )
{
  out << "(" << c.real << "," << c.imag << ")" << std::endl;
  return out;
}

/** Pour un c complexe donné, calcul le nombre d'itérations de mandelbrot
 * nécessaires pour détecter une éventuelle divergence. Si la suite
 * converge, la fonction retourne la valeur maxIter
 **/
int iterMandelbrot( int maxIter, const Complex& c)
{
    Complex z{0.,0.};
    // On vérifie dans un premier temps si le complexe
    // n'appartient pas à une zone de convergence connue :
    // Appartenance aux disques  C0{(0,0),1/4} et C1{(-1,0),1/4}
    if ( c.real*c.real+c.imag*c.imag < 0.0625 )
        return maxIter;
    if ( (c.real+1)*(c.real+1)+c.imag*c.imag < 0.0625 )
        return maxIter;
    // Appartenance à la cardioïde {(1/4,0),1/2(1-cos(theta))}    
    if ((c.real > -0.75) && (c.real < 0.5) ) {
        Complex ct{c.real-0.25,c.imag};
        double ctnrm2 = sqrt(ct.sqNorm());
        if (ctnrm2 < 0.5*(1-ct.real/ctnrm2)) return maxIter;
    }
    int niter = 0;
    while ((z.sqNorm() < 4.) && (niter < maxIter))
    {
        z = z*z + c;
        ++niter;
    }
    return niter;
}

/**
 * On parcourt chaque pixel de l'espace image et on fait correspondre par
 * translation et homothétie une valeur complexe c qui servira pour
 * itérer sur la suite de Mandelbrot. Le nombre d'itérations renvoyé
 * servira pour construire l'image finale.
 
 Sortie : un vecteur de taille W*H avec pour chaque case un nombre d'étape de convergence de 0 à maxIter
 MODIFICATION DE LA FONCTION :
 j'ai supprimé le paramètre W étant donné que maintenant, cette fonction ne prendra plus que des lignes de taille W en argument.
 **/
void computeMandelbrotSetRow( int W, int H, int maxIter, int num_ligne, int* pixels)
{
    // Calcul le facteur d'échelle pour rester dans le disque de rayon 2
    // centré en (0,0)
    double scaleX = 3./(W-1);
    double scaleY = 2.25/(H-1.);
    //
    // On parcourt les pixels de l'espace image :
    for ( int j = 0; j < W; ++j ) {
       Complex c{-2.+j*scaleX,-1.125+ num_ligne*scaleY};
       pixels[j] = iterMandelbrot( maxIter, c );
    }
}

std::vector<int> computeMandelbrotSet( int W, int H, int maxIter )
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::vector<int> pixels(W*H);
    start = std::chrono::system_clock::now();
    // On parcourt les pixels de l'espace image :
    for ( int i = 0; i < H; ++i ) {
      computeMandelbrotSetRow(W, H, maxIter, i, pixels.data() + W*(H-i-1) );
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Temps calcul ensemble mandelbrot : " << elapsed_seconds.count() 
              << std::endl;
    return pixels;
}

std::vector<int> computeMandelbrotSetLines( int W, int H, int maxIter, int begin, int ending, int rank)
{
    std::cout << "Process "<<rank<<" calculating from "<<begin<<" to "<<ending<<std::endl;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::vector<int> pixels(W*(ending-begin));
    start = std::chrono::system_clock::now();
    // On parcourt les pixels de l'espace image :
    for ( int i = begin; i < ending; ++i ) {
      computeMandelbrotSetRow(W, H, maxIter, i, pixels.data() + (W*(i-begin)));
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Process " << rank << " : Temps calcul ensemble mandelbrot : " << elapsed_seconds.count() 
              << std::endl;
    return pixels;
}

/** Construit et sauvegarde l'image finale **/
void savePicture( const std::string& filename, int W, int H, const std::vector<int>& nbIters, int maxIter )
{
    double scaleCol = 1./maxIter;//16777216
    std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
    ofs << "P6\n"
        << W << " " << H << "\n255\n";
    for ( int i = 0; i < W * H; ++i ) {
        double iter = scaleCol*nbIters[i];
        unsigned char r = (unsigned char)(256 - (unsigned (iter*256.) & 0xFF));
        unsigned char b = (unsigned char)(256 - (unsigned (iter*65536) & 0xFF));
        unsigned char g = (unsigned char)(256 - (unsigned( iter*16777216) & 0xFF));
        ofs << r << g << b;
    }
    ofs.close();
}

int main(int argc, char *argv[] ) 
 {
    MPI_Init( &argc, &argv );
	// Pour des raisons de portabilité qui débordent largement du cadre
	// de ce cours, on préfère toujours cloner le communicateur global
	// MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour connaître le nombre de processus
	// qui ont été lancés par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour connaître l'identifiant qui
	// m'a été attribué ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);

    const int W = 800;
    const int H = 600;
    // Normalement, pour un bon rendu, il faudrait le nombre d'itérations
    // ci--dessous :
    //const int maxIter = 16777216;
    const int maxIter = 8*65536;

    int nbLinesPerProcess = H/nbp;
    int begin = nbLinesPerProcess*rank;
    int ending = nbLinesPerProcess*(rank+1);
    //if(rank==nbp) ending = H;

    auto iters = computeMandelbrotSetLines(W,H,maxIter,begin,ending,rank);
    std::cout << "Process " << rank << " calculated " << iters.size() << " pixels " << std::endl;

    std::vector<int> result(0);
    int recvcount = nbLinesPerProcess*W;
    if(rank==0)
    {
        result.resize(W*H);
        std::cout << "Process " << rank << " : expecting " << result.size() << " pixels" << std::endl;
        recvcount=nbLinesPerProcess*W;
    }

    std::cout << "Process " << rank << " : calculation of "<< ending-begin << " lines ended." << std::endl;
    std::cout << "Process " << rank << " : recvcount = " << recvcount << std::endl;
    int r = MPI_Gather(iters.data(), (ending-begin)*W, MPI_INT, result.data(), recvcount, MPI_INT, 0, globComm);
    std::cout << r << std::endl;

    if(rank==0)
    {
        savePicture("mandelbrot.tga", W, H, result, maxIter);
        savePicture("Mandelbrot_iters.tga",W,iters.size()/W,iters,maxIter);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
 }
    
