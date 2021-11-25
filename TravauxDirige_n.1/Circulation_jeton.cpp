# include <iostream>
# include <cstdlib>
# include <mpi.h>

int main( int nargs, char* argv[] )
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...

	MPI_Init( &nargs, &argv );
	// Pour des raison préfère toujours cloner le communicateur global
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

	// On peut maintenant commencer à écrire notre programme parallèle en utilisant les
	// services offerts par MPI.
	std::cout << "Hello World, I'm processus " << rank << " on " << nbp << " processes.\n";

	int version_programme = 1; //0 : un seul jeton, 1 autant de jetons que de process
	int buf;

	if (version_programme == 0)
	{
		if (rank==0)
		{
			buf=42;
			std::cout << "Process " << rank << " sending " << buf << " to process " << rank+1 << std::endl;
			MPI_Send(&buf,1,MPI_INT,rank+1,0,globComm);
			MPI_Status status;
			MPI_Recv(&buf,1,MPI_INT,nbp-1,0,globComm,&status);
			std::cout << "Process " << rank << " received " << buf << " from process " << nbp-1 << std::endl;
		}
		else
		{
			MPI_Status status;
			MPI_Recv(&buf,1,MPI_INT,rank-1,0,globComm,&status);
			std::cout << "Process " << rank << " received " << buf << " from process " << rank-1 << std::endl;
			buf++;
			std::cout << "Process " << rank << " sending " << buf << " to process " << (rank+1)%nbp << std::endl;
			MPI_Send(&buf,1,MPI_INT,(rank+1)%nbp,0,globComm);
		}
	}
	else if (version_programme == 1)
	{
		buf = 10*rank;
		for(int i=0; i<nbp; i++)
		{
			int target = (rank+1)%nbp;
			int src = (rank-1)%nbp;
			std::cout << "Process " << rank << " sending " << buf << " to process " << target << std::endl;
			MPI_Send(&buf,1,MPI_INT,target,0,globComm);
			MPI_Status status;
			MPI_Recv(&buf,1,MPI_INT,src,0,globComm,&status);
			std::cout << "Process " << rank << " received " << buf << " from process " << src << std::endl;
			buf++;
		}
	}


	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
