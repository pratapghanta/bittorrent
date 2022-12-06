#include <vector>
#include <string>
#include <cstring>
#include <csignal>
#include <unistd.h>

#include "common/Helpers.hpp"
#include "common/Defaults.hpp"
#include "peer/Leecher.hpp"
#include "peer/Seeder.hpp"

#include "StartParams.hpp"

namespace {
	void handleInterruptSignal()
	{
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = SIG_IGN;
		act.sa_flags = SA_RESTART;
		sigaction(SIGPIPE, &act, NULL);
	}

	std::vector<std::string> getVectorOfStrings(int const argc, char** argv)
	{
		std::vector<std::string> data;
		data.reserve(argc);
		for (int i = 1; i < argc; i++)
		{
			data.push_back(std::string(argv[i]));
		}
		return data;
	}
}

int main (int argc, char * argv[]) 
{
	handleInterruptSignal();

	BT::StartParams const params(getVectorOfStrings(argc, argv));
	if (params.helpRequested) 
	{
		std::cout << BT::Defaults::HelpMessage << std::endl;
		return 0;
	}

	STATUSCODE status = STATUSCODE::SC_SUCCESS;
	BT::Torrent torrent(params.torrentFilename, status);
	if (SC_FAILED(status)) 
	{
		return -1;
	}

	if (params.enableVerbose) 
	{
		std::cout << params << std::endl;
		std::cout << torrent << std::endl;
	}

	if(params.IsSeeder()) 
	{
		BT::Seeder_t s(torrent, params.seederPort);
        s.StartTransfer();
	}
	else 
	{
		for (auto& seeder : params.peers) 
        {
            BT::Leecher_t l(torrent, seeder);
            l.startTransfer();
        }
	}
	
	return 0;
}



#if 0
	void *threadData = NULL;

	pthread_t seederId[cn_MaxConnections];
	bool      isTransfered = false;
	
	/* Starting thread for each leecher(Client) */
	for(i=0; i < cn_MaxConnections && peers[i] != NULL; i++)
		seederId[i] = 0;		    
	alivePeers = i;
		
	for(i=0; i < cn_MaxConnections; i++)
	{
		seederId[i] = 0;
		if(peers[i] != NULL)
		{
			_M_N_TMPARR[i] = i;
			threadData = (void *) (&_M_N_TMPARR[i]); /* indicating Nth peer */
			pthread_create(&seederId[i], NULL,  startClient, threadData);
		}
	}
		
	/* Waiting for the transfer to complete */
	while(! isTransfered && alivePeers > 0)
	{
		isTransfered = true;
		for(i=0; i< t.getNumPieces(); i++)
			if(piecesInfo[i].state != PC_COMPLETE)
				isTransfered = false;
	}

	/* Transfer complete     */
	/* Assembling the pieces */
	if(isTransfered)
	{
        ifstream  piece;
	    ofstream  save;
	    char pieceName[cn_MaxBuf] = "";
	    
		cout << "Completing the transfer.." << endl;
		log("COMPLETING THE TRANSFER");
		save.open(args.getSaveFile().c_str(), ios::binary);

		if(save.is_open())
		{
			char _tmp[1+1];
	
			for(i=0; i< t.getNumPieces(); i++)
			{
				/* Generating the piece name */
				memset(pieceName, '\0', cn_MaxBuf);
				sprintf(pieceName,".%s_%d", args.getSaveFile().c_str(), (i+1));
		
				piece.open(pieceName, ios::binary);
		
				if(piece.is_open())
				{
					while(1)
					{
						char ch = (char) piece.get();
				
						if(piece.eof())
						{
							piece.close();
							break;
						}
				
						_tmp[0] = ch;
						_tmp[1] = '\0';
				
						save.write(_tmp, 1) ;
						save.flush();
					}
			
					remove(pieceName);
				}

			}
	
			log("TRANSFER COMPLETE");
			cout << args.getSaveFile() << " completed." << endl;
			save.close();
		}
	}
	
	pthread_exit(NULL);
#endif
